#include <windows.h>
#include <stdio.h>
#include "D2EM.h"

extern unsigned int create_d2_process(PROCESS_INFORMATION &pi, char *d2exe, const char *key0, const char *key1, const char *dllnz);
extern CDKeyManager cdkeymgr;
extern char core_dll_fpath[256];
//extern FILE *cfp;
extern void set_dlv_text(int num, int column, char *text);

void create_process(PAccountConfig ac);
void set_wnd_handle(PAccountConfig ac);
DWORD WINAPI cs_login(LPVOID param);
DWORD WINAPI cs_select_char(LPVOID param);
DWORD WINAPI cs_create_game(LPVOID param);
DWORD WINAPI cs_join_game(LPVOID param);
void send_click(HWND hwnd, int x, int y);
void send_key(HWND hwnd, UINT key);
void send_text(HWND hwnd, const char *str);
int is_cs_thread_done(PAccountConfig ac);
void set_time_limit(PAccountConfig ac, int delay_ms, int timeout_ms);
void get_key(PAccountConfig ac);
void put_key(PAccountConfig ac);

enum {
	CS_CREATE_PROCESS = 1,
	CS_CREATE_PROCESS_RES,
	CS_LOGIN,
	CS_LOGIN_RES,
	CS_SELECT_CHAR,
	CS_SELECT_CHAR_RES,
	CS_CREATE_GAME,
	CS_CREATE_GAME_RES,
	CS_PRE_MONITOR_GAME,
	CS_MONITOR_GAME,
	CS_WAIT4_MASTER,
	CS_CLOSE_PROCESS,
	CS_WAIT2_CREATE_PROCESS,
	CS_WAIT2_CLOSE_PROCESS,
};

void show_d2_window(int idx) {
	idx += 1;
	if(idx < 1 || idx >= ACONFIG_SIZE || !aconfig[idx].num) return;

	PAccountConfig ac = &aconfig[idx];
	if(ac->ai.hwnd) {
		ShowWindowAsync(ac->ai.hwnd, SW_SHOWNORMAL);
	}

}

void open_d2_process(int idx) {
	idx += 1;
	if(idx < 1 || idx >= ACONFIG_SIZE || !aconfig[idx].num) return;

	PAccountConfig ac = &aconfig[idx];
	if(ac->ai.state) {
		ac->ai.is_running = 0;
		if(ac->ai.state != CS_WAIT2_CLOSE_PROCESS && ac->ai.state != CS_CLOSE_PROCESS && ac->ai.state != CS_WAIT2_CREATE_PROCESS) {
			ac->ai.last_error_status = 0;
			ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
			set_time_limit(ac, 0, 0);
			set_dlv_text(ac->num, 3, "Wait To Close Process");
		}

	} else {
		ac->ai.shm->game_count = 0;
		ac->ai.shm->srate = 0;
		ac->ai.is_running = 1;
		ac->ai.state = CS_CREATE_PROCESS;
		set_dlv_text(ac->num, 3, "Wait To Create Process");
		set_dlv_text(ac->num, 4, "");
	}

}

static int is_cs_thread_done(PAccountConfig ac) {
	if(ac->ai.cs_thread) {
		if( GetTickCount() - ac->ai.cs_thread_ts >= 10000 ) {
			TerminateThread(ac->ai.cs_thread, 1);
			ac->ai.last_error_status = 0;
			ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
			set_time_limit(ac, 0, 0);
			set_dlv_text(ac->num, 3, "Thread TimeOut!");

		} else if( WaitForSingleObject(ac->ai.cs_thread, 0) ) {
			return 0;

		}

		CloseHandle(ac->ai.cs_thread);
		ac->ai.cs_thread = 0;
	}

	return 1;
}

static void set_time_limit(PAccountConfig ac, int delay_ms, int timeout_ms) {
	ac->ai.time_stamp = GetTickCount();
	
	if(delay_ms) {
		ac->ai.delay = 1;
		ac->ai.delay_ms = delay_ms;
	} else {
		ac->ai.delay = 0;
	}

	if(timeout_ms) {
		ac->ai.timeout = 1;
		ac->ai.timeout_ms = timeout_ms;
	} else {
		ac->ai.timeout = 0;
	}
}

static void get_key(PAccountConfig ac) {
	if(ac->ai.cdkey.key0) return;

	ac->ai.max_gcount = ac->kmaxgame;
	if(cdkeymgr.get_key(ac->ai.cdkey)) {
		ac->ai.cur_gcount = 0;
		if(ac->ai.cdkey.count && ac->ai.cdkey.count < ac->kmaxgame)
			ac->ai.max_gcount = ac->ai.cdkey.count;
		ac->ai.cdkey.count = 0;

		__dlog(ac->num, "GetKey#%d, MaxCount:%d", get_cdkey_idx(ac->ai.cdkey.key0), ac->ai.max_gcount);
	}
}

static void put_key(PAccountConfig ac) {
	if(ac->ai.cdkey.key0) {
		ac->ai.cdkey.count = ac->ai.cur_gcount;
		ac->ai.cur_gcount = 0;
		__dlog(ac->num, "PutKey#%d, UsedCount:%d", get_cdkey_idx(ac->ai.cdkey.key0), ac->ai.cdkey.count);
		cdkeymgr.put_key(ac->ai.cdkey);
		memset(&ac->ai.cdkey, 0x00, sizeof(ac->ai.cdkey));
	}
}

void update_d2_process() {
	PAccountConfig ac = &aconfig[1];
	for(int i = 1; i < ACONFIG_SIZE; i++, ac++) {
		if( !is_cs_thread_done(ac) ) continue;

		if(ac->ai.delay) {
			if(GetTickCount() - ac->ai.time_stamp >= ac->ai.delay_ms) {
				ac->ai.delay = 0;
				ac->ai.time_stamp = GetTickCount();
			} else
				continue;

		} else if(ac->ai.timeout) {
			if(GetTickCount() - ac->ai.time_stamp >= ac->ai.timeout_ms) {
				ac->ai.timeout = 0;
				ac->ai.last_error_status = 0x40000000 | ac->ai.state;
				ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
			}
		}

		switch(ac->ai.state) {
			case CS_CREATE_PROCESS:
				{
					srand( GetTickCount() );
					create_process(ac);
				}
				break;

			case CS_CREATE_PROCESS_RES:
				{
					if((ac->ai.shm->gs_state & 0x07) == 0x07) {
						set_wnd_handle(ac);
						if(ac->ai.hwnd && !IsIconic(ac->ai.hwnd)) ShowWindowAsync(ac->ai.hwnd, SW_MINIMIZE);
						ac->ai.state = CS_LOGIN;
						set_time_limit(ac, 1500, 0);
					}
				}
				break;

			case CS_LOGIN:
				{
					ac->ai.state = CS_LOGIN_RES;
					set_time_limit(ac, 0, 10000);
					ac->ai.cs_thread = CreateThread(0, 0, &cs_login, ac, 0, 0);
					ac->ai.cs_thread_ts = GetTickCount();
					set_dlv_text(ac->num, 3, "Login");
				}
				break;

			case CS_LOGIN_RES:
				{
					if(ac->ai.shm->gc_state & 0x01) {
						ac->ai.realm_fail_count = 0;
						ac->ai.state = CS_SELECT_CHAR;
						set_time_limit(ac, 1500, 0);
					}
				}
				break;

			case CS_SELECT_CHAR:
				{
					ac->ai.state = CS_SELECT_CHAR_RES;
					set_time_limit(ac, 0, 10000);
					ac->ai.cs_thread = CreateThread(0, 0, &cs_select_char, ac, 0, 0);
					ac->ai.cs_thread_ts = GetTickCount();
					set_dlv_text(ac->num, 3, "Select Char");
				}
				break;

			case CS_SELECT_CHAR_RES:
				{
					if(ac->ai.shm->gc_state & 0x04) {
						if(ac->ai.shm->gc_state & 0x08) {
							char buf[128];
							unsigned int delay_ms = 6000 + (rand() % 5) * 1000;
							set_time_limit(ac, delay_ms, 0);

							if(ac == ac->master_ac) {
								ac->ai.state = CS_CREATE_GAME;
								sprintf(buf, "Wait To Create Game[%ds]", (delay_ms + 999) / 1000);
								set_dlv_text(ac->num, 3, buf);
							} else {
								ac->ai.state = CS_WAIT4_MASTER;
								sprintf(buf, "Wait For Master(%d)[%ds]", ac->master, (delay_ms + 999) / 1000);
								set_dlv_text(ac->num, 3, buf);
							}

						} else {
							ac->ai.last_error_status = 0x80000000 | CS_SELECT_CHAR_RES;
							ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
							set_time_limit(ac, 0, 0);
							set_dlv_text(ac->num, 3, "Failed To Select Char");
						}
					}
				}
				break;

			case CS_WAIT4_MASTER:
				{
					unsigned int seq_num = ac->master_ac->ai.shm->ms_ss[ac->master].seq_num;
					if(seq_num) {
						if(seq_num != ac->ai.gn_seq_num) {
							ac->ai.gc_fail_count = 0;
							ac->ai.gc_retry_count = 0;
							ac->ai.gn_seq_num = seq_num;
						}

						if(ac->ai.gc_retry_count < 4 && ac->ai.last_seq_num != seq_num) {
							ac->ai.state = CS_CREATE_GAME;
							set_time_limit(ac, 0, 0);
							set_dlv_text(ac->num, 3, "Wait To Join Game");
						}
					}
				}
				break;

			case CS_CREATE_GAME:
				{
					ac->ai.shm->tick = 0;
					ac->ai.shm->gs_state = 0;
					ac->ai.shm->gc_state = 0;
					ac->ai.tick_check = 0;
					ac->ai.last_tick = 0;
					ac->ai.last_tick_ts = 0;

					if( ac->ai.cur_gcount < ac->ai.max_gcount
						&& (ac->ai.cur_gcount < 3 || (GetTickCount() - ac->ai.gcount_ts) / ac->ai.cur_gcount >= ac->kminasec)
						|| cdkeymgr.empty() )
					{
						if(ac->ai.gc_fail_count < 3) {
							if(ac->ai.gc_retry_count < 5) {
								ac->ai.state = CS_CREATE_GAME_RES;
								set_time_limit(ac, 0, 10000);
								ac->ai.gc_retry_count++;
								if(ac == ac->master_ac) {
									ac->ai.gn_seq_num++;
									ac->ai.shm->seq_num = ac->ai.gn_seq_num;
									ac->ai.cs_thread = CreateThread(0, 0, &cs_create_game, ac, 0, 0);
									ac->ai.cs_thread_ts = GetTickCount();
									set_dlv_text(ac->num, 3, "Create Game");
									__dlog(ac->num, "Create game %d", ac->ai.gn_seq_num);
								} else {
									ac->ai.shm->seq_num = ac->ai.gn_seq_num;
									ac->ai.cs_thread = CreateThread(0, 0, &cs_join_game, ac, 0, 0);
									ac->ai.cs_thread_ts = GetTickCount();
									set_dlv_text(ac->num, 3, "Join Game");
									__dlog(ac->num, "Join game %d", ac->ai.gn_seq_num);
								}
							} else {
								ac->ai.last_error_status = 0;
								ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
								set_time_limit(ac, 0, 0);
								set_dlv_text(ac->num, 3, "Failed To Create Game");
							}

						} else {
							ac->ai.last_error_status = 0x80000100;
							ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
							set_time_limit(ac, 0, 0);
							put_key(ac);
							set_dlv_text(ac->num, 3, "Failed To Join Game");
						}

					} else {
						ac->ai.last_error_status = 0;
						ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
						set_time_limit(ac, 0, 0);
						put_key(ac);
						set_dlv_text(ac->num, 3, "Need A New Key");
					}
				}
				break;

			case CS_CREATE_GAME_RES:
				{
					if(ac->ai.shm->gc_state & 0x10 && !(ac->ai.shm->gc_state & 0x20)
						|| ac->ai.shm->gc_state & 0x40 && !(ac->ai.shm->gc_state & 0x80))
					{
						if(ac == ac->master_ac) {
							ac->ai.state = CS_CREATE_GAME;
							set_time_limit(ac, 2000, 0);
							set_dlv_text(ac->num, 3, "Recreate Game");
						} else {
							ac->ai.state = CS_WAIT4_MASTER;
							set_time_limit(ac, 2000, 0);
							set_dlv_text(ac->num, 3, "Wait For Master");
						}

					} else if(/*ac->ai.shm->gc_state & 0x10 && */ac->ai.shm->gc_state & 0x40) {
						ac->ai.gc_retry_count = 0;
						ac->ai.state = CS_PRE_MONITOR_GAME;
						ac->ai.gc_start_ts = GetTickCount();
						if(!ac->ai.cur_gcount) ac->ai.gcount_ts = GetTickCount();
						ac->ai.cur_gcount++;
						set_time_limit(ac, 0, 0);

						char buf[64];
						sprintf(buf, "InGame[%s%d:%s]", ac->gname, ac->ai.gn_seq_num, ac->gpass);
						set_dlv_text(ac->num, 3, buf);
					}
				}
				break;

			case CS_PRE_MONITOR_GAME:
				{
					unsigned int val = GetTickCount() - ac->ai.gc_start_ts;
					if(val <= 20000) {
						if(ac->ai.shm->gc_state & 0x100 && !(ac->ai.shm->gc_state & 0x200)) {
							if(!ac->ai.min_wnd && !ac->ai.shm->le_mode && IsIconic(ac->ai.hwnd)) {
								ac->ai.min_wnd = 1;
								ShowWindowAsync(ac->ai.hwnd, SW_SHOWNORMAL);
								ShowWindowAsync(ac->ai.hwnd, SW_MINIMIZE);
							}

							ac->ai.state = CS_MONITOR_GAME;
							set_time_limit(ac, /*20000 - val*/0, 0);
						}
					} else
						ac->ai.state = CS_MONITOR_GAME;
				}
				break;

			case CS_MONITOR_GAME:
				{
					unsigned int val = GetTickCount() - ac->ai.gc_start_ts;
					if(ac->ai.shm->gc_state & 0x100 && !(ac->ai.shm->gc_state & 0x200)) {
						unsigned int cur_tick = ac->ai.shm->tick;
						unsigned int sms = 0;
						if(ac->ai.tick_check && ac->ai.last_tick == cur_tick) {
							sms = GetTickCount() - ac->ai.last_tick_ts;
						} else {
							ac->ai.tick_check = 1;
							ac->ai.last_tick = cur_tick;
							ac->ai.last_tick_ts = GetTickCount();
							ac->ai.last_seq_num = ac->ai.gn_seq_num;
						}

						if(ac->gmaxsec && val > ac->gmaxsec || sms > 20000) {
							ac->ai.last_error_status = 0;
							ac->ai.state = CS_WAIT2_CLOSE_PROCESS;
							set_time_limit(ac, 0, 0);
							set_dlv_text(ac->num, 3, "Time Exceeded");
						}

					} else {
						if(ac->ai.shm->gc_state & 0x100) {
							ac->ai.gc_fail_count = 0;

							if(ac->ai.shm->game_count) {
								float srate = ac->ai.shm->srate;
								char buf[64];
								sprintf(buf, "%d:%.1f%%",
									ac->ai.shm->game_count,
									srate / 100.0);
								set_dlv_text(ac->num, 4, buf);
							}
						} else {
							ac->ai.cur_gcount--;
							ac->ai.gc_fail_count++;
						}

						ac->ai.state = CS_SELECT_CHAR_RES;
						if(ac->ai.shm->gc_state & 0x100 && ac->gminsec && val < ac->gminsec)
							set_time_limit(ac, ac->gminsec - val, 30000);
						else
							set_time_limit(ac, 0, 30000);

						set_dlv_text(ac->num, 3, "Game Exited");
					}
				}
				break;

			case CS_CLOSE_PROCESS:
				{
					if( !WaitForSingleObject(ac->ai.pi.hProcess, 0) ) {
						CloseHandle(ac->ai.pi.hThread);
						CloseHandle(ac->ai.pi.hProcess);
						ac->ai.state = CS_WAIT2_CREATE_PROCESS;

					} else if( GetTickCount() - ac->ai.quit_ts > 20000 ) {
						TerminateProcess(ac->ai.pi.hProcess, 0);
						CloseHandle(ac->ai.pi.hThread);
						CloseHandle(ac->ai.pi.hProcess);
						ac->ai.state = CS_WAIT2_CREATE_PROCESS;

					}
				}
				break;

			case CS_WAIT2_CREATE_PROCESS:
				{
					set_dlv_text(ac->num, 1, "");
					unsigned int delay_ms = 10000;
					if(ac->ai.last_error_status == (0x40000000 | CS_LOGIN_RES)) {
						if(ac->ai.o_gs_state & 0x08 && ac->ai.o_gs_state & 0x10) {
							__dlog(ac->num, "Realm Down...");
							if(ac->stoponrd) {
								ac->ai.is_running = 0;
							} else {
								delay_ms = 30 * 60000 * (1 << ac->ai.realm_fail_count);
								ac->ai.realm_fail_count++;
							}
						}

						put_key(ac);

					} else if(ac->ai.last_error_status == 0x80000100) {
						delay_ms = 10 * 60000;

					}

					memset(&ac->ai.pi, 0x00, sizeof(ac->ai.pi));
					ac->ai.hwnd = 0;
					ac->ai.min_wnd = 0;
					ac->ai.last_error_status = 0;
					ac->ai.gc_fail_count = 0;
					ac->ai.gc_retry_count = 0;
					ac->ai.shm->gs_state = 0;
					ac->ai.shm->gc_state = 0;
					ac->master_ac->ai.shm->ms_ss[ac->num].seq_num = 0;

					if(ac->ai.is_running) {
						ac->ai.state = CS_CREATE_PROCESS;
						set_time_limit(ac, delay_ms, 0);

						char buf[64];
						sprintf(buf, "Recreate Process After %ds", delay_ms / 1000);
						set_dlv_text(ac->num, 3, buf);
					} else {
						ac->ai.state = 0;
						put_key(ac);
						set_dlv_text(ac->num, 3, "");
					}
				}
				break;

			case CS_WAIT2_CLOSE_PROCESS:
				{
					ac->ai.o_gc_state = ac->ai.shm->gc_state;
					ac->ai.o_gs_state = ac->ai.shm->gs_state;

					if(ac->ai.pi.hProcess) {
						if(ac->ai.hwnd) {
							PostMessage(ac->ai.hwnd, WM_QUIT, 0, 0);
						} else {
							set_wnd_handle(ac);
							if(ac->ai.hwnd) PostMessage(ac->ai.hwnd, WM_QUIT, 0, 0);
						}
						ac->ai.state = CS_CLOSE_PROCESS;

					} else {
						ac->ai.state = CS_WAIT2_CREATE_PROCESS;

					}

					ac->ai.quit_ts = GetTickCount();
					set_dlv_text(ac->num, 3, "Close Process");
				}
				break;
		} //end switch

	}//end for

}

static void create_process(PAccountConfig ac) {
	get_key(ac);
	if( create_d2_process(ac->ai.pi, ac->d2exec, ac->ai.cdkey.key0, ac->ai.cdkey.key1, core_dll_fpath) ) {
		ac->ai.state = CS_CREATE_PROCESS_RES;
		set_time_limit(ac, 0, 20000);

		char buf[128];
		sprintf(buf, "%d", ac->ai.pi.dwProcessId);
		set_dlv_text(ac->num, 1, buf);
		set_dlv_text(ac->num, 3, "Wait For Login Prompt");

	} else {
		ac->ai.state = 0;
		ac->ai.is_running = 0;
		put_key(ac);
		set_dlv_text(ac->num, 3, "Can't Open Process!");
	}
}


static BOOL CALLBACK _EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	unsigned int *args = (unsigned int*)lParam;
	char *wnd_clsnz = (char*)args[0];
	PAccountConfig ac = (PAccountConfig)args[1];

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if(pid == ac->ai.pi.dwProcessId) {
		char buf[128];
		if( GetClassName(hwnd, buf, sizeof(buf)) ) {
			if( !strcmp(buf, wnd_clsnz) ) {
				ac->ai.hwnd = hwnd;
				return FALSE;
			}
		}
	}

	return TRUE;
}

static void set_wnd_handle(PAccountConfig ac) {
	char buf[128];
	sprintf(buf, "D%08X", ac->ai.pi.dwProcessId);
	ac->ai.hwnd = 0;

	unsigned int args[2] = {(unsigned int)buf, (unsigned int)ac};
	EnumWindows(&_EnumWindowsProc, (LPARAM)args);

}

static DWORD WINAPI cs_login(LPVOID param) {
	PAccountConfig ac = (PAccountConfig)param;

	send_click(ac->ai.hwnd, 405, 335);
	send_click(ac->ai.hwnd, 405, 335);
	send_text(ac->ai.hwnd, ac->user);
	send_key(ac->ai.hwnd, VK_TAB);
	send_text(ac->ai.hwnd, ac->pass);
	send_key(ac->ai.hwnd, VK_RETURN);

	return 0;
}

static const int D2_CHAR_POS[][2] =
{ 
	{300,140}, {540,140}, 
	{300,230}, {540,230},
	{300,315}, {540,315},
	{300,425}, {500,425}
};

static DWORD WINAPI cs_select_char(LPVOID param) {
	PAccountConfig ac = (PAccountConfig)param;
	if(ac->cpos < 8) {
		send_click(ac->ai.hwnd, D2_CHAR_POS[ac->cpos][0], D2_CHAR_POS[ac->cpos][1]);
		send_click(ac->ai.hwnd, D2_CHAR_POS[ac->cpos][0], D2_CHAR_POS[ac->cpos][1]);
	}
	return 0;
}

static const int D2_GC_DF_POS[][2] =
{
	{440, 375},
	{565, 375},
	{708, 375},
};

static DWORD WINAPI cs_create_game(LPVOID param) {
	PAccountConfig ac = (PAccountConfig)param;

	char gname[64];
	sprintf(gname, "%s%d", ac->gname, ac->ai.gn_seq_num);

	send_click(ac->ai.hwnd, 593, 462);
	send_text(ac->ai.hwnd, gname);
	send_key(ac->ai.hwnd, VK_TAB);
	send_text(ac->ai.hwnd, ac->gpass);

	if(ac->gdiff < 3) {
		send_click(ac->ai.hwnd, D2_GC_DF_POS[ac->gdiff][0], D2_GC_DF_POS[ac->gdiff][1]);
	}

	send_click(ac->ai.hwnd, 682, 419);

	return 0;
}

static DWORD WINAPI cs_join_game(LPVOID param) {
	PAccountConfig ac = (PAccountConfig)param;

	char gname[64];
	sprintf(gname, "%s%d", ac->master_ac->gname, ac->ai.gn_seq_num);

	send_click(ac->ai.hwnd, 714, 462);
	send_text(ac->ai.hwnd, gname);
	send_key(ac->ai.hwnd, VK_TAB);
	send_text(ac->ai.hwnd, ac->master_ac->gpass);

	send_click(ac->ai.hwnd, 682, 419);

	return 0;
}

static void send_click(HWND hwnd, int x, int y) {
	LPARAM pos = MAKELPARAM(x, y);
	SendMessage(hwnd, WM_MOUSEMOVE, 0, pos);
	SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, pos );
	SendMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, pos );
}


static void send_key(HWND hwnd, UINT key) {
	SendMessage(hwnd, WM_KEYDOWN, key, 0);
	SendMessage(hwnd, WM_KEYUP, key, 0);
}

static void send_text(HWND hwnd, const char *str) {
	int mlen = strlen(str);
	for(int i = 0; i < mlen; i++) {
		if( isupper(str[i]) ) {
			SendMessage(hwnd, WM_KEYDOWN, VK_SHIFT, 0 );
			SendMessage(hwnd, WM_CHAR, (WPARAM)str[i], 0 ); 
			SendMessage(hwnd, WM_KEYUP, VK_SHIFT, 0 );
		} else {
			SendMessage(hwnd, WM_CHAR, (WPARAM)str[i], 0 ); 
		}
	}

}
