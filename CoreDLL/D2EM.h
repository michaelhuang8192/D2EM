#ifndef __D2EM__
#define __D2EM__

#include <windows.h>
#include "D2Module.h"


struct D2RU5 {
	unsigned int cls_id;
	unsigned int dst;
	D2RU5 *next;
};

struct D2RU {
	unsigned int type;
	unsigned int cls_id;
	unsigned int x;
	unsigned int y;
	D2RU *next;
};

struct D2RM {
	unsigned int level_no;
	unsigned int x;
	unsigned int y;
	unsigned int sx;
	unsigned int sy;
};

struct D2RMEX {
	D2Room *room;
	D2RM rm;
	D2RM *nbrm;
	int nbrm_count;
};

struct D2LevelMap {
	unsigned int level_no;
	unsigned int x;
	unsigned int y;
	unsigned int sx;
	unsigned int sy;
	unsigned short *map;

	D2RU5 *ru5;
	int ru5_count;

	D2RU *ru;
	int ru_count;

	D2RMEX *rmex;
	int rmex_count;
};

void Init_D2EM();
void Del_D2EM();

void __dprintf(const char *fmt, ...);
void dumphex(const char *buf, int size);
void *get_output_file();
const char *get_dll_path();

int OnSendPacket(int size, int type, const char *buf);
int OnRecvPacket(int size, int type, const char *buf);
void OnDraw(HDC hdc);
void OnExit();
void YieldControl();

void OnRecvPacket_Realm(const char *buf, int size);

#endif
