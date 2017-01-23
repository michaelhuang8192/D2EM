#include "D2EM.h"
#include "D2EM_SHM.h"
#include <Python.h>
#include <stdio.h>

extern unsigned int child_sleep(unsigned int ms);
extern void child_sleep_nointr(unsigned int ms);
extern void text_out(unsigned int x, unsigned int y, const char *str, unsigned int color);
extern void draw_text(const char *str, unsigned int color, LPRECT prect, DWORD ftm);
extern int get_error_code();
extern int post_message(UINT msg, WPARAM wParam, LPARAM lParam);
extern int send_packet(const char *buf, int size, int dst, int deferred);
extern void set_focus_on_unit(unsigned int type, unsigned int id);
extern int is_map_ready();
extern int get_teleport_path(unsigned short *map, int sx, int sy, POINT src, POINT dst, LPPOINT buf, DWORD size);
extern int get_walk_path(unsigned short *map, int sx, int sy, POINT src, POINT dst, LPPOINT buf, DWORD size);
extern D2Stat* find_stat(D2StatTable *stat_table, unsigned short id, int *count);

extern PD2EM_SHM d2em_shm;
extern PD2EM_SHM d2em_shm_master;
extern D2LevelMap level_map;

static char sys_path[4096];

static D2Unit* find_unit(unsigned int type, unsigned int id) {
	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp || !cp->_r0 ) return 0;

	D2RoomEx *rex = cp->_r0->next_roomex;
	while(rex) {
		D2Unit *unit = rex->unit;
		while(unit) {
			if(unit->type == type && unit->id == id)
				return unit;

			unit = unit->next1;
		}

		rex = rex->next;
	}

	return 0;
}

static D2Unit* get_unit(unsigned int type, unsigned int id) {
	D2Unit *cp;
	if( !type && !id ) {
		cp = d2client.GetCurrentPlayer();
	
	//} else if(type == 3) {
	//	cp = find_unit(type, id);

	} else {
		cp = d2client.GetUnit(type, id);

	}

	return cp;
}

static D2PlayerInfo *find_player_info(unsigned int id) {
	D2PlayerInfo *ret = d2client.GetPlayerInfo();
	for(; ret && ret->player_id != id; ret = ret->next) {
	}

	return ret;
}

static PyObject* _build_path(D2Unit *unit) {
	if( !unit->path ) Py_RETURN_NONE;

	PyObject *tp = 0;
	switch(unit->type) {
		case 2:
		case 4:
		case 5:
			{
				tp = PyTuple_New(2);
				D2Path2 *p2 = unit->path2;
				PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(p2->x) );
				PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(p2->y) );
			}
			break;

		default:
			{
				tp = PyTuple_New(5);
				D2Path0 *p0 = unit->path0;
				PyTuple_SET_ITEM(tp, 0, PyInt_FromLong(p0->x_s) );
				PyTuple_SET_ITEM(tp, 1, PyInt_FromLong(p0->y_s) );
				PyTuple_SET_ITEM(tp, 2, PyInt_FromLong((unsigned long)(p0->dst[0].x)) );
				PyTuple_SET_ITEM(tp, 3, PyInt_FromLong((unsigned long)(p0->dst[0].y)) );

				if( p0->pt_max && p0->dst[0].val == p0->dst[1].val ) {
					int delta[2];
					d2common.GetNextPosDelta(p0, (int*)delta);
					PyObject *tn = PyTuple_New(2);
					PyTuple_SET_ITEM(tn, 0, PyInt_FromLong(delta[0]) );
					PyTuple_SET_ITEM(tn, 1, PyInt_FromLong(delta[1]) );
					PyTuple_SET_ITEM(tp, 4, tn);

				} else {
					Py_INCREF(Py_None);
					PyTuple_SET_ITEM(tp, 4, Py_None);

				}

			}

	}

	return tp;
}

static PyObject* _build_item_ex(D2Unit *unit) {
	D2UnitEx4 *ue = unit->unit_ex4;

	unsigned int type = 0;
	unsigned int id = 0;
	if(ue->inv_table && ue->inv_table->owner) {
		D2Unit *owner = ue->inv_table->owner;
		type = owner->type;
		id = owner->id;
	}

	PyObject *tp = PyTuple_New(8);
	PyTuple_SET_ITEM(tp, 0, PyInt_FromLong(ue->quality) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(ue->flag) );
	PyTuple_SET_ITEM(tp, 2, PyInt_FromLong(ue->level) );
	PyTuple_SET_ITEM(tp, 3, PyInt_FromLong(ue->dst0) );
	PyTuple_SET_ITEM(tp, 4, PyInt_FromLong(ue->dst1) );
	PyTuple_SET_ITEM(tp, 5, PyLong_FromUnsignedLong(type) );
	PyTuple_SET_ITEM(tp, 6, PyLong_FromUnsignedLong(id) );
	PyTuple_SET_ITEM(tp, 7, PyLong_FromUnsignedLong(ue->ex_cls_id) );

	return tp;
}

static PyObject* _build_pet(D2Unit *unit) {
	D2PetRef *pet_ref = d2client.GetPetRefList();
	if( !pet_ref ) Py_RETURN_NONE;

	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type == 0) {
		PyObject *ls = PyList_New(0);
		PyObject *l;
		while( pet_ref )	 {
			if(pet_ref->player_id == id) {
				l = PyTuple_New(4);
				PyTuple_SET_ITEM(l, 0, PyLong_FromUnsignedLong(pet_ref->cls_id) );
				PyTuple_SET_ITEM(l, 1, PyLong_FromUnsignedLong(pet_ref->id) );
				PyTuple_SET_ITEM(l, 2, PyLong_FromUnsignedLong(pet_ref->flag) );
				PyTuple_SET_ITEM(l, 3, PyLong_FromUnsignedLong(pet_ref->life_percent) );

				PyList_Append(ls, l);
				Py_DECREF(l);
			}

			pet_ref = pet_ref->next;
		}

		return ls;

	} else if(type == 1) {
		while( pet_ref )	 {
			if(pet_ref->id == id) {
				return PyLong_FromUnsignedLong(pet_ref->player_id);
			}

			pet_ref = pet_ref->next;
		}

	}

	Py_RETURN_NONE;
}


static PyObject* _build_portal(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type == 0) {
		D2PlayerInfo *pi = find_player_info(id);
		if(pi) {
			PyObject *tp = PyTuple_New(2);
			PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(pi->portal_id0) );
			PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(pi->portal_id1) );
			return tp;
		}

	} else if(type == 2) {
		D2PlayerInfo *pl = d2client.GetPlayerInfo();
		while(pl) {
			if(pl->portal_id0 == id || pl->portal_id1 == id) {
				return PyLong_FromUnsignedLong(pl->player_id);
			}

			pl = pl->next;
		}

	}

	Py_RETURN_NONE;
}

static PyObject* _build_party(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type == 0) {
		D2PlayerInfo *pi = find_player_info(id);
		if(pi) {
			PyObject *tp = PyTuple_New(2);
			PyTuple_SET_ITEM(tp, 0, PyInt_FromLong(pi->party_state) );
			PyTuple_SET_ITEM(tp, 1, PyInt_FromLong(pi->party_id) );
			return tp;
		}

	}

	Py_RETURN_NONE;
}

static PyObject* _build_level_pos(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if ( type == 0){
		D2PlayerInfo *pi = find_player_info(id);
		if( pi ) {
			PyObject *tp = PyTuple_New(3);
			PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(pi->level_no) );
			PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(pi->x) );
			PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(pi->y) );
			return tp;
		}
	}

	Py_RETURN_NONE;
}

static PyObject* _build_relation(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type == 0) {
		D2PlayerInfo *pi = find_player_info(id);
		if( pi && pi->relation && *pi->relation ) {
			D2PlayerRelation *rel = *pi->relation;

			PyObject *ls = PyList_New(0);
			PyObject *l;

			for(; rel; rel = rel->next) {
				l = PyTuple_New(2);
				PyTuple_SET_ITEM(l, 0, PyLong_FromUnsignedLong(rel->player_id) );
				PyTuple_SET_ITEM(l, 1, PyLong_FromUnsignedLong(rel->relation) );

				PyList_Append(ls, l);
				Py_DECREF(l);
			}

			return ls;
		}

	}

	Py_RETURN_NONE;
}

static PyObject* _build_su_life(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type == 0) {
		D2PlayerInfo *pi = find_player_info(id);
		if(pi) return PyInt_FromLong(pi->life);

	} else if(type == 1) {
		D2PetRef *pet_ref = d2client.GetPetRefList();
		for(; pet_ref; pet_ref = pet_ref->next) {
			if(pet_ref->id == id)
				return PyLong_FromUnsignedLong(pet_ref->life_percent);
		}

	}

	Py_RETURN_NONE;
}

static PyObject* _build_skill(D2Skill *skill) {
	D2Unit *me = d2client.GetCurrentPlayer();
	PyObject * tp = PyTuple_New(7);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(skill->data->skill_id) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(skill->level) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(skill->flag) );
	PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong(skill->quantity) );
	PyTuple_SET_ITEM(tp, 4, PyLong_FromUnsignedLong(skill->charge) );
	PyTuple_SET_ITEM(tp, 5, PyLong_FromUnsignedLong( d2common.GetSkillLevel(me, skill) ) );
	PyTuple_SET_ITEM(tp, 6, PyLong_FromUnsignedLong( d2common.GetSkillMana(me, skill) ) );

	return tp;
}

static PyObject* _build_cur_skill(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type == 0 || type == 1) {
		D2SkillTable *st = unit->skill_table;
		if(st && st->cur_skill) { return _build_skill(st->cur_skill); }
	}

	Py_RETURN_NONE;
}

static PyObject* _build_relpos(D2Unit *unit) {
	if( !unit->path ) Py_RETURN_NONE;

	int rx, ry;
	switch(unit->type) {
		case 2:
		case 4:
		case 5:
			{
				rx = unit->path2->rx;
				ry = unit->path2->ry;
			}
			break;

		default:
			{
				rx = unit->path0->rx;
				ry = unit->path0->ry;
			}
			break;
	}

	PyObject *tp = PyTuple_New(2);
	PyTuple_SET_ITEM(tp, 0, PyInt_FromLong(rx) );
	PyTuple_SET_ITEM(tp, 1, PyInt_FromLong(ry) );

	return tp;
}

static PyObject *_build_unit_ex(D2Unit *unit) {
	if( !unit->unit_ex ) Py_RETURN_NONE;

	unsigned int type = unit->type;
	if(type == 4) {
		return _build_item_ex(unit);

	} else if(type == 1) {
		D2UnitEx1 *u1 = unit->unit_ex1;

		PyObject *tp = PyTuple_New(3);
		PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(u1->u_cls_id) );
		PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(u1->flag) );
		PyTuple_SET_ITEM(tp, 2, PyString_FromStringAndSize( (char*)u1->attrs, sizeof(u1->attrs) ) );

		return tp;

	} else if(type == 3) {
		D2UnitEx3 *u3 = unit->unit_ex3;

		PyObject *tp = PyTuple_New(2);
		PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(u3->owner_type) );
		PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(u3->owner_id) );

		return tp;
	}

	Py_RETURN_NONE;
}

static PyObject* _build_aura(D2Unit *unit) {
	unsigned int type = unit->type;
	unsigned int id = unit->id;
	if(type < 2) {
		if(unit->stat_table && unit->stat_table->aura)
			return PyString_FromStringAndSize((const char*)unit->stat_table->aura, 32);
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetExecPath(PyObject *self, PyObject *args) {
	return PyString_FromString( get_dll_path() );
}

static PyObject* D2EM_GetFlag(PyObject *self, PyObject *args) {
	unsigned int type;
	unsigned int idx;
	unsigned int val = 0;
	if( !PyArg_ParseTuple(args, "II|I", &type, &idx, &val) ) return 0;

	if(type == 0) {
		return PyLong_FromUnsignedLong( d2client.GetPanelFlag(idx) );

	} else if(type == 1) {
		int ret = 0;
		D2Unit *cp = d2client.GetCurrentPlayer();
		if( !cp ) Py_RETURN_NONE;

		if(cp) {
			switch(idx) {
				case 0: //packet in list
					{
						ret = (cp->_delay_packet && cp->_delay_packet->cur_idx);
					}
					break;

				case 1: //moving
					{
						ret = (cp->state == 3 || cp->state == 6);
					}
					break;

				case 3: //unable to click
					{
						D2Skill *sh = val ? cp->skill_table->right_hand : cp->skill_table->left_hand;
						ret = ( !d2client.TestPlayerState(cp, sh) );

					}
					break;

				case 4: //casting delay
					{
						D2Skill *sh = val ? cp->skill_table->right_hand : cp->skill_table->left_hand;
						ret = d2client.GetCastStatus(cp, sh);
						ret = (ret != 0 && ret != 5);

					}
					break;

			}
		}

		return PyInt_FromLong(ret);
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetAllStats(PyObject *self, PyObject *args) {
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|II", &type, &id) ) return 0;
	
	D2Unit *cp = get_unit(type, id);
	if( !cp || !cp->stat_table ) Py_RETURN_NONE;

	D2Stat *stat;
	int size;
	if(cp->stat_table->flag & 0x80000000) {
		stat = cp->stat_table->stat;
		size = cp->stat_table->stat_size;

	} else {
		stat = cp->stat_table->base_stat;
		size = cp->stat_table->base_stat_size;

	}

	if( !stat ) Py_RETURN_NONE;

	PyObject *tp = PyTuple_New(size);
	for(int i = 0; i < size; i++) {
		PyObject *ep = PyTuple_New(2);
		PyTuple_SET_ITEM(ep, 0, PyLong_FromUnsignedLong( stat->sid ) );
		PyTuple_SET_ITEM(ep, 1, PyLong_FromUnsignedLong( stat->val ) );
		PyTuple_SET_ITEM(tp, i, ep);

		stat += 1;
	}

	return tp;
}

static PyObject* _build_stat(D2StatTable *stat_tbl, unsigned int idx, int mul) {
	PyObject *tp;
	D2Stat *stat;

	if( !mul ) {
		stat = find_stat(stat_tbl, idx, 0);
		if(!stat) Py_RETURN_NONE;

		tp = PyTuple_New(2);
		PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong( stat->sid ) );
		PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong( stat->val ) );

	} else {
		int count;
		stat = find_stat(stat_tbl, idx, &count);
		if(!stat) Py_RETURN_NONE;

		tp = PyTuple_New(count);
		for(int i = 0; i < count; i++) {
			PyObject *ep = PyTuple_New(2);
			PyTuple_SET_ITEM(ep, 0, PyLong_FromUnsignedLong( stat->sid ) );
			PyTuple_SET_ITEM(ep, 1, PyLong_FromUnsignedLong( stat->val ) );
			PyTuple_SET_ITEM(tp, i, ep);

			stat += 1;
		}

	}

	return tp;
}

static PyObject* D2EM_GetStat(PyObject *self, PyObject *args) {
	unsigned short idx;
	int mul= 0;
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "H|iII", &idx, &mul, &type, &id) ) return 0;

	D2Unit *cp = get_unit(type, id);
	if( !cp || !cp->stat_table ) Py_RETURN_NONE;

	return _build_stat(cp->stat_table, idx, mul);
}

static PyObject* D2EM_GetWindowSize(PyObject *self, PyObject *args) {
	int sx, sy, center_rel, rx, ry;
	d2client.GetWindowSize(&sx, &sy, &center_rel, &rx, &ry);

	PyObject *tp = PyTuple_New(5);
	PyTuple_SET_ITEM(tp, 0, PyInt_FromLong( sx ) );
	PyTuple_SET_ITEM(tp, 1, PyInt_FromLong( sy ) );
	PyTuple_SET_ITEM(tp, 2, PyInt_FromLong( center_rel ) );
	PyTuple_SET_ITEM(tp, 3, PyInt_FromLong( rx ) );
	PyTuple_SET_ITEM(tp, 4, PyInt_FromLong( ry ) );

	return tp;
}

static PyObject* D2EM_GetNPCDialog(PyObject *self, PyObject *args) {
	D2Dialog *dl = d2client.GetNPCDialog();
	if( !dl || dl->_flag ) Py_RETURN_NONE;

	PyObject *tp = PyTuple_New(7);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong( dl->tick ) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromLong( dl->_flag ) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong( dl->sx ) );
	PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong( dl->sy ) );
	PyTuple_SET_ITEM(tp, 4, PyLong_FromUnsignedLong( dl->width) );
	PyTuple_SET_ITEM(tp, 5, PyLong_FromUnsignedLong( dl->height) );

	const unsigned int num_entry = dl->num_entry;
	PyObject *sp = PyTuple_New(num_entry);
	
	char buf[256];
	for(unsigned int i = 0; i < num_entry; i++) {
		D2DialogEntry *ec = &(dl->entrys[i]);

		if( ec->name && wcstombs(buf, ec->name, sizeof(buf)) > 0 )
			buf[sizeof(buf) - 1] = 0x00;
		else
			buf[0] = 0x00;

		PyObject *ep = PyTuple_New(2);
		PyTuple_SET_ITEM(ep, 0, PyString_FromString(buf) );
		PyTuple_SET_ITEM(ep, 1, PyLong_FromUnsignedLong( ec->height ) );

		PyTuple_SET_ITEM(sp, i, ep);
	}

	PyTuple_SET_ITEM(tp, 6, sp);

	return tp;
}

static POINT _path_points[128];
static PyObject* D2EM_GetTeleportPath(PyObject *self, PyObject *args) {
	POINT src, dst;
	if( !PyArg_ParseTuple(args, "iiii", &src.x, &src.y, &dst.x, &dst.y) ) return 0;

	if( !level_map.level_no ) Py_RETURN_NONE;

	const unsigned int _x = level_map.x;
	const unsigned int _y = level_map.y;

	src.x -= _x;
	src.y -= _y;
	dst.x -= _x;
	dst.y -= _y;
	const int ret = get_teleport_path(level_map.map, (int)level_map.sx, (int)level_map.sy, src, dst, _path_points, 128);	

	PyObject *tp = PyTuple_New(ret);
	for(int i = 0; i< ret; i++) {
		PyObject *s = PyTuple_New(2);
		PyTuple_SET_ITEM(s, 0, PyLong_FromLong( (unsigned int)(_path_points[i].x) + _x) );
		PyTuple_SET_ITEM(s, 1, PyLong_FromLong( (unsigned int)(_path_points[i].y) + _y) );
		PyTuple_SET_ITEM(tp, i, s);
	}

	return tp;
}

static PyObject* D2EM_GetWalkPath(PyObject *self, PyObject *args) {
	POINT src, dst;
	if( !PyArg_ParseTuple(args, "iiii", &src.x, &src.y, &dst.x, &dst.y) ) return 0;

	if( !level_map.level_no ) Py_RETURN_NONE;

	const unsigned int _x = level_map.x;
	const unsigned int _y = level_map.y;

	src.x -= _x;
	src.y -= _y;
	dst.x -= _x;
	dst.y -= _y;
	const int ret = get_walk_path(level_map.map, (int)level_map.sx, (int)level_map.sy, src, dst, _path_points, 128);	

	PyObject *tp = PyTuple_New(ret);
	for(int i = 0; i< ret; i++) {
		PyObject *s = PyTuple_New(2);
		PyTuple_SET_ITEM(s, 0, PyLong_FromLong( (unsigned int)(_path_points[i].x) + _x) );
		PyTuple_SET_ITEM(s, 1, PyLong_FromLong( (unsigned int)(_path_points[i].y) + _y) );
		PyTuple_SET_ITEM(tp, i, s);
	}

	return tp;
}

static PyObject* D2EM_GetPointInMap(PyObject *self, PyObject *args) {
	unsigned int x, y; 
	if( !PyArg_ParseTuple(args, "II", &x, &y) ) return 0;
	if( !level_map.level_no ) Py_RETURN_NONE;

	x -= level_map.x;
	y -= level_map.y;
	if(x >= level_map.sx || y >= level_map.sy)
		Py_RETURN_NONE;

	return PyLong_FromUnsignedLong( level_map.map[ y * level_map.sx + x ] );
}

static PyObject* _build_static_unit(D2RU *ru) {
	PyObject *tp = PyTuple_New(5);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(ru->type) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(ru->cls_id) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(ru->x) );
	PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong(ru->y) );

	int _s = 0;
	if( ru->type == 0x05 && level_map.ru5_count ) {
		D2RU5 *ru5 = level_map.ru5;
		while(ru5) {
			if(ru5->cls_id == ru->cls_id) {
				PyTuple_SET_ITEM(tp, 4, PyLong_FromUnsignedLong(ru5->dst) );
				_s = 1;
				break;
			}

			ru5 = ru5->next;
		}
	}

	if( !_s ) {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(tp, 4, Py_None);
	}

	return tp;
}

static PyObject* D2EM_GetStaticUnits(PyObject *self, PyObject *args) {
	unsigned int type;
	unsigned int cls_id = (unsigned int)-1;
	if( !PyArg_ParseTuple(args, "I|I", &type, &cls_id) ) return 0;

	if( !level_map.level_no || !level_map.ru_count ) Py_RETURN_NONE;

	D2RU *ru = level_map.ru;
	PyObject *ls = PyList_New(0);
	PyObject *l;
	while(ru) {
		if(ru->type == type && (cls_id == (unsigned int)-1 || ru->cls_id == cls_id) ) {
			l = _build_static_unit(ru);
			PyList_Append(ls, l);
			Py_DECREF(l);
		}

		ru = ru->next;
	}

	return ls;
}

static PyObject* D2EM_IsMapReady(PyObject *self, PyObject *args) {
	return PyLong_FromLong( is_map_ready() );
}

static PyObject* D2EM_GetErrorCode(PyObject *self, PyObject *args) {
	return PyInt_FromLong( get_error_code() );
}

static PyObject* D2EM_GetTickCount(PyObject *self, PyObject *args) {
	return PyLong_FromUnsignedLong( GetTickCount() );
}

static PyObject* D2EM_GetTickInterval(PyObject *self, PyObject *args) {
	unsigned int last_tick;
	if( !PyArg_ParseTuple(args, "I", &last_tick) ) return 0;

	return PyLong_FromUnsignedLong( (unsigned long)(GetTickCount() - last_tick) );
}

static PyObject* D2EM_GetSkill(PyObject *self, PyObject *args) {
	unsigned short skill_id;
	unsigned int flag = -1;
	if( !PyArg_ParseTuple(args, "H|I", &skill_id,  &flag) ) return 0;

	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp || !cp->skill_table ) Py_RETURN_NONE;

	D2Skill *skill = cp->skill_table->skill_list;
	while(skill) {
		if(skill->data->skill_id == skill_id && skill->flag == flag) {
			return _build_skill(skill);
		}

		skill = skill->next;
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetSkillBySide(PyObject *self, PyObject *args) {
	unsigned int side;
	if( !PyArg_ParseTuple(args, "I", &side) ) return 0;
	
	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp || !cp->skill_table ) Py_RETURN_NONE;

	D2Skill *skill;
	skill = (side ? cp->skill_table->right_hand : cp->skill_table->left_hand);
	if( !skill ) Py_RETURN_NONE;

	return _build_skill(skill);
}

static PyObject* D2EM_GetAllSkills(PyObject *self, PyObject *args) {
	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp || !cp->skill_table ) Py_RETURN_NONE;

	PyObject *ls = PyList_New(0);
	D2Skill *skill = cp->skill_table->skill_list;
	while(skill) {
		PyObject *t = _build_skill(skill);
		PyList_Append(ls, t);
		Py_DECREF(t);
		skill = skill->next;
	}

	return ls;
}

static PyObject* D2EM_GetInvInfo(PyObject *self, PyObject *args) {
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|II", &type, &id) ) return 0;
	
	D2Unit *cp = get_unit(type, id);
	if( !cp || !cp->inv_table ) Py_RETURN_NONE;

	D2InventoryTable *inv_tb = cp->inv_table;
	unsigned int cursor_id = 0;
	if(inv_tb->cursor) cursor_id = inv_tb->cursor->id;

	PyObject *tp = PyTuple_New(3);
	PyTuple_SET_ITEM(tp, 0, PyInt_FromLong( inv_tb->num_inv ) );
	PyTuple_SET_ITEM(tp, 1, PyInt_FromLong( inv_tb->count ) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(cursor_id) );
	
	return tp;
}

static PyObject* D2EM_GetMapFromInv(PyObject *self, PyObject *args) {
	int idx;
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "i|II", &idx, &type, &id) ) return 0;
	
	D2Unit *cp = get_unit(type, id);
	if( !cp || !cp->inv_table || idx < 0 || idx >= cp->inv_table->num_inv ) Py_RETURN_NONE;

	D2Inventory *inv = cp->inv_table->inv;
	if( !inv ) Py_RETURN_NONE;

	inv += idx;
	PyObject *tp = PyTuple_New(2);
	PyObject *tp_0 = PyTuple_New(2);
	PyTuple_SET_ITEM(tp_0, 0, PyLong_FromUnsignedLong(inv->w) );
	PyTuple_SET_ITEM(tp_0, 1, PyLong_FromUnsignedLong(inv->h) );
	PyTuple_SET_ITEM(tp, 0, tp_0);

	const unsigned int sz = inv->w * inv->h;
	PyObject *tp_1 = PyTuple_New( sz );
	D2Unit **map = inv->map;
	for(unsigned int i = 0; i < sz; i++) {
		if( *map == 0) {
			PyTuple_SET_ITEM(tp_1, i, PyLong_FromUnsignedLong( 0 ) );
		} else {
			PyTuple_SET_ITEM(tp_1, i, PyLong_FromUnsignedLong( (*map)->id ) );
		}

		map++;
	}

	PyTuple_SET_ITEM(tp, 1, tp_1);

	return tp;
}

static PyObject *_build_unit4(D2Unit* unit) {
	PyObject *tp = PyTuple_New(11);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(unit->cls_id) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(unit->id) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(unit->state) );

	D2UnitEx4 *ue4 = unit->unit_ex4;
	PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong(ue4->level) );
	PyTuple_SET_ITEM(tp, 4, PyLong_FromUnsignedLong(ue4->quality) );
	PyTuple_SET_ITEM(tp, 5, PyLong_FromUnsignedLong(ue4->owner_id) );
	PyTuple_SET_ITEM(tp, 6, PyLong_FromUnsignedLong(ue4->flag) );
	PyTuple_SET_ITEM(tp, 7, PyLong_FromUnsignedLong(ue4->dst0) );
	PyTuple_SET_ITEM(tp, 8, PyLong_FromUnsignedLong(ue4->dst1) );

	PyTuple_SET_ITEM(tp, 9, PyLong_FromUnsignedLong(unit->path2->x) );
	PyTuple_SET_ITEM(tp, 10, PyLong_FromUnsignedLong(unit->path2->y) );

	return tp;
}

static PyObject* D2EM_GetItemsFromInv(PyObject *self, PyObject *args) {
	int idx = -1;
	unsigned int cls_id = (unsigned int)-1;
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|iIII", &idx, &cls_id, &type, &id) ) return 0;
	
	D2Unit *cp = get_unit(type, id);
	if( !cp || !cp->inv_table || idx >= cp->inv_table->num_inv ) Py_RETURN_NONE;

	D2Unit *item;
	if(idx >= 0) {
		D2Inventory *inv = cp->inv_table->inv;
		if( !inv ) Py_RETURN_NONE;

		inv += idx;
		item = inv->head;

	} else {
		item = cp->inv_table->head;

	}

	PyObject *ls = PyList_New(0);
	while(item) {
		if( cls_id == (unsigned int)-1 || cls_id == item->cls_id ) {
			PyObject *t = PyTuple_New(4);
			PyTuple_SET_ITEM(t, 0, PyLong_FromUnsignedLong(item->cls_id) );
			PyTuple_SET_ITEM(t, 1, PyLong_FromUnsignedLong(item->id) );
			PyTuple_SET_ITEM(t, 2, PyLong_FromUnsignedLong(item->state) );
			PyTuple_SET_ITEM(t, 3, PyLong_FromUnsignedLong(item->flag) );

			PyList_Append(ls, t);
			Py_DECREF(t);
		}
		
		if( idx >= 0) {
			item = item->unit_ex4->next_c;
		} else {
			item = item->unit_ex4->next_a;
		}

	}

	return ls;
}

static PyObject* D2EM_ItemCodeToClsId(PyObject *self, PyObject *args) {
	const char *code;
	if( !PyArg_ParseTuple(args, "s", &code) ) return 0;
	
	if( strlen(code) != 3 ) Py_RETURN_NONE;

	unsigned int i_code = ( *(unsigned int*)code | 0x20000000 );
	unsigned int cls_id = d2common.CodeToClsId(i_code);

	if(cls_id == -1)
		Py_RETURN_NONE;
	else
		return PyLong_FromUnsignedLong( cls_id );
}

static PyObject* D2EM_GetItemSize(PyObject *self, PyObject *args) {
	unsigned int cls_id;
	if( !PyArg_ParseTuple(args, "I", &cls_id) ) return 0;

	unsigned char *d = (unsigned char*)d2common.GetClassData4(cls_id);
	if( !d ) Py_RETURN_NONE;

	PyObject *tp = PyTuple_New(2);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong( d[0x10F] ) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong( d[0x110] ) );

	return tp;
}

static PyObject* D2EM_ASleep(PyObject *self, PyObject *args) {
	unsigned int ms;
	if( !PyArg_ParseTuple(args, "I", &ms) ) return 0;

	return PyLong_FromUnsignedLong( child_sleep(ms) );
}

static PyObject* D2EM_TextOut(PyObject *self, PyObject *args) {
	unsigned int x, y, color;
	const char *str;

	if( !PyArg_ParseTuple(args, "IIsI", &x, &y, &str, &color) ) return 0;

	text_out(x, y, str, color);

	Py_RETURN_NONE;
}

static PyObject* D2EM_PostMessage(PyObject *self, PyObject *args) {
	UINT msg;
	WPARAM wp;
	LPARAM lp;

	if( !PyArg_ParseTuple(args, "III", &msg, &wp, &lp) ) return 0;

	//d2client.SyncScreenOffset();

	return PyLong_FromLong( post_message(msg, wp, lp) );
}

static PyObject* D2EM_SendPacket(PyObject *self, PyObject *args) {
	const char *buf;
	int size;
	int dst;
	int deferred;

	if( !PyArg_ParseTuple(args, "s#ii", &buf, &size, &dst, &deferred) ) return 0;

	return PyLong_FromLong( send_packet(buf, size, dst, deferred) );
}

static PyObject* D2EM_SetFocusOnUnit(PyObject *self, PyObject *args) {
	unsigned int type, id;
	if( !PyArg_ParseTuple(args, "II", &type, &id) ) return 0;

	set_focus_on_unit(type, id);

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetPos(PyObject *self, PyObject *args) {
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|II", &type, &id) ) return 0;
	
	D2Unit *cp = get_unit(type, id);

	unsigned int x;
	unsigned int y;
	if( !cp || !d2common.GetPosition(cp, &x, &y) ) Py_RETURN_NONE;

	PyObject *tp = PyTuple_New(2);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(x) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(y) );

	return tp;
}

static PyObject* D2EM_GetFreeGrid(PyObject *self, PyObject *args) {
	int inv_id, w, h;	
	if( !PyArg_ParseTuple(args, "iii", &inv_id, &w, &h) ) return 0;

	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp ) return 0;

	D2InventoryTable *inv_tb = cp->inv_table;
	if(inv_id < 1 || inv_id > inv_tb->num_inv)
		Py_RETURN_NONE;

	int x, y;
	int ret = d2common.FindFreeGrid( &inv_tb->inv[inv_id - 1],  &x, &y, w, h );
	if( !ret ) Py_RETURN_NONE;

	PyObject *tp = PyTuple_New(2);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromLong(x) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromLong(y) );

	return tp;
}

static PyObject* D2EM_DumpCurrentMap(PyObject *self, PyObject *args) {
	const char *path;
	unsigned short flag;
	if( !PyArg_ParseTuple(args, "sH", &path, &flag) ) return 0;

	D2Unit *cp = d2client.GetCurrentPlayer();
	if(!cp || !level_map.level_no) Py_RETURN_NONE;

	FILE *fp = fopen(path, "w");
	if(!fp) Py_RETURN_NONE;

	unsigned short me_off_x = cp->path0->x - level_map.x;
	unsigned short me_off_y = cp->path0->y - level_map.y;
	unsigned short map_sx = level_map.sx;
	unsigned short map_sy = level_map.sy;

	__dprintf("Dump Map[LevelNo:%d, Size:%dx%d], PlayerOffset[%d,%d]\r\n",
		level_map.level_no, map_sx, map_sy, me_off_x, me_off_y);

	fprintf(fp, "LevelNo:%d, Size:%dx%d, MapOffset:<%d,%d>, PlayerOffset:<%d,%d>\r\n", 
		level_map.level_no, map_sx, map_sy, level_map.x, level_map.y, me_off_x, me_off_y);

	for(unsigned int y = 0; y < map_sy; y++) {
		for(unsigned int x = 0; x < map_sx; x++) {
			if(me_off_x == x && me_off_y == y) {
				fputc('P', fp);

			} else if( *(level_map.map + y * map_sx + x) & flag ) {
				fputc('X', fp);

			} else {
				fputc(' ', fp);

			}

		}
		fputc('\n', fp);
	}

	fclose(fp);

	Py_RETURN_NONE;
}

static PyObject* D2EM_Debug(PyObject *self, PyObject *args) {

	//if( !PyArg_ParseTuple(args, "s", &type, &id) ) return 0;

	Py_RETURN_NONE;
}

static PyObject *_build_room(D2RoomEx *rex) {
	PyObject *tp = PyTuple_New(5);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(rex->x_m) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(rex->y_m) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(rex->sx_m) );
	PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong(rex->sy_m) );
	PyTuple_SET_ITEM(tp, 4, PyLong_FromUnsignedLong(rex->room->level->level_no) );

	return tp;
}

static PyObject* D2EM_GetCurrentRoom(PyObject *self, PyObject *args) {
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|II", &type, &id) ) return 0;

	D2Unit *cp = get_unit(type, id);
	if( !cp ) Py_RETURN_NONE;

	D2RoomEx *rex = d2common.GetRoomEx( cp );
	if( !rex ) Py_RETURN_NONE;

	return _build_room(rex);
}

static PyObject* D2EM_GetCurrentMap(PyObject *self, PyObject *args) {
	unsigned int type = 0;
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|II", &type, &id) ) return 0;

	D2Unit *cp = get_unit(type, id);
	if( !cp ) Py_RETURN_NONE;

	D2RoomEx *rex = d2common.GetRoomEx( cp );
	if( !rex ) Py_RETURN_NONE;

	D2Level *lvl = rex->room->level;
	PyObject *tp = PyTuple_New(5);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(lvl->x * 5) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(lvl->y * 5) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(lvl->sx * 5) );
	PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong(lvl->sy * 5) );
	PyTuple_SET_ITEM(tp, 4, PyLong_FromUnsignedLong(lvl->level_no) );

	return tp;
}

static PyObject* D2EM_GetAllRooms(PyObject *self, PyObject *args) {
	unsigned int cls_type = 0;
	if( !PyArg_ParseTuple(args, "|I", &cls_type) ) return 0;

	if(cls_type == 0) {
		D2Unit *cp = d2client.GetCurrentPlayer();
		if( !cp || !cp->path0 || !cp->path0->room_ex ) Py_RETURN_NONE;

		D2Level *lvl = cp->path0->room_ex->room->level;
		const unsigned int lvl_no = lvl->level_no;
		const int num_room = lvl->num_room;
		D2Room *room = lvl->room;
		PyObject *tp = PyTuple_New( num_room );
		PyObject *s;
		for(int i = 0; i < num_room && room; i++) {
			s = PyTuple_New(5);
			PyTuple_SET_ITEM(s, 0, PyLong_FromUnsignedLong(room->x * 5) );
			PyTuple_SET_ITEM(s, 1, PyLong_FromUnsignedLong(room->y * 5) );
			PyTuple_SET_ITEM(s, 2, PyLong_FromUnsignedLong(room->sx * 5) );
			PyTuple_SET_ITEM(s, 3, PyLong_FromUnsignedLong(room->sy * 5) );
			PyTuple_SET_ITEM(s, 4, PyLong_FromUnsignedLong(lvl_no) );

			PyTuple_SET_ITEM(tp, i, s);
			room = room->next_over_all;
		}

		return tp;

	} else if(cls_type == 1) {
		if(level_map.level_no) {
			PyObject *tp = PyTuple_New( level_map.rmex_count );
			PyObject *s, *z, *r;
			D2RMEX *rmex = level_map.rmex;
			for(int a = 0; a < level_map.rmex_count; a++, rmex++) {
				s = PyTuple_New(6);
				PyTuple_SET_ITEM(tp, a, s);

				PyTuple_SET_ITEM(s, 0, PyLong_FromUnsignedLong(rmex->rm.x) );
				PyTuple_SET_ITEM(s, 1, PyLong_FromUnsignedLong(rmex->rm.y) );
				PyTuple_SET_ITEM(s, 2, PyLong_FromUnsignedLong(rmex->rm.sx) );
				PyTuple_SET_ITEM(s, 3, PyLong_FromUnsignedLong(rmex->rm.sy) );
				PyTuple_SET_ITEM(s, 4, PyLong_FromUnsignedLong(rmex->rm.level_no) );

				z = PyTuple_New(rmex->nbrm_count);
				PyTuple_SET_ITEM(s, 5, z);

				D2RM *nbrm = rmex->nbrm;
				for(int b = 0; b < rmex->nbrm_count; b++, nbrm++) {
					r = PyTuple_New(5);
					PyTuple_SET_ITEM(z, b, r);
					PyTuple_SET_ITEM(r, 0, PyLong_FromUnsignedLong(nbrm->x) );
					PyTuple_SET_ITEM(r, 1, PyLong_FromUnsignedLong(nbrm->y) );
					PyTuple_SET_ITEM(r, 2, PyLong_FromUnsignedLong(nbrm->sx) );
					PyTuple_SET_ITEM(r, 3, PyLong_FromUnsignedLong(nbrm->sy) );
					PyTuple_SET_ITEM(r, 4, PyLong_FromUnsignedLong(nbrm->level_no) );
				}
			}

			return tp;
		}

	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetNearbyRooms(PyObject *self, PyObject *args) {
	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp || !cp->path0 || !cp->path0->room_ex ) Py_RETURN_NONE;

	D2RoomEx *rex = cp->_r0->next_roomex;
	PyObject *ls = PyList_New(0);
	while(rex) {
		PyObject *l = _build_room(rex);
		PyList_Append(ls, l);
		Py_DECREF(l);

		rex = rex->next;
	}

	return ls;
}

static PyObject* D2EM_GetNearbyUnits(PyObject *self, PyObject *args) {
	unsigned int type;
	unsigned int cls_id = (unsigned int)-1;
	if( !PyArg_ParseTuple(args, "I|I", &type, &cls_id) ) return 0;

	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp || !cp->path0 || !cp->path0->room_ex ) Py_RETURN_NONE;

	D2RoomEx *rex = cp->_r0->next_roomex;
	D2Unit *unit;
	PyObject *ls = PyList_New(0);
	PyObject *l;
	unsigned int level_no;
	while(rex) {
		unit = rex->unit;
		level_no = rex->room->level->level_no;

		while(unit) {
			if(unit->type == type && (cls_id == (unsigned int)-1 || unit->cls_id == cls_id ) ) {
				l = PyTuple_New(5);
				PyTuple_SET_ITEM(l, 0, PyLong_FromUnsignedLong(unit->cls_id) );
				PyTuple_SET_ITEM(l, 1, PyLong_FromUnsignedLong(unit->id) );
				PyTuple_SET_ITEM(l, 2, PyLong_FromUnsignedLong(unit->state) );
				PyTuple_SET_ITEM(l, 3, PyLong_FromUnsignedLong(unit->flag) );
				PyTuple_SET_ITEM(l, 4, PyLong_FromUnsignedLong(level_no) );

				PyList_Append(ls, l);
				Py_DECREF(l);
			}

			unit = unit->next1;
		}

		rex = rex->next;
	}

	return ls;
}

static PyObject* _get_name(D2Unit *unit) {
	PyObject *nz;
	switch(unit->type) {
		case 0:
			{
				if(unit->unit_ex) {
					nz = PyString_FromString( unit->unit_ex0->name );
				}

			}
			break;

		case 1:
		case 2:
		case 4:
			{
				wchar_t *name = d2client.GetName(unit);
				char buf[256];
				if( name && wcstombs(buf, name, sizeof(buf)) > 0 ){
					buf[sizeof(buf) - 1] = 0x00;
					nz = PyString_FromString(buf);
				}
			}
			break;

	}

	if( !nz ) {
		Py_INCREF(Py_None);
		nz = Py_None;
	}

	return nz;
}

static PyObject* D2EM_GetItemPrice(PyObject *self, PyObject *args) {
	unsigned int item_id, npc_clsid;
	int trading_type;
	if( !PyArg_ParseTuple(args, "IIi", &item_id, &npc_clsid, &trading_type) ) return 0;

	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp ) Py_RETURN_NONE;

	D2Unit *item = d2client.GetUnit(4, item_id);
	if( !item ) Py_RETURN_NONE;

	unsigned int diff_lvl = d2client.GetDiffcultyLevel();
	void *sb_data = d2client.GetGLBSellBuyData();

	int price = d2common.GetUnit4Price(cp, item, diff_lvl, sb_data, npc_clsid, trading_type);
	return PyInt_FromLong(price);
}

PyObject* _get_unit_status(D2Unit *cp) {
	PyObject *tp = PyTuple_New(3);
	PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong(cp->cls_id) );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong(cp->state) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong(cp->flag) );

	return tp;
}

typedef struct _FnCall_GetUnit {
	PyObject* (*fn)(D2Unit*);
	int need_unit_obj;

} FnCall_GetUnit, *PFnCall_GetUnit;

static FnCall_GetUnit fncall_getunit_tbl[] = {
	{&_get_unit_status, 1}, //0, state & flag
	{&_get_name,        1}, //1, name
	{&_build_path,      1}, //2, path
	{&_build_unit_ex,   1}, //3, extension
	{&_build_pet,       0}, //4, pets
	{&_build_portal,    0}, //5, portal
	{&_build_party,     0}, //6, party
	{&_build_relation,  0}, //7, player relation
	{&_build_su_life,   0}, //8, pet or player life
	{&_build_cur_skill, 1}, //9, current skill
	{&_build_relpos,    1}, //10, rel pos, (x + y, y - x)
	{&_build_aura,      1}, //11, aura
	{&_build_level_pos, 0}, //12, player level_no, position
};

static PyObject* D2EM_GetUnit(PyObject *self, PyObject *args) {
	unsigned int idx = 0, type = 0, id = 0;
	if( !PyArg_ParseTuple(args, "|III", &idx, &type, &id) ) return 0;

	PFnCall_GetUnit fncall;
	PyObject *ret = 0;

	if( idx < sizeof(fncall_getunit_tbl) / sizeof(fncall_getunit_tbl[0]) ) {
		fncall = &fncall_getunit_tbl[idx];
		if(fncall->need_unit_obj) {
			D2Unit *cp = get_unit(type, id);
			if(cp) ret = fncall->fn(cp);

		} else {
			D2Unit vp;
			vp.type = type;
			vp.id = id;
			if(!type && !id) {
				D2Unit *me = d2client.GetCurrentPlayer();
				if(me) vp.id = me->id;
			}

			ret = fncall->fn(&vp);

		}

		if(ret) return ret;

	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetGoldMax(PyObject *self, PyObject *args) {
	D2Unit *cp = d2client.GetCurrentPlayer();
	if( !cp ) Py_RETURN_NONE;

	return PyInt_FromLong( d2common.GetGoldMax(cp) );
}

static PyObject* D2EM_GetGameInfo(PyObject *self, PyObject *args) {
	int info_type = 0;
	if( !PyArg_ParseTuple(args, "|i", &info_type) ) return 0;

	if(info_type == 0) {
		return PyLong_FromUnsignedLong( d2client.GetTick_LSR() );

	} else if(info_type == 1) {
		return PyLong_FromUnsignedLong( d2client.GetPingMs() );

	}

	Py_RETURN_NONE;
}

static PyObject* _get_item_mod_ptr(D2Unit* item) {
	D2StatTable *stb = d2common.CreateStatTable(item);
	if(stb){
		PyObject *ret = 0;
		if( !(stb->flag & 0x80000000) && stb->base_stat && stb->base_stat_size ) {
			ret = PyString_FromStringAndSize( (const char*)stb->base_stat, stb->base_stat_size * sizeof(D2Stat) );
		}

		d2common.FreeStatTable(stb);

		if(ret) return ret;
	}

	Py_RETURN_NONE;
}

static PyObject* _get_item_stat_ptr(D2Unit* item) {
	D2StatTable *stb = item->stat_table;

	if(stb) {
		D2Stat *stat;
		int size;

		if(stb->flag & 0x80000000) {
			stat = stb->stat;
			size = stb->stat_size;

		} else {
			stat = stb->base_stat;
			size = stb->base_stat_size;

		}

		if(stat && size) {
			return PyString_FromStringAndSize( (const char*)stat, size * sizeof(D2Stat) );
		}

	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetItemInfo(PyObject *self, PyObject *args) {
	unsigned int id;
	unsigned int info_type = 0;
	if( !PyArg_ParseTuple(args, "I|I", &id, &info_type) ) return 0;
	
	D2Unit * cp = get_unit(4, id);
	if( !cp || !cp->unit_ex ) Py_RETURN_NONE;

	PyObject *tp = PyTuple_New(5);
	PyObject *ptr = info_type ? _get_item_mod_ptr(cp) : _get_item_stat_ptr(cp);
	PyTuple_SET_ITEM(tp, 0, ptr );
	PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong( cp->cls_id ) );
	PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong( cp->unit_ex4->flag ) );
	PyTuple_SET_ITEM(tp, 3, PyInt_FromLong( cp->unit_ex4->quality ) );
	PyTuple_SET_ITEM(tp, 4, PyInt_FromLong( cp->unit_ex4->level ) );

	return tp;
}

static PyObject* D2EM_GetItemStat(PyObject *self, PyObject *args) {
	const char *ptr;
	unsigned int size;
	unsigned int idx;
	int mul = 0;
	if( !PyArg_ParseTuple(args, "s#I|i", &ptr, &size, &idx, &mul) ) return 0;

	if( !ptr ) Py_RETURN_NONE;

	D2StatTable stb;
	stb.flag = 0x80000000;
	stb.stat = (D2Stat*)ptr;
	stb.stat_size = (unsigned short)( size / sizeof(D2Stat) );

	//__dprintf("%p, %d, %x, %x\r\n", ptr, size, size / sizeof(D2Stat), idx);
	//dumphex(ptr, size);

	return _build_stat(&stb, idx, mul);
}



static PyObject* D2EM_IsAttackableEx(PyObject *self, PyObject *args) {
	unsigned int type;
	unsigned int id;
	if( !PyArg_ParseTuple(args, "II", &type, &id) ) return 0;

	D2Unit *me = d2client.GetCurrentPlayer();
	D2Unit *tar = get_unit(type, id);
	if(!me || !tar) Py_RETURN_FALSE;

	if( (tar->flag & 4) && d2client.IsAttackable(me, tar) ) { 
		Py_RETURN_TRUE;
	
	} else {
		Py_RETURN_FALSE;

	}

}

static PyObject* D2EM_DrawText(PyObject *self, PyObject *args) {
	unsigned int color, fmt;
	const char *str;
	RECT rect;

	if( !PyArg_ParseTuple(args, "sIiiiiI", &str, &color, &rect.left, &rect.top, &rect.right, &rect.bottom, &fmt) ) return 0;

	draw_text(str, color, &rect, fmt);

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetBeltSize(PyObject *self, PyObject *args) {
	return PyInt_FromLong( d2client.GetBeltSize() );
}

static PyObject* D2EM_GetWeaponWzNo(PyObject *self, PyObject *args) {
	return PyInt_FromLong( d2client.GetWeaponWzNo() );
}

static PyObject* D2EM_GetCorpseIds(PyObject *self, PyObject *args) {
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "|I", &id) ) return 0;

	D2Unit *cp = get_unit(0, id);
	if( !cp ) Py_RETURN_NONE;

	D2PlayerInfo *pi = find_player_info(cp->id);
	if(pi && pi->corpse) {
		PyObject *ls = PyList_New(0);
		PyObject *l;
		D2PlayerCorpse *corpse = pi->corpse;
		for(; corpse; corpse = corpse->next) {
			l = PyLong_FromUnsignedLong(corpse->corpse_id);
			PyList_Append(ls, l);
			Py_DECREF(l);

		}

		return ls;
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_DumpItemMods(PyObject *self, PyObject *args) {
	unsigned int id = 0;
	if( !PyArg_ParseTuple(args, "I", &id) ) return 0;

	D2Unit *item = get_unit(4, id);
	if(item) {
		D2StatTable *stb = d2common.CreateStatTable(item);
		if(stb){
			if( !(stb->flag & 0x80000000) && stb->base_stat && stb->base_stat_size ) {
				wchar_t buf[0x80];
				memset(buf, 0, sizeof(buf));
				short size = stb->base_stat_size;
				D2Stat *mod = stb->base_stat;

				__dprintf("Dump ItemMods[%08X], Count(%d):\r\n", id, size);
				while(size) {
					d2client.GetItemStatString(item, stb, mod->attr, mod->val, mod->id, buf);
					__dprintf("[%08X,%08X]: %S\r\n", mod->sid, mod->val, buf);

					mod++;
					size--;
				}
				__dprintf("--------------------\r\n");
			}

			d2common.FreeStatTable(stb);
		}

	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetSHM(PyObject *self, PyObject *args) {
	unsigned int type;
	if( !PyArg_ParseTuple(args, "I", &type) ) return 0;

	switch(type) {
		case 0x01:
			{
				return PyLong_FromUnsignedLong( d2em_shm->game_count );
			}
			break;

		case 0x02:
			{
				return PyLong_FromUnsignedLong( d2em_shm->srate );
			}
			break;

		case 0x03:
			{
				return PyString_FromString( d2em_shm->script );
			}
			break;

		case 0x04:
			{
				if(d2em_shm->script_config_size)
					return PyString_FromStringAndSize(d2em_shm->script_config, d2em_shm->script_config_size);
			}
			break;
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_SetSHM(PyObject *self, PyObject *args) {
	unsigned int type, val;
	if( !PyArg_ParseTuple(args, "II", &type, &val) ) return 0;

	switch(type) {
		case 0x01:
			{
				d2em_shm->game_count = val;
			}
			break;

		case 0x02:
			{
				d2em_shm->srate = val;
			}
			break;
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetPrivateProfileString(PyObject *self, PyObject *args) {
	const char *app_name, *key_name, *default_val, *file_name;
	if( !PyArg_ParseTuple(args, "ssss", &app_name, &key_name, &default_val, &file_name) ) return 0;

	if(app_name[0] == 0x00) app_name = 0;
	if(key_name[0] == 0x00) key_name = 0;
	if(default_val[0] == 0x00) default_val = 0;

	char buf[512];
	DWORD ret = GetPrivateProfileString(app_name, key_name, default_val, buf, sizeof(buf), file_name);
	if(ret) return PyString_FromStringAndSize(buf, ret);

	Py_RETURN_NONE;
}

static PyObject* D2EM_ReadMemory(PyObject *self, PyObject *args) {
	unsigned int mptr, size;
	if( !PyArg_ParseTuple(args, "II", &mptr, &size) ) return 0;

	char buf[1024];
	SIZE_T rbnum = 0;
	if( size <= sizeof(buf) && ReadProcessMemory(GetCurrentProcess(), (LPCVOID)mptr, buf, size, &rbnum) ) {
		if(rbnum) return PyString_FromStringAndSize(buf, rbnum);
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_GetModuleHandle(PyObject *self, PyObject *args) {
	const char *hname = 0;
	if( !PyArg_ParseTuple(args, "|s", &hname) ) return 0;

	if(hname[0] == 0x00) hname = 0;
	return PyLong_FromUnsignedLong( (unsigned int)GetModuleHandle(hname) );
}

static PyObject* D2EM_GetD2DataTables(PyObject *self, PyObject *args) {
	return PyLong_FromUnsignedLong( (unsigned long)d2client.GetDataTables() );
}

static PyObject* D2EM_GetD2StaticString(PyObject *self, PyObject *args) {
	unsigned int sid;
	if( !PyArg_ParseTuple(args, "I", &sid) ) return 0;

	wchar_t *ustr = d2lang.GetUnicodeString(sid);
	char buf[256];
	if(ustr && wcstombs(buf, ustr, sizeof(buf)) > 0) {
		buf[sizeof(buf) - 1] = 0x00;
		return PyString_FromString(buf);
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_D2DrawText(PyObject *self, PyObject *args) {
	const char *str;
	int x, y, color, unk=0;
	if( !PyArg_ParseTuple(args, "siii|i", &str, &x, &y, &color, &unk) ) return 0;

	wchar_t ustr[256];
	if( mbstowcs(ustr, str, sizeof(ustr) / sizeof(ustr[0])) > 0 ) {
		ustr[ sizeof(ustr) / sizeof(ustr[0]) - 1 ] = L'\0';
		d2win.D2DrawText(ustr, x, y, color, unk);
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_D2GetTextLength(PyObject *self, PyObject *args) {
	const char *str;
	if( !PyArg_ParseTuple(args, "s", &str) ) return 0;

	wchar_t ustr[256];
	int slen = strlen(str);
	int tlen = 0;
	if( mbstowcs(ustr, str, sizeof(ustr) / sizeof(ustr[0])) > 0 ) {
		ustr[ sizeof(ustr) / sizeof(ustr[0]) - 1 ] = L'\0';
		tlen = d2win.GetStringLength(ustr);
	}

	return PyInt_FromLong(tlen);
}

static PyObject* D2EM_D2SelectFont(PyObject *self, PyObject *args) {
	int val;
	if( !PyArg_ParseTuple(args, "i", &val) ) return 0;

	int old_val = d2win.SelectFont(val);
	return PyInt_FromLong(old_val);
}

static PyObject* D2EM_D2DrawRect(PyObject *self, PyObject *args) {
	int left, top, right, bottom, color, opaque;
	if( !PyArg_ParseTuple(args, "iiiiii", &left, &top, &right, &bottom, &color, &opaque) ) return 0;

	d2gfx.DrawRect(left, top, right, bottom, color, opaque);
	Py_RETURN_NONE;
}

static PyObject* D2EM_D2Print(PyObject *self, PyObject *args) {
	const char *str;
	int type = 0;
	if( !PyArg_ParseTuple(args, "s|i", &str, &type) ) return 0;

	wchar_t ustr[256];
	if( mbstowcs(ustr, str, sizeof(ustr) / sizeof(ustr[0])) > 0 ) {
		ustr[ sizeof(ustr) / sizeof(ustr[0]) - 1 ] = L'\0';
		d2client.D2Print(ustr, type);
	}

	Py_RETURN_NONE;
}

static PyObject* D2EM_ISMCtrl(PyObject *self, PyObject *args) {
	unsigned int k = 0;
	unsigned int v = 0;
	unsigned int p0 = 0;
	unsigned int p1 = 0;
	unsigned int p2 = 0;
	unsigned int p3 = 0;

	if( !PyArg_ParseTuple(args, "I|IIIII", &k, &v, &p0, &p1, &p2, &p3) ) return 0;

	switch(k) {
		case 0: //get info (sid, master_sid, num_shared_slots)
			{
				PyObject *tp = PyTuple_New(3);
				PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong( d2em_shm->sid ) );
				PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong( d2em_shm->master ) );
				PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong( ACONFIG_SIZE ) );

				return tp;
			}
			break;

		case 1: //is in game?
			{
				unsigned int c_seq_num = d2em_shm->seq_num;
				unsigned int o_seq_num = d2em_shm_master->ms_ss[ (v < ACONFIG_SIZE ? v : d2em_shm->master) ].seq_num;
				return PyInt_FromLong( o_seq_num && c_seq_num == o_seq_num );
			}
			break;

		case 2: //get data[4]
			{
				__MS_SharedSection *ms_ss = &d2em_shm_master->ms_ss[ (v < ACONFIG_SIZE ? v : d2em_shm->sid) ];
				PyObject *tp = PyTuple_New(4);
				PyTuple_SET_ITEM(tp, 0, PyLong_FromUnsignedLong( ms_ss->data[0] ) );
				PyTuple_SET_ITEM(tp, 1, PyLong_FromUnsignedLong( ms_ss->data[1] ) );
				PyTuple_SET_ITEM(tp, 2, PyLong_FromUnsignedLong( ms_ss->data[2] ) );
				PyTuple_SET_ITEM(tp, 3, PyLong_FromUnsignedLong( ms_ss->data[3] ) );

				return tp;
			}
			break;

		case 3: //set data[4]
			{
				__MS_SharedSection *ms_ss = &d2em_shm_master->ms_ss[ (v < ACONFIG_SIZE ? v : d2em_shm->sid) ];
				ms_ss->data[0] = p0;
				ms_ss->data[1] = p1;
				ms_ss->data[2] = p2;
				ms_ss->data[3] = p3;
			}
			break;
	}

	Py_RETURN_NONE;
}


static PyMethodDef D2EM_Py_Methods[] = {
	//{"Debug", (PyCFunction)D2EM_Debug, METH_NOARGS, 0},
	{"GetErrorCode", (PyCFunction)D2EM_GetErrorCode, METH_NOARGS, 0},
	{"GetExecPath", (PyCFunction)D2EM_GetExecPath, METH_NOARGS, 0},
	{"GetTickCount", (PyCFunction)D2EM_GetTickCount, METH_NOARGS, 0},
	{"GetTickInterval", (PyCFunction)D2EM_GetTickInterval, METH_VARARGS, 0},
	{"ASleep", (PyCFunction)D2EM_ASleep, METH_VARARGS, 0},
	{"TextOut", (PyCFunction)D2EM_TextOut, METH_VARARGS, 0},
	{"DrawText", (PyCFunction)D2EM_DrawText, METH_VARARGS, 0},
	{"PostMessage", (PyCFunction)D2EM_PostMessage, METH_VARARGS, 0},
	{"SendPacket", (PyCFunction)D2EM_SendPacket, METH_VARARGS, 0},
	{"GetPos", (PyCFunction)D2EM_GetPos, METH_VARARGS, 0},
	{"GetFreeGrid", (PyCFunction)D2EM_GetFreeGrid, METH_VARARGS, 0},
	{"GetStaticUnits", (PyCFunction)D2EM_GetStaticUnits, METH_VARARGS, 0},
	{"GetPointInMap", (PyCFunction)D2EM_GetPointInMap, METH_VARARGS, 0},
	{"GetFlag", (PyCFunction)D2EM_GetFlag, METH_VARARGS, 0},
	{"GetTeleportPath", (PyCFunction)D2EM_GetTeleportPath, METH_VARARGS, 0},
	{"GetWalkPath", (PyCFunction)D2EM_GetWalkPath, METH_VARARGS, 0},
	{"GetNPCDialog", (PyCFunction)D2EM_GetNPCDialog, METH_NOARGS, 0},
	{"GetWindowSize", (PyCFunction)D2EM_GetWindowSize, METH_NOARGS, 0},
	{"GetStat", (PyCFunction)D2EM_GetStat, METH_VARARGS, 0},
	{"GetAllStats", (PyCFunction)D2EM_GetAllStats, METH_VARARGS, 0},
	{"GetSkill", (PyCFunction)D2EM_GetSkill, METH_VARARGS, 0},
	{"GetSkillBySide", (PyCFunction)D2EM_GetSkillBySide, METH_VARARGS, 0},
	{"GetAllSkills", (PyCFunction)D2EM_GetAllSkills, METH_NOARGS, 0},
	{"GetInvInfo", (PyCFunction)D2EM_GetInvInfo, METH_VARARGS, 0},
	{"GetMapFromInv", (PyCFunction)D2EM_GetMapFromInv, METH_VARARGS, 0},
	{"GetItemsFromInv", (PyCFunction)D2EM_GetItemsFromInv, METH_VARARGS, 0},
	{"ItemCodeToClsId", (PyCFunction)D2EM_ItemCodeToClsId, METH_VARARGS, 0},
	{"GetItemSize", (PyCFunction)D2EM_GetItemSize, METH_VARARGS, 0},
	{"GetRoom", (PyCFunction)D2EM_GetCurrentRoom, METH_VARARGS, 0},
	{"GetCurrentMap", (PyCFunction)D2EM_GetCurrentMap, METH_VARARGS, 0},
	{"GetAllRooms", (PyCFunction)D2EM_GetAllRooms, METH_VARARGS, 0},
	{"GetNearbyRooms", (PyCFunction)D2EM_GetNearbyRooms, METH_NOARGS, 0},
	{"GetNearbyUnits", (PyCFunction)D2EM_GetNearbyUnits, METH_VARARGS, 0},
	{"GetUnit", (PyCFunction)D2EM_GetUnit, METH_VARARGS, 0},
	{"GetItemPrice", (PyCFunction)D2EM_GetItemPrice, METH_VARARGS, 0},
	{"GetGoldMax", (PyCFunction)D2EM_GetGoldMax, METH_NOARGS, 0},
	{"GetGameInfo", (PyCFunction)D2EM_GetGameInfo, METH_VARARGS, 0},
	{"GetItemInfo", (PyCFunction)D2EM_GetItemInfo, METH_VARARGS, 0},
	{"GetItemStat", (PyCFunction)D2EM_GetItemStat, METH_VARARGS, 0},
	{"IsAttackableEx", (PyCFunction)D2EM_IsAttackableEx, METH_VARARGS, 0},
	{"GetBeltSize", (PyCFunction)D2EM_GetBeltSize, METH_NOARGS, 0},
	{"GetWeaponWzNo", (PyCFunction)D2EM_GetWeaponWzNo, METH_NOARGS, 0},
	{"GetCorpseIds", (PyCFunction)D2EM_GetCorpseIds, METH_VARARGS, 0},
	{"DumpCurrentMap", (PyCFunction)D2EM_DumpCurrentMap, METH_VARARGS, 0},
	{"DumpItemMods", (PyCFunction)D2EM_DumpItemMods, METH_VARARGS, 0},
	{"GetSHM", (PyCFunction)D2EM_GetSHM, METH_VARARGS, 0},
	{"SetSHM", (PyCFunction)D2EM_SetSHM, METH_VARARGS, 0},
	{"GetPrivateProfileString", (PyCFunction)D2EM_GetPrivateProfileString, METH_VARARGS, 0},
	{"ReadMemory", (PyCFunction)D2EM_ReadMemory, METH_VARARGS, 0},
	{"GetModuleHandle", (PyCFunction)D2EM_GetModuleHandle, METH_VARARGS, 0},
	{"GetD2DataTables", (PyCFunction)D2EM_GetD2DataTables, METH_NOARGS, 0},
	{"GetD2StaticString", (PyCFunction)D2EM_GetD2StaticString, METH_VARARGS, 0},
	
	{"D2DrawText", (PyCFunction)D2EM_D2DrawText, METH_VARARGS, 0},
	{"D2GetTextLength", (PyCFunction)D2EM_D2GetTextLength, METH_VARARGS, 0},
	{"D2SelectFont", (PyCFunction)D2EM_D2SelectFont, METH_VARARGS, 0},
	{"D2DrawRect", (PyCFunction)D2EM_D2DrawRect, METH_VARARGS, 0},
	{"D2Print", (PyCFunction)D2EM_D2Print, METH_VARARGS, 0},

	{"ISMCtrl", (PyCFunction)D2EM_ISMCtrl, METH_VARARGS, 0},

	{0}
};

void Init_D2EM_Py_Module() {
	Py_InitModule("_D2EM", D2EM_Py_Methods);

}

void Init_D2EM_Py() {
	Py_NoSiteFlag++;
	Py_DontWriteBytecodeFlag++;

	PyImport_AppendInittab("_D2EM", &Init_D2EM_Py_Module);
	Py_Initialize();

	PyObject *py_out = PyFile_FromFile( (FILE*)get_output_file(), "LOG", "a", 0);
	if(py_out) {
		PySys_SetObject("stdout", py_out);
		PySys_SetObject("stderr", py_out);

		Py_DECREF(py_out);
	}
	
	const char *dirp = get_dll_path();
	strcat(sys_path, dirp);
	strcat(sys_path, "\\pylib.zip;");
	strcat(sys_path, dirp);
	strcat(sys_path, "\\core.zip;");
	strcat(sys_path, dirp);
	strcat(sys_path, "\\core;");
	strcat(sys_path, dirp);
	strcat(sys_path, "\\lib.zip;");
	strcat(sys_path, dirp);
	strcat(sys_path, "\\lib;");

	PySys_SetPath(sys_path);

}

void Del_D2EM_Py() {
	Py_Finalize();

}

