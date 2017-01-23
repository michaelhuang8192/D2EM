#include "D2EM.h"
#include "D2EM_SHM.h"
#include <Python.h>
#include <stdio.h>

static FILE *_em_stdout;
static HANDLE parent_event;
static HANDLE child_event;
static DWORD _last_tick_count;
static HANDLE _child_thread;
static int _game_status;
static char dll_path[256];
static HDC _em_hdc;
static HWND _em_hwnd;
static int _packets_count;
static unsigned int _s_intval;
static int _log_to_console;

static char _shm_name[32];
static D2EM_SHM _bi_shm;
static int _gss_done;

static PyObject *sc_module;
static PyObject *sc_funcs_0args;

typedef int (__stdcall* wndproc_t)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static wndproc_t _wndproc;

static unsigned int _focus_unit[2];

D2LevelMap level_map;
static unsigned char _auto_map_flag[32];
unsigned int d2em_automap;

struct _Packet {
	int size;
	int type;
	_Packet *next;
	const char buf[1];
};

static _Packet *_deferred_packet_head;
static _Packet **_deferred_packet_tail;

static struct {
	const char *nz;
	PyObject *fz;

} sc_funcs[] = {
	{"Main", 0},
	{"OnSendPacket", 0},
	{"OnRecvPacket", 0},
	{"OnDraw", 0}

};


PD2EM_SHM d2em_shm, d2em_shm_master;
unsigned int d2em_le_mode;

extern void Init_D2EM_Py();
extern void Del_D2EM_Py();
extern HINSTANCE current_dll_base;
extern int send_packet(const char *buf, int size, int dst, int deferred);


void yield_to_child();
void yield_to_parent();
void async_with_parent();
void sync_with_parent();

DWORD WINAPI start_child(LPVOID param);

//*******************************************************

static PD2EM_SHM __get_shared_memory(const char *nz) {
	PD2EM_SHM tmp;
	char buf[64];
	sprintf(buf, "D2EM_%s", nz);
	
	HANDLE hfm = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, buf);
	if(!hfm) return 0;

	tmp = (PD2EM_SHM)MapViewOfFile(hfm, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
	if(!tmp) {
		CloseHandle(hfm);
		return 0;
	}

	CloseHandle(hfm);
	return tmp;
}

static void open_shared_memory() {
	_bi_shm.sid = _bi_shm.master = 1;

	d2em_shm = __get_shared_memory(_shm_name);
	if(!d2em_shm) d2em_shm  = &_bi_shm;


	if(d2em_shm->sid == d2em_shm->master) {
		d2em_shm_master = d2em_shm;

	} else {
		d2em_shm_master = __get_shared_memory(d2em_shm->master_shm_name);
		if(!d2em_shm_master) {
			_bi_shm.sid = _bi_shm.master = d2em_shm->master;
			d2em_shm_master = &_bi_shm;
		}
	}
	
}

static void close_shared_memory() {
	if(d2em_shm && d2em_shm != &_bi_shm) {
		UnmapViewOfFile(d2em_shm);
	}

	if(d2em_shm_master && d2em_shm_master != &_bi_shm && d2em_shm_master != d2em_shm) {
		UnmapViewOfFile(d2em_shm_master);
	}

	d2em_shm = d2em_shm_master = 0;
}

unsigned int child_sleep(unsigned int ms) {
	_s_intval = ms;
	_packets_count = 0;
	_last_tick_count = GetTickCount();

	yield_to_parent();

	unsigned int intval = GetTickCount() - _last_tick_count;
	
	return (intval >= ms ? 0 : (unsigned int)(ms - intval) );
}

void child_sleep_nointr(unsigned int ms) {
	while( (ms = child_sleep(ms)) );

}

static void post_key(int key) {
	PostMessage(_em_hwnd, 0x100, key, 0);
	PostMessage(_em_hwnd, 0x101, key, 0);
}

D2Stat* find_stat(D2StatTable *stat_table, unsigned short idx, int *count) {
	if( stat_table && (stat_table->base_stat || stat_table->stat) ) {

		D2Stat *stat;
		int size;
		
		if(stat_table->flag & 0x80000000) {
			stat = stat_table->stat;
			size = stat_table->stat_size;

		} else {
			stat = stat_table->base_stat;
			size = stat_table->base_stat_size;

		}
		
		if( size > 0 )  {

			int s = 0, e= size;
			int m;
			unsigned short _id;
			while(s < e) {
				m = (e + s) / 2;
				_id = stat[m].id;
				if( idx == _id ) {
					int cc = m + 1;
					m--;
					for(; m >= 0; m--) {
						if(idx != stat[m].id) break;
					}
					m++;

					if(count) {
						for(; cc < size; cc++) {
							if(idx != stat[cc].id) break;
						}

						*count = cc - m;
					}
					
					return &stat[m];

				} else if(idx > _id) {
					s = m + 1;
				} else {
					e = m;
				}

			} //end while

		}

	}
	
	return 0;
}

int is_map_ready() {
	return level_map.level_no;

}

static void reveal_automap(D2Level *level) {
	unsigned int level_no = level->level_no;
	if(level_no >= sizeof(_auto_map_flag) * 8) return;

	if( !( _auto_map_flag[ level_no / 8 ] & ( 1 << (level_no % 8) ) ) ) {
		__dprintf("[Revealing AutoMap]\r\n");

		int automap_clsid = *( (int*)d2common.GetMapLevelData(level_no) + 2 );
		void *auto_map = d2client.GetAutoMap(automap_clsid);

		D2Room *nroom = level->room;
		while(nroom) {
			d2client.RevealRoom(nroom->room_ex, 1, auto_map);
			nroom = nroom->next_over_all;
		}

		_auto_map_flag[ level_no / 8 ] |= 1 << (level_no % 8);
	}

}

static void reset_level_map() {
	if(level_map.level_no) {
		level_map.level_no = 0;

		free(level_map.map);
		level_map.map = 0;

		void *tmp;
		D2RU5 *head0 = level_map.ru5;
		level_map.ru5 = 0;
		while(head0) {
			tmp = (void*)head0;
			head0 = head0->next;
			free(tmp);
		};

		D2RU *head1 = level_map.ru;
		level_map.ru = 0;
		while(head1) {
			tmp = (void*)head1;
			head1 = head1->next;
			free(tmp);
		};

		D2RMEX *rmex = level_map.rmex;
		for(int n = 0; n < level_map.rmex_count; n++, rmex++) {
			if(rmex->nbrm_count) free(rmex->nbrm);
		}
		free(level_map.rmex);
		level_map.rmex = 0;
	}

}

static int load_map() {
	D2Unit *cp = d2client.GetCurrentPlayer();
	if(cp && cp->path0) {
		D2RoomEx *room_ex = cp->path0->room_ex;
		if(room_ex) { //update or load map
			D2Level *level = room_ex->room->level;
			unsigned int level_no = level->level_no;
			unsigned int lvl_x = level->x * 5;
			unsigned int lvl_y = level->y * 5;
			unsigned int lvl_sx = level->sx * 5;
			unsigned int lvl_sy = level->sy * 5;
			unsigned int rm_x, rm_y, rm_sx, rm_sy, x_off, y_off;
			unsigned short *dst_map;
			unsigned short *src_map;
			
			if(level_map.level_no == level_no) {
				D2Room *nroom = level->room;
				while(nroom) {
					if(nroom->room_ex && nroom->_ref_type <= 1) {
						D2RoomEx *nr = nroom->room_ex;
						rm_x = nr->x_m;
						rm_y = nr->y_m;
						rm_sx = nr->sx_m;
						rm_sy = nr->sy_m;
						x_off = rm_x - lvl_x;
						y_off = rm_y - lvl_y;
						dst_map = level_map.map + y_off * lvl_sx + x_off;
						src_map = nr->collisn_map->map;

						for(unsigned int y = 0; y < rm_sy; y++) {
							memcpy(dst_map, src_map, rm_sx * sizeof(unsigned short) );
							dst_map += lvl_sx;
							src_map += rm_sx;
						}
					}

					nroom = nroom->next_over_all;
				}//end while
				
			} else {
				reset_level_map();

				int num_room = level->num_room;
				D2RMEX *rmex = (D2RMEX*)malloc( sizeof(D2RMEX) * num_room );
				memset(rmex, 0, sizeof(D2RMEX) * num_room );

				D2Room *room = level->room;
				unsigned short *new_map = (unsigned short*)malloc( lvl_sx *  lvl_sy * sizeof(unsigned short) );
				memset(new_map, 0xFF,  lvl_sx *  lvl_sy * sizeof(unsigned short) );

				unsigned char buf[6];
				buf[5] = (unsigned char)(level_no);
				buf[0] = 0x07;

				int r_count = 0;
				int a_count = 0;
				int ru_count = 0;
				int ru5_count = 0;
				D2RMEX *rmex_tmp = rmex;

				__dprintf("INIT LEVEL MAP[0x%02X](X:0x%04X, Y:0x%04X, SX:0x%04X, SY:0x%04X)...\r\n", level_no, lvl_x, lvl_y, lvl_sx, lvl_sy);

				for(; r_count < num_room && room; r_count++, rmex_tmp++) {
					if( !room->room_ex || room->_ref_type > 1 ) {
						rmex_tmp->room = room;
						*(unsigned short*)(buf + 1) = (unsigned short)(room->x);
						*(unsigned short*)(buf + 3) = (unsigned short)(room->y);
						(*d2client.ProcessPacket)((char*)buf, sizeof(buf) );
						a_count++;
					}

					if( !room->room_ex || room->_ref_type > 1 ) {
						__dprintf("[FatalError]::InitMap!\r\n");
						MessageBox(0, "Fatal Error!", "Init Map", 0);
					}

					rm_x = room->x * 5;
					rm_y = room->y * 5;
					rm_sx = room->sx * 5;
					rm_sy = room->sy * 5;
					x_off = rm_x - lvl_x;
					y_off = rm_y - lvl_y;
					dst_map = new_map + y_off * lvl_sx + x_off;
					src_map = room->room_ex->collisn_map->map;

					for(unsigned int y = 0; y < rm_sy; y++) {
						memcpy(dst_map, src_map, rm_sx * sizeof(unsigned short) );
						dst_map += lvl_sx;
						src_map += rm_sx;
					}

					D2RoomUnit5Ref *rm_unit5_ref = room->room_unit5_ref;
					D2RU5 *new0;
					while(rm_unit5_ref) {
						new0 = (D2RU5*)malloc( sizeof(D2RU5) );
						new0->cls_id = rm_unit5_ref->data->cls_id;
						new0->dst = 0;
						if( rm_unit5_ref->dst_room )
							new0->dst = rm_unit5_ref->dst_room->level->level_no;

						new0->next = level_map.ru5;
						level_map.ru5 = new0;
						ru5_count++;

						rm_unit5_ref = rm_unit5_ref->next;
					}
					
					D2RoomUnit *rm_unit= room->room_unit;
					D2RU *new1;
					while(rm_unit) {
						new1 = (D2RU*)malloc( sizeof(D2RU) );
						new1->type = rm_unit->type;
						new1->cls_id = rm_unit->cls_id;
						new1->x = rm_unit->x + rm_x;
						new1->y = rm_unit->y + rm_y;
						new1->next = level_map.ru;
						level_map.ru = new1;
						ru_count++;

						rm_unit = rm_unit->next;
					}

					rmex_tmp->rm.level_no = level_no;
					rmex_tmp->rm.x = rm_x;
					rmex_tmp->rm.y = rm_y;
					rmex_tmp->rm.sx = rm_sx;
					rmex_tmp->rm.sy = rm_sy;
					if(room->num_near_room) {
						rmex_tmp->nbrm = (D2RM*)malloc( sizeof(D2RM) * room->num_near_room );
						rmex_tmp->nbrm_count = room->num_near_room;
						
						D2Room *_room_tmp;
						D2RM *_rm_tmp;
						for(int r = 0; r < room->num_near_room; r++) {
							_room_tmp = room->near_room[r];
							_rm_tmp = &rmex_tmp->nbrm[r];

							_rm_tmp->level_no = _room_tmp->level->level_no;
							_rm_tmp->x = _room_tmp->x * 5;
							_rm_tmp->y = _room_tmp->y * 5;
							_rm_tmp->sx = _room_tmp->sx * 5;
							_rm_tmp->sy = _room_tmp->sy * 5;
						}
					}

					room = room->next_over_all;

				} //end for, add room
				
				while(room) {
					r_count++;
					room = room->next_over_all;
				}

				if(d2em_automap) reveal_automap(level);

				buf[0] = 0x08;
				for(int i = num_room - 1; i >= 0; i--) {
					room = rmex[i].room;
					if(room) {
						rmex[i].room = 0;
						*(unsigned short*)(buf + 1) = (unsigned short)(room->x);
						*(unsigned short*)(buf + 3) = (unsigned short)(room->y);
						(*d2client.ProcessPacket)((char*)buf, sizeof(buf) );
					}

				} //remove room

				level_map.level_no = level_no;
				level_map.map = new_map;
				level_map.x = lvl_x;
				level_map.y = lvl_y;
				level_map.sx = lvl_sx;
				level_map.sy = lvl_sy;
				level_map.ru_count = ru_count;
				level_map.ru5_count = ru5_count;
				level_map.rmex = rmex;
				level_map.rmex_count = num_room;

				__dprintf("Done(GRC:%d, RC:%d, AR:%d, RUC:%d, RU5C:%d).\r\n", num_room, r_count, a_count, ru_count, ru5_count);
			} //lvl

			return 1;
		}

	}

	reset_level_map();

	return 0;
}


static void init_deferred_list() {
	_deferred_packet_head = 0;
	_deferred_packet_tail = &_deferred_packet_head;
}

static void del_deferred_list() {

	if(_deferred_packet_head) {
		_Packet *head = _deferred_packet_head;
		_deferred_packet_head = 0;
		_deferred_packet_tail = &_deferred_packet_head;

		_Packet *tmp;
		do {
			tmp = head->next;
			free(head);
			head = tmp;
		} while(head);
	}

}

void send_virtual_packet_to_client(const char *buf, int size) {
	unsigned int packet_id = buf[0];

	switch(packet_id) {
		case 0x05: //left
		case 0x08:
		case 0x0C: //right
		case 0x0F:
			{
				D2Unit *cp = d2client.GetCurrentPlayer();
				D2Skill *sh = (packet_id >= 0x0C ? cp->skill_table->right_hand : cp->skill_table->left_hand);
				unsigned short skill_id = sh->data ? sh->data->skill_id : 0;
				unsigned short skill_lvl = sh->level;

				unsigned char _buf[0x11];
				_buf[0] = 0x4d;
				_buf[1] = 0x00;
				*(unsigned int*)(_buf + 2) = cp->id;
				*(unsigned int*)(_buf + 6) = skill_id;
				_buf[10] = (unsigned char)skill_lvl;
				*(unsigned short*)(_buf + 11) = *(unsigned short*)(buf + 1);
				*(unsigned short*)(_buf + 13) = *(unsigned short*)(buf + 3);
				*(unsigned short*)(_buf + 15) = 0x00;

				send_packet((const char*)_buf, sizeof(_buf), 0, 1);
			}
			break;

		case 0x06: //left
		case 0x07:
		case 0x09:
		case 0x0A:
		case 0x0D: //right
		case 0x0E:
		case 0x10:
		case 0x11:
			{
				D2Unit *cp = d2client.GetCurrentPlayer();
				D2Skill *sh = (packet_id >= 0x0D ? cp->skill_table->right_hand : cp->skill_table->left_hand);
				unsigned short skill_id = sh->data ? sh->data->skill_id : 0;
				unsigned short skill_lvl = sh->level;

				unsigned char _buf[0x10];
				_buf[0] = 0x4C;
				_buf[1] = 0x00;
				*(unsigned int*)(_buf + 2) = cp->id;
				*(unsigned short*)(_buf + 6) = skill_id;
				_buf[8] = (unsigned char)skill_lvl;
				_buf[9] = buf[1];
				*(unsigned int*)(_buf + 10) = *(unsigned int*)(buf + 5);
				*(unsigned short*)(_buf + 14) = 0x00;

				send_packet((const char*)_buf, sizeof(_buf), 0, 1);
			}
			break;

		case 0x03:
			{
				D2Unit *cp = d2client.GetCurrentPlayer();

				unsigned char _buf[0x10];
				_buf[0] = 0x0F;
				_buf[1] = 0x00;
				*(unsigned int*)(_buf + 2) = cp->id;
				_buf[6] = 0x17;
				*(unsigned short*)(_buf + 7) = *(unsigned short*)(buf + 1);
				*(unsigned short*)(_buf + 9) = *(unsigned short*)(buf + 3);
				_buf[11] = 0x00;
				*(unsigned short*)(_buf + 12) = cp->path0->x;
				*(unsigned short*)(_buf + 14) = cp->path0->y;

				send_packet((const char*)_buf, sizeof(_buf), 0, 1);
			}
			break;

		case 0x04:
			{
				D2Unit *cp = d2client.GetCurrentPlayer();

				unsigned char _buf[0x10];
				_buf[0] = 0x10;
				_buf[1] = 0x00;
				*(unsigned int*)(_buf + 2) = cp->id;
				_buf[6] = 0x18;
				_buf[7] = buf[1];
				*(unsigned int*)(_buf + 8) = *(unsigned int*)(buf + 5);
				*(unsigned short*)(_buf + 12) = cp->path0->x;
				*(unsigned short*)(_buf + 14) = cp->path0->y;

				send_packet((const char*)_buf, sizeof(_buf), 0, 1);
			}
			break;

	}

}

int send_packet(const char *buf, int size, int dst, int deferred) {
	if( _game_status || !size ) return 0;

	int ret = size;

	if(deferred) {
		_Packet *p;
		p = (_Packet*)malloc( (unsigned int)(((_Packet*)0)->buf) + size );
		memcpy((void*)p->buf, buf, size);
		p->size = size;
		p->next = 0;
		p->type = dst ? 1 : 0;
		*_deferred_packet_tail = p;
		_deferred_packet_tail = &p->next;

	} else {
		if(dst) { //server
			ret = (*d2net.SendPacket)(size, 1, buf);
			send_virtual_packet_to_client(buf, size);
			
		} else { //client
			(*d2client.ProcessPacket)(buf, size);

		}

	}

	return ret;
}

int post_message(UINT msg, WPARAM wParam, LPARAM lParam) {
	return PostMessage(_em_hwnd, msg, wParam, lParam);
}

int get_error_code() {
	return _game_status;
}

void set_focus_on_unit(unsigned int type, unsigned int id) {
	_focus_unit[0] = type;
	_focus_unit[1] = id;
}

void text_out(unsigned int x, unsigned int y, const char *str, unsigned int color) {
	if(_em_hdc) {
		COLORREF old_color = SetTextColor(_em_hdc, (COLORREF)color );
		TextOut(_em_hdc, x, y, str, strlen(str) );
		SetTextColor(_em_hdc, old_color );

	}
}

void draw_text(const char *str, unsigned int color, LPRECT prect, DWORD ftm) {
	if(_em_hdc) {
		COLORREF old_color = SetTextColor(_em_hdc, (COLORREF)color );
		DrawText(_em_hdc, str, strlen(str), prect, ftm);
		SetTextColor(_em_hdc, old_color );

	}
}

const char *get_dll_path() {
	return dll_path;
}

static void init_pyfuncs() {
	sc_module = PyImport_ImportModule("EM_Main");
	if(sc_module) {
		for(int i = 0; i < sizeof(sc_funcs) / sizeof(sc_funcs[0]); i++ ) {
			if( PyObject_HasAttrString(sc_module, sc_funcs[i].nz) ) {
				PyObject *tmp = PyObject_GetAttrString(sc_module, sc_funcs[i].nz);
				if( PyCallable_Check(tmp) )
					sc_funcs[i].fz = tmp;
				else { Py_DECREF(tmp); }

			}
		}

		sc_funcs_0args = PyTuple_New(0);

	} else {
		PyErr_Print();

	}

}

static void del_pyfuncs() {
	if(sc_module) {
		for(int i = 0; i < sizeof(sc_funcs) / sizeof(sc_funcs[0]); i++ ) {
			if( sc_funcs[i].fz ) { Py_DECREF(sc_funcs[i].fz); }
		}

		Py_DECREF(sc_funcs_0args);
		Py_DECREF(sc_module);
	}

}

static void open_console() {
	AllocConsole();
}

static void close_console() {
	FreeConsole();
}


void __dprintf(const char *fmt, ...) {
	if(_em_stdout) {
		char buf[512];

		va_list args;
		va_start(args, fmt);
		int retsz = _vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);

		if(retsz > 0) {
			buf[ sizeof(buf) - 1 ] = 0x00;
			fwrite(buf, 1, retsz, _em_stdout);
		}

	}

}

void dumphex(const char *buf, int size) {
	for(int i = 0; i < size; i++) {
		__dprintf("%02X ", (unsigned char)buf[i]);

	}
	__dprintf("\r\n");

}

static void init_exctrl() {
	parent_event = CreateEvent(0, 0, 0, 0);
	child_event = CreateEvent(0, 0, 0, 0);

}

static void del_exctrl() {
	CloseHandle(parent_event);
	CloseHandle(child_event);

}


void *get_output_file() {
	return _em_stdout;
}

void Init_D2EM() {
	_log_to_console = 0;
	d2em_le_mode = 0;
	d2em_automap = 0;

	const char *cmdline = GetCommandLine();
	if(cmdline) {
		const char *f;
		f = strstr(cmdline, "-c");
		if(f && (f[2] == 0x00 || f[2] == ' ') ) {
			_log_to_console = 1;
		}

		f = strstr(cmdline, "-le");
		if(f && (f[3] == 0x00 || f[3] == ' ') ) {
			d2em_le_mode = 1;
		}

		f = strstr(cmdline, "-am");
		if(f && (f[3] == 0x00 || f[3] == ' ') ) {
			d2em_automap = 1;
		}

		f = strstr(cmdline, "-s ");
		if(f) {
			f += 3;
			while( *f != 0x00 && *f == ' ' ) f++;
			
			int i = 0;
			for(; *f != 0x00 && *f != ' ' && i < sizeof(_shm_name) - 1; i++, f++) {
				_shm_name[i] = *f;
			}
			_shm_name[i] = 0x00;
		}
	}

	open_shared_memory();
	d2em_shm->gs_state = 0x01;
	d2em_shm->le_mode = d2em_le_mode;

	_em_hwnd = (*d2gfx.GetHWND)();

	GetModuleFileName(current_dll_base, dll_path, sizeof(dll_path) );
	char *schr = strrchr(dll_path, '\\');
	if(schr)
		schr[0] = 0x00;
	else
		dll_path[0] = 0x00;

	init_exctrl();

	if(_log_to_console) {
		open_console();
		_em_stdout = fopen("CONOUT$", "a");
		setbuf(_em_stdout, 0);

	} else {
		char fname[256];
		_snprintf(fname, sizeof(fname), "%s\\log\\log_s%02d.txt", dll_path, d2em_shm == &_bi_shm ? GetCurrentProcessId() : d2em_shm->sid );
		_em_stdout = fopen(fname, "a");

	}

	if(d2em_shm == &_bi_shm) __dprintf("[%s] - No Shared Memory!\r\n", _shm_name);

	__dprintf("*********+Process[%05d]+*********\r\n", GetCurrentProcessId() );

	Init_D2EM_Py();
	init_pyfuncs();
}

void Del_D2EM() {
	d2em_shm->gs_state = 0x00;
	close_shared_memory();

	del_pyfuncs();
	Del_D2EM_Py();

	__dprintf("*********-Process[%05d]-*********\r\n\r\n", GetCurrentProcessId() );

	if(_em_stdout) fclose(_em_stdout);
	if(_log_to_console) close_console();
	_em_stdout = 0;

	del_exctrl();

}


void yield_to_child() {
	SetEvent(child_event);
	WaitForSingleObject(parent_event, INFINITE);

}

void yield_to_parent() {
	SetEvent(parent_event);
	WaitForSingleObject(child_event, INFINITE);

}
void async_with_parent() {
	SetEvent(parent_event);

}

void sync_with_parent() {
	WaitForSingleObject(child_event, INFINITE);

}

int OnSendPacket(int size, int type, const char *buf) {
	_packets_count++;

	if(type == 0) {
		switch(buf[0]) {
			case 0x69:
				{
					_game_status = -1;
					if(_child_thread) yield_to_child();

				}
				break;

		}

	}

	if( _child_thread && !_game_status && sc_funcs[1].fz ) {
		PyObject *args = PyTuple_New(3);
		PyTuple_SET_ITEM(args, 0, PyLong_FromLong(size) );
		PyTuple_SET_ITEM(args, 1, PyLong_FromLong(type) );
		PyTuple_SET_ITEM(args, 2, PyString_FromStringAndSize(buf, size) );

		PyObject *ret = PyObject_Call(sc_funcs[1].fz, args, 0);
		Py_DECREF(args);

		if(ret) {
			if(ret == Py_False) {
				Py_DECREF(ret);
				return 0;

			} else { Py_DECREF(ret); }

		} else { PyErr_Print(); }

	}

	return 1;
}

int OnRecvPacket(int size, int type, const char *buf) {
	_packets_count++;

	if(type == 1) {
		switch( buf[0] ) {
			case 0x02:
				{
					if(_child_thread) {
						__dprintf("error! clean up the old thread[%x]..\r\n", _child_thread);
						_game_status = -1;
						yield_to_child();

					}
					
					memset(&_auto_map_flag, 0x00, sizeof(_auto_map_flag));
					
					_game_status = 0;
					_child_thread = CreateThread(0, 0, &start_child, 0, 0, 0);
					if(_child_thread) yield_to_child();

				}
				break;

			case 0x06:
				{
					_game_status = -1;
					if(_child_thread) yield_to_child();

				}
				break;

			case 0x15:
				{
					D2Unit *cp = d2client.GetCurrentPlayer();
					if( cp && buf[1] == 0x00 && *(unsigned int*)(buf + 2) == cp->id) {
						*(char*)(buf + 10) = 0x00;
					}

				}
				break;
		}

	}

	if( _child_thread && !_game_status && sc_funcs[2].fz ) {
		PyObject *args = PyTuple_New(3);
		PyTuple_SET_ITEM(args, 0, PyLong_FromLong(size) );
		PyTuple_SET_ITEM(args, 1, PyLong_FromLong(type) );
		PyTuple_SET_ITEM(args, 2, PyString_FromStringAndSize(buf, size) );

		PyObject *ret = PyObject_Call(sc_funcs[2].fz, args, 0);
		Py_DECREF(args);

		if(ret) {
			if(ret == Py_False) {
				Py_DECREF(ret);
				return 0;

			} else { Py_DECREF(ret); }

		} else { PyErr_Print(); }

	}

	return 1;
}

void OnDraw(HDC hdc) {
	if( !_gss_done ) {
		if( d2bnclient.GetPacketState(0x25) )
			d2em_shm->gs_state |= 0x02;

		if( d2bnclient.GetPacketState(0x51) )
			d2em_shm->gs_state |= 0x04;

		if( d2bnclient.GetPacketState(0x3a) )
			d2em_shm->gs_state |= 0x08;
		
		if( d2mcpclient.GetRealmLoginResCode() == 0x7F )
			d2em_shm->gs_state |= 0x10;

		if( d2em_shm->gc_state & 0x01 )
			_gss_done = 1;

		//__dprintf("%d\r\n", d2em_shm->gs_state);
	}

	if( _child_thread && !_game_status && sc_funcs[3].fz ) {
		_em_hdc = hdc;
		int old_bkm = SetBkMode(_em_hdc, TRANSPARENT);

		PyObject *ret = PyObject_Call(sc_funcs[3].fz, sc_funcs_0args, 0);
		if(ret) { Py_DECREF(ret); } else { PyErr_Print(); }

		SetBkMode(_em_hdc, old_bkm);
		_em_hdc = 0;
	}

}

void YieldControl() {
	if(_child_thread) {
		unsigned int now_tick = GetTickCount();
		if(d2em_shm) d2em_shm->tick = now_tick;

		unsigned int intval = now_tick - _last_tick_count;
		if( _packets_count ||  (intval > 1 && _s_intval <= intval) ) {
			if( !_game_status && !load_map() ) return;

			yield_to_child();

			if( !_game_status && _deferred_packet_head ) {
				_Packet *head = _deferred_packet_head;
				_deferred_packet_head = 0;
				_deferred_packet_tail = &_deferred_packet_head;
				_Packet *tmp;
				do {
					tmp = head->next;
					if(head->type) { //server
						(*d2net.SendPacket)(head->size, 1, head->buf);

					} else { //client
						(*d2client.ProcessPacket)(head->buf, head->size);

					}

					free(head);
					head = tmp;
				} while(head);

			} //end if

		}

	}

}

void OnExit() {
	_game_status = -2;
	if(_child_thread) yield_to_child();

}

void OnRecvPacket_Realm(const char *buf, int size) {
	unsigned int packet_id = (unsigned int)buf[0];
	switch(packet_id) {
		case 0x19:
			{
				d2em_shm->gc_state |= 0x01;
			}
			break;

		case 0x07:
			{
				d2em_shm->gc_state |= 0x04;
				if( *(unsigned int*)(buf + 1) )
					d2em_shm->gc_state &= ~0x08;
				else
					d2em_shm->gc_state |= 0x08;

			}
			break;

		case 0x03:
			{
				d2em_shm->gc_state |= 0x10;
				if( *(unsigned int*)(buf + 7) )
					d2em_shm->gc_state &= ~0x20;
				else
					d2em_shm->gc_state |= 0x20;
			}
			break;

		case 0x04:
			{
				d2em_shm->gc_state |= 0x40;
				if( *(unsigned int*)(buf + 15) )
					d2em_shm->gc_state &= ~0x80;
				else
					d2em_shm->gc_state |= 0x80;
			}
			break;
	}

}

static int __stdcall OnProcMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//__dprintf("%X, %X, %X\r\n", message, wParam, lParam );

	if( message != 0x0101 || wParam != 0xFFEECC1B ) {
		return (*_wndproc)(hWnd, message, wParam, lParam);

	} else {
		wParam &= 0x000000FF;
		int ret = (*_wndproc)(hWnd, message, wParam, lParam);
		if( d2client.GetPanelFlag(9) ) {
			post_key(0x26);
			post_key(0x0D);
		}

		return ret;
	}

}

DWORD WINAPI start_child(LPVOID param) {
	sync_with_parent();
	//------
	__dprintf("*********+Thread[0x%08X]+*********\r\n", GetCurrentThreadId() );
	d2em_shm->game_count++;
	d2em_shm->gc_state |= 0x100;
	memset(d2em_shm_master->ms_ss[ d2em_shm->sid ].data, 0, sizeof(d2em_shm_master->ms_ss[d2em_shm->sid].data) );
	d2em_shm_master->ms_ss[ d2em_shm->sid ].seq_num = d2em_shm->seq_num;

	//start up
	init_deferred_list();

	_wndproc = (wndproc_t)(*(DWORD*)d2win.GetWndProcVar());
	if( _wndproc && (DWORD)_wndproc != (DWORD)&OnProcMsg )
		*(DWORD*)d2win.GetWndProcVar() = (DWORD)&OnProcMsg;
	
	//run script
	if( sc_funcs[0].fz ) {
		PyObject *ret = PyObject_Call(sc_funcs[0].fz, sc_funcs_0args, 0);
		if(ret) { Py_DECREF(ret); } else { PyErr_Print(); }
	}

	if( !_game_status ) {
		_game_status = 1;
		while( _game_status >= 0 ) {
			/*
			if( d2client.GetPanelFlag(8) && IsIconic(_em_hwnd) ) {
				ShowWindowAsync(_em_hwnd, SW_SHOWNORMAL);
				child_sleep(50); if(_game_status < 0) break;

				ShowWindowAsync(_em_hwnd, SW_SHOWMINIMIZED);
				child_sleep(50); if(_game_status < 0) break;

			}
			*/

			PostMessage(_em_hwnd, 0x100, 0x0000001B, 0);
			PostMessage(_em_hwnd, 0x101, 0xFFEECC1B, 0);
			child_sleep(50);
		}
	}

	//clean up
	DWORD _cwproc = *(DWORD*)d2win.GetWndProcVar();
	if( _cwproc && _cwproc == (DWORD)&OnProcMsg )
		*(DWORD*)d2win.GetWndProcVar() = (DWORD)_wndproc;

	_wndproc = 0;
	CloseHandle(_child_thread);
	_child_thread = 0;

	del_deferred_list();
	reset_level_map();

	__dprintf("*********-Thread[0x%08X]-*********\r\n", GetCurrentThreadId() );
	d2em_shm_master->ms_ss[ d2em_shm->sid ].seq_num = 0;
	d2em_shm->gc_state |= 0x200;

	if(_em_stdout) fflush(_em_stdout);

	//------
	async_with_parent();
	return 1;
}
