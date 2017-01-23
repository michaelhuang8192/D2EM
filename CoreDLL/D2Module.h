#ifndef __D2MODULE__
#define __D2MODULE__

#include <windows.h>

#pragma pack(push)
#pragma pack(1)

struct D2Unit;
struct D2Level;
struct D2Room;
struct D2RoomEx;
struct D2CollisionMap;


//++ D2 Data Struct
struct D2DialogEntry {
	unsigned int _00;
	unsigned int _flag;
	wchar_t name[0x78];
	unsigned int height; //0x108;
	unsigned _11C[0x05];

};

struct D2Dialog {
	unsigned int tick;
	int _04[0x03];
	int _flag;	//0x10
	int _14[0x06];
	unsigned int sx; //0x2c, (+0x0F, -0x0F)
	unsigned int sy;
	unsigned int width; // (-0x0B, +0x04)
	unsigned int height;
	unsigned int _3C[0x05]; //0x3c
	unsigned int num_entry;
	unsigned int _54[0x03];
	D2DialogEntry entrys[0x01]; //0x60
	//0xB0C //next

};


struct D2Skill {
	struct {
		unsigned short skill_id;

	} *data;

	D2Skill *next;
	int _flag; //type1: data[0x11], type2+,type0: data[0x10], type0+skill5: 0x10
	int _0C[0x07];
	unsigned int level; //0x28
	int _2C;
	unsigned int quantity;
	unsigned int flag;
	unsigned int charge;
};

struct D2SkillTable {
	int _00;
	D2Skill *skill_list;
	D2Skill *left_hand;
	D2Skill *right_hand;
	D2Skill *cur_skill;
};

struct D2Path0 {
	union {
		int x_s;
		struct {
			unsigned short _0x;
			unsigned short x;
		};
	};
	union {
		int y_s;
		struct {
			unsigned short _0y;
			unsigned short y;
		};
	};

	int rx;
	int ry;

	union {
		struct {
			unsigned short x;
			unsigned short y;
		};
		unsigned int val;
	} dst[3];

	D2RoomEx *room_ex;
	D2RoomEx *_r;
	int pt_idx;
	int pt_max;
	int _2C;
	D2Unit *unit;
	unsigned int flag_0;
	int _38;
	unsigned int flag_1;
	int _40[0x0C];
	unsigned short _70;
	int delta_x; //0x72
	int delta_y; //0x76
	unsigned short _7A;
	int _7C[0x08];
	struct {
		unsigned int x;
		unsigned int y;
	} pt[1];

};

struct D2Path2 {
	D2Room *room;
	int rx;
	int ry;
	unsigned short x;
	unsigned short _0E;
	unsigned short y;
	unsigned short _12;

};

/*
UNITITEM-> [0x14] -> +0x18(type), +0x5C(inventory_id)
0x5C -> [0x08](4, id), [0x44] -> item
*/

struct D2Inventory {
	D2Unit *head;
	D2Unit *tail;
	unsigned char w;
	unsigned char h;
	unsigned char _0A[0x02];
	D2Unit **map;
};

struct D2InventoryTable {
	//int _00[0x02]; //0x01020304, 0x00
	unsigned int magic;
	int _04;
	D2Unit *owner; //0x08
	D2Unit *head;
	D2Unit *tail;
	D2Inventory *inv;
	int num_inv;
	int _1C;
	D2Unit *cursor;
	int _24;
	int count;
};

struct D2UnitEx4 {
	int quality;
	int _04[0x02];
	unsigned int owner_id;
	int _10[0x02];
	unsigned int flag; //0x18
	int _1C;
	int _20[0x03];
	int level;
	int version;
	int _34;
	unsigned short ex_cls_id; //0x38
	unsigned short _0x3A;
	int _3C[0x02];
	int dst0;
	int _48[0x05];
	D2InventoryTable *inv_table;
	D2Unit *prev_a;
	D2Unit *next_a;
	int dst1;
	D2Unit *prev_c;
	D2Unit *next_c;
};

struct D2UnitEx0 {
	char name[0x10];

};

struct D2UnitEx1 {
	int _00[0x05];
	unsigned short u_cls_id;
	unsigned int flag; //0x16
	unsigned short _0A; //0x1A
	unsigned char attrs[0x09]; //0x1C
};

struct D2UnitEx3 {
	int _00[0x06];
	unsigned int owner_type;
	unsigned int owner_id;
};

struct D2Stat {
	union {
		struct {
			unsigned short attr;
			unsigned short id;
		};
		unsigned int sid;
	};
	unsigned int val;
};

struct D2StatTable {
	int _00;
	D2Unit *parent_unit;
	unsigned int type;
	unsigned int id;
	int flag;
	int _14[0x04];
	D2Stat *base_stat; //0x24
	short base_stat_size;
	short base_stat_buf_size;

	D2StatTable *next;
	D2StatTable *prev;
	D2StatTable *parent_stb;
	int _38;
	D2StatTable *child_stb;
	int _40;
	D2Unit *unit;

	D2Stat *stat;
	short stat_size;
	short stat_buf_size;

	int _50[2];
	unsigned char *aura;
};

struct D2Unit {
	unsigned int type;
	unsigned int cls_id;
	int alloc_flag;
	unsigned int id;
	unsigned int state; //0x10
	
	union {
		D2UnitEx4 *unit_ex4;
		D2UnitEx0 *unit_ex0;
		D2UnitEx1 *unit_ex1;
		D2UnitEx3 *unit_ex3;
		void *unit_ex;
	};

	int _18;
	
	//0x1c
	struct {
			int _00[0x0D];
			D2RoomEx *next_roomex;
	} *_r0;

	int _20[0x03];
	
	union {
		D2Path0 *path0;
		D2Path2 *path2;
		void *path;
	};

	int _0x30[0x0B];
	D2StatTable *stat_table;
	D2InventoryTable *inv_table;

	int _64[0x11];

	D2SkillTable *skill_table; //0xA8
	int _AC[0x06];

	unsigned int flag; //0xC8
	int _CC[0x04];

	struct { //0xD8
		int max_packet;
		int cur_idx;
		int _08;
		void *_packet_buf;
	} *_delay_packet;

	int _DC[0x02];

	D2Unit *next0; //0xe4, in same hash table
	D2Unit *next1; //0xe8, in same room
};

struct D2PetRef {
	unsigned int cls_id;
	unsigned int flag; //
	unsigned int id;
	unsigned int player_id;
	int _10[0x03];
	unsigned int life_percent;
	int _20[0x04];
	D2PetRef *next;

};

struct D2PlayerCorpse {
	unsigned int corpse_id;
	D2PlayerCorpse *next;
};

struct D2PlayerRelation {
	unsigned int player_id;
	unsigned int relation;
	D2PlayerRelation *next;
};

struct D2PlayerInfo {
	const char player_name[16];
	unsigned int player_id;
	int life;
	int _18[0x02];
	short level;
	short party_id;
	//int _24[0x03];
	unsigned int level_no;
	unsigned int x;
	unsigned int y;
	int party_state;
	D2PlayerRelation **relation;
	D2PlayerCorpse *corpse;
	unsigned int portal_id0;
	unsigned int portal_id1;
	int _44[0x0F];
	D2PlayerInfo *next;

};

struct D2CollisionMap {
	unsigned int x_m;
	unsigned int y_m;
	unsigned int sx_m;
	unsigned int sy_m;
	unsigned int x;
	unsigned int y;
	unsigned int sx;
	unsigned int sy;
	unsigned short *map;
};

struct D2RoomEx {
	D2RoomEx **near_room_ex;
	int _04[0x05];
	unsigned int x_m;
	unsigned int y_m;
	unsigned int sx_m;
	unsigned int sy_m;
	unsigned int x;
	unsigned int y;
	unsigned int sx;
	unsigned int sy;
	D2RoomEx *next;
	int _3C;
	D2Unit *unit;
	int _44[0x03];
	D2CollisionMap *collisn_map;
	int _54[0x05];
	void *map_unit;
	int _6C;
	D2Room *room;
	int _74;
	int num_near_room_ex;
};

struct D2RoomUnit5Ref {
	struct {
		unsigned int cls_id;
	} *data;
	D2Room *dst_room;
	int _0x08[0x02];
	D2RoomUnit5Ref *next;
};

struct D2RoomUnit {
	unsigned int cls_id;
	int _04[0x02];
	unsigned int x;
	int _10;
	unsigned int y;
	D2RoomUnit *next;
	unsigned int type;
};

struct D2Room {
	D2Level *level;
	int _04;
	int num_near_room;
	D2RoomUnit5Ref *room_unit5_ref;
	D2Room **near_room;
	int _14[0x03];
	unsigned short _ref0;
	unsigned short _ref1;
	unsigned short _ref2;
	unsigned short _ref3;
	D2Room *prev;
	unsigned int x;
	unsigned int y;
	unsigned int sx;
	unsigned int sy;
	void *preset;
	int _40[0x21];//0x40
	D2RoomUnit *room_unit;
	unsigned int _ref_type; //0xC8
	int _CC;
	void *map_objs; //0xD0, +14 ->Collisn[68]
	D2Room *next_over_all;
	D2RoomEx *room_ex;
	int _DC[0x02];
	int flag; //0xE4
	D2Room *next;
};

struct D2Level {
	int _00[0x17];
	D2Level *next; //0x5C
	int _60;

	//0x64
	struct {
		int _00[0x03];
		D2Room ref_room[4];
	} *_r0;

	int _68;
	unsigned int x; //0x6C
	unsigned int y;
	unsigned int sx;
	unsigned int sy;
	int _7C[0x06];
	unsigned int level_no;
	int _98[0x61];
	D2Room *room;
	int _220;
	int num_room;

};


#pragma pack(pop)


//--

#define D2CLIENTBASE 0x6FAB0000
#define D2WINBASE 0x6F8E0000
#define D2COMMONBASE 0x6FD50000
#define D2GFXBASE 0x6FA80000
#define D2BNCLIENTBASE 0x6FF20000
#define D2MCPCLIENTBASE 0x6FA20000

class D2ModuleBase {
public:
	HMODULE _base;
	virtual void InitBase(HMODULE base) { _base = base; };
};

class D2Bnclient : public D2ModuleBase {
public:
	int GetPacketState(int packet_id);
};

class D2Game : public D2ModuleBase {
};

class D2Gdi : public D2ModuleBase {
};


//onclick - sub_6FB0C7E0
//isdead - sub_6FACEAC0
class D2Client : public D2ModuleBase {
public:
	void InitBase(HMODULE base);

	unsigned int GetPanelFlag(unsigned int idx);
	D2Unit* GetCurrentPlayer(); //dword_6FBCC3D0
	D2Unit* GetUnit(unsigned int type, unsigned int id); //sub_6FB154F0
	D2PetRef* GetPetRefList(); //dword_06FBCB97C
	D2PlayerInfo* GetPlayerInfo(); //dword_6FBCC080
	void SetFocusOnUnit(unsigned int type, unsigned int id);
	int GetWeaponWzNo();
	D2Dialog* GetNPCDialog(); //0x6FBCC18C
	void GetWindowSize(int *sx, int *sy, int *center_rel, int *rcx, int *rcy);
	void* GetGLBSellBuyData();
	unsigned int GetDiffcultyLevel();
	D2Unit* GetCursorUnit();
	unsigned int GetPingMs();
	wchar_t* GetAttrName(unsigned int attr_id);
	int GetEscKeyState();
	int GetBeltSize();
	void* GetWndStatePtr();
	unsigned int GetTick_LSR();
	void* GetDataTables();


	void (__stdcall *D2Print)(wchar_t *text, int type);
	void (__stdcall *RevealRoom)(D2RoomEx *room_ex, int whole_room, void *auto_map);

	//void (__stdcall *SyncScreenOffset)(); //0x6FB0DB10

	int (__stdcall *TestPlayerState)(D2Unit *unit, D2Skill *skill);
	DWORD _TestPlayerState;

	int (__stdcall *GetCastStatus)(D2Unit *unit, D2Skill *skill);
	DWORD _GetCastStatus;

	void (__stdcall *ProcessPacket)(const char*, int);
	DWORD _ProcessPacket;

	wchar_t* (__stdcall *GetName)(D2Unit *unit);
	DWORD _GetName;

	int (__stdcall *IsAttackable)(D2Unit *me, D2Unit *target);
	DWORD _IsAttackable;

	void (__stdcall *GetItemStatString)(D2Unit *item, D2StatTable *stb, unsigned int attr, unsigned int val, unsigned int sid, wchar_t *buf);
	DWORD _GetItemStatString;

	void* (__stdcall *GetAutoMap)(int automap_clsid); //d2common.10301[8]
	DWORD _GetAutoMap;
};

class D2Net : public D2ModuleBase {
public:
	int (__stdcall *SendPacket)(int, int, const char*);
	int (__stdcall *RecvPacket0)(const char*, int);
	int (__stdcall *RecvPacket1)(const char*, int);
	int (__stdcall *GetStatus)();

	void InitBase(HMODULE base);
};

class D2gfx : public D2ModuleBase {
public:
	HWND (__stdcall *GetHWND)();
	int (__stdcall *HideWindow)(int hidden);
	void (__stdcall *DrawRect)(int left, int top, int right, int bottom, int color, int opaque);

	int IsHidden();

	void InitBase(HMODULE base);
};

class D2Win : public D2ModuleBase {
public:
	void* GetWndProcVar();
	int IsWndProcDisable();
	void* GetWndStatePtr();

	int (__fastcall *GetStringLength)(wchar_t *str);
	void (__fastcall *D2DrawText)(wchar_t *str, int x, int y, int color, int unk);
	int (__fastcall *SelectFont)(int font_id);

	void InitBase(HMODULE base);
};

class D2Common : public D2ModuleBase {
public:
	//10126 get left skill
	//10566 get right skill
	//10078(Inventory);
	//10796(Inventory, item); is belong to
	//10113 get required level
	//10989 test item flag
	//10366 get roomex
	//10511 get lvlno
	//10438 is in town
	//11166 get level numer by wrap

	//++AutoMap
	void (__stdcall *GetUnit1Data)(D2Unit *unit, void *p1, void *p2, void *p3, void *p4, void *p5);
	//--

	unsigned int (__stdcall *GetSkillStatus)(D2Unit *unit, D2Skill *skill); //10715
	void* (__stdcall *GetClassData4)(unsigned int cls_id); //10887 (x:10F, y:110)
	D2RoomEx* (__stdcall *GetRoomEx)(D2Unit *unit);
	void* (__fastcall *GetMapLevelData)(int level_no);

	//type: enum{ buy = 0, sell, gamble, repair }
	int (__stdcall *GetUnit4Price)(D2Unit *unit, D2Unit *unit4, unsigned int diff_lvl, void *data, unsigned int npc_clsid, int type);
	int (__stdcall *GetGoldMax)(D2Unit *unit);
	int (__stdcall *IsTown)(D2RoomEx *rex);
	int (__stdcall *IsDemon)(D2Unit *unit); //10790
	int (__stdcall *IsUndead)(D2Unit *unit); //10455

	void (__stdcall *GetBeltData)(int belt_val, int gfx_val, unsigned long *buf);
	unsigned int (__stdcall *GetNormalSkillLevel)(D2Unit *me, D2Skill *skill, int flag); //flag = 1 always
	int (__stdcall *IsSpecialSkill)(D2Skill *skill, unsigned int *flag, unsigned int *id, unsigned int *level, unsigned int *charge);

	unsigned int CodeToClsId(unsigned int code); //6FDF07EC
	int GetPosition(D2Unit *unit, unsigned int *x, unsigned int *y);
	int GetItemPrice(D2Unit *player, D2Unit *item, unsigned int npc_clsid, int type);
	unsigned int GetSkillLevel(D2Unit *me, D2Skill *skill);
	unsigned int GetSkillMana(D2Unit *me, D2Skill *skill);

	D2StatTable* (__stdcall *AllocStatTable)(int alloc_flag, unsigned int flag, int unk0, unsigned int type, unsigned int id); //10235
	void (__stdcall *FreeStatTable)(D2StatTable* ptr); //10049
	D2StatTable* (__stdcall *FindStatTable)(D2Unit *target, unsigned int key, unsigned int flag); //10611
	void (__stdcall *FillStatTable)(D2StatTable *dst, D2StatTable *src); //10911
	D2StatTable* CreateStatTable(D2Unit *target);

	int (__stdcall *FindFreeGrid)(void *inv, int *x, int *y, int w, int h);
	DWORD _FindFreeGrid;

	int (__stdcall *GetNextPosDelta)(D2Path0 *path, int *pt); //sub_6FD9A780
	DWORD _GetNextPosDelta;

	void InitBase(HMODULE base);
};

class D2Fog : public D2ModuleBase {
public:
	/*
	10073 - recv realm packet
	10072 - fetch realm packet

	*/
	unsigned int (__stdcall *BinarySearch)(void *data, unsigned int key, int unused);
	int (__stdcall *RecvPacket_Realm)(void *ptr, char *buf, int size);

	void InitBase(HMODULE base);
};

class D2Lang : public D2ModuleBase {
public:
	wchar_t* (__fastcall *GetUnicodeString)(unsigned int id);

	void InitBase(HMODULE base);
};

class D2MCPClient : public D2ModuleBase {
public:
	/*
	10003 - process realm procket
	*/
	
	unsigned int GetRealmLoginResCode();
};

extern D2Game d2game;
extern D2Net d2net;
extern D2Client d2client;
extern D2Gdi d2gdi;
extern D2gfx d2gfx;
extern D2Win d2win;
extern D2Common d2common;
extern D2Fog d2fog;
extern D2Lang d2lang;
extern D2Bnclient d2bnclient;
extern D2MCPClient d2mcpclient;

//D2Game - 10050: set fps
/*
D2Client
6FB2CEE0: main loop
6FBCBFF8: mode, single player(0), mul(8), bnet(3), openbnet(6)
6FB39BB0: bottom_stat
6FBCC1D0: window flag

D2Common.10628 - act2-duriel
*/

#endif

