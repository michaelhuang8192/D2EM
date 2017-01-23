#include "D2Module.h"

D2Client d2client;
D2Game d2game;
D2Net d2net;
D2Gdi d2gdi;
D2gfx d2gfx;
D2Win d2win;
D2Common d2common;
D2Fog d2fog;
D2Lang d2lang;
D2Bnclient d2bnclient;
D2MCPClient d2mcpclient;


#define D2CLIENT_PTR(ptr) ((DWORD)_base + ptr - D2CLIENTBASE)
#define D2WIN_PTR(ptr) ((DWORD)_base + ptr - D2WINBASE)
#define D2COMMON_PTR(ptr) ((DWORD)_base + ptr - D2COMMONBASE)
#define D2GFX_PTR(ptr) ((DWORD)_base + ptr - D2GFXBASE)
#define D2BNCLIENT_PTR(ptr) ((DWORD)_base + ptr - D2BNCLIENTBASE)
#define D2MCPCLIENT_PTR(ptr) ((DWORD)_base + ptr - D2MCPCLIENTBASE)


__declspec(naked) static void __stdcall D2Client_ProcessPacket(const char *buf, int size) {
	__asm {
		mov eax, [esp + 0x04]; //buf
		push [esp + 0x08]; //size
		mov ecx, d2client._ProcessPacket;
		call ecx;
		retn 0x08;
	}

}

__declspec(naked) static int __stdcall D2Client_TestPlayerState(D2Unit *unit, D2Skill *skill) {
	__asm {
		mov eax, [esp + 0x04]; //unit
		push [esp + 0x08]; //skill
		mov ecx, d2client._TestPlayerState;
		call ecx;
		retn 0x08;
	}

}

__declspec(naked) static int __stdcall D2Client_GetCastStatus(D2Unit *unit, D2Skill *skill) {
	__asm {
		push esi;
		mov esi, [esp + 0x08]; //unit
		mov eax, [esp + 0x0C]; //skill
		mov ecx, d2client._GetCastStatus;
		call ecx;
		pop esi;
		retn 0x08;
	}

}

__declspec(naked) static int __stdcall D2Common_FindFreeGrid(void *inv, int *x, int *y, int w, int h) {
	__asm {
		push ebx;
		mov ebx, [esp + 0x18];
		push [esp + 0x14];
		push [esp + 0x14];
		push [esp + 0x14];
		push [esp + 0x14];
		mov eax, d2common._FindFreeGrid;
		call eax;
		pop ebx;
		retn 0x14;
	};

}

__declspec(naked) static int __stdcall D2Common_GetNextPosDelta(D2Path0 *path, int *pt) {
	__asm {
		mov edx, [esp + 0x04];
		push [esp + 0x08];
		mov eax, d2common._GetNextPosDelta;
		call eax;
		retn 0x08;
	};

}

__declspec(naked) static wchar_t* __stdcall D2Client_GetName(D2Unit *unit) {
	__asm {
		mov eax, [esp + 0x04];
		mov ecx, d2client._GetName;
		call ecx;
		retn 0x04;
	};

}

__declspec(naked) static int __stdcall D2Client_IsAttackable(D2Unit *me, D2Unit *target) {
	__asm {
		mov eax, [esp + 0x08];
		push [esp + 0x04];
		mov ecx, d2client._IsAttackable;
		call ecx;
		retn 0x08;
	};

}

__declspec(naked) static void __stdcall D2Client_GetItemStatString(D2Unit *item, D2StatTable *stb, unsigned int attr, unsigned int val, unsigned int id, wchar_t *buf) {
	__asm {
		push esi;
		mov esi, [esp + 0x1C];
		mov eax, [esp + 0x18];
		push [esp + 0x14];
		push [esp + 0x14];
		push [esp + 0x14];
		push [esp + 0x14];
		mov ecx, d2client._GetItemStatString;
		call ecx;
		pop esi;
		retn 0x18;
	};

}

__declspec(naked) static void* __stdcall D2Client_GetAutoMap(int automap_clsid) {
	__asm {
		mov eax, [esp + 0x04];
		mov ecx, d2client._GetAutoMap;
		call ecx;
		retn 0x04;
	};

}

void D2Client::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	_ProcessPacket = D2CLIENT_PTR(0x6FB150D0);
	ProcessPacket = &D2Client_ProcessPacket;

	_TestPlayerState = D2CLIENT_PTR(0x6FACEB20);
	TestPlayerState = &D2Client_TestPlayerState;

	_GetCastStatus = D2CLIENT_PTR(0x6FB53610);
	GetCastStatus = &D2Client_GetCastStatus;

	//SyncScreenOffset = (void(__stdcall*)())D2CLIENT_PTR(0x6FB0DB10);
	*(DWORD*)&D2Print = D2CLIENT_PTR(0x6FB21740);
	*(DWORD*)&RevealRoom = D2CLIENT_PTR(0x6FAF04C0);

	_GetName = D2CLIENT_PTR(0x6FACF3D0);
	GetName = &D2Client_GetName;

	_IsAttackable = D2CLIENT_PTR(0x6FACF980);
	IsAttackable = &D2Client_IsAttackable;

	_GetItemStatString = D2CLIENT_PTR(0x6FB38EA0);
	GetItemStatString = &D2Client_GetItemStatString;

	_GetAutoMap = D2CLIENT_PTR(0x6FAF0650);
	GetAutoMap = &D2Client_GetAutoMap;

}

void* D2Client::GetGLBSellBuyData() {
	return (void*)( *(DWORD*)D2CLIENT_PTR(0x6FBCB643) );
}

unsigned int D2Client::GetDiffcultyLevel() {
	return ( *(unsigned char*)D2CLIENT_PTR(0x6FBCBFF4) );
}

void* D2Client::GetWndStatePtr() {
	return (void*)D2CLIENT_PTR(0x6FB93A80);
}

unsigned int D2Client::GetTick_LSR() {
	return *(unsigned int*)D2CLIENT_PTR(0x6FBB32FC);
}

void D2Net::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	*(FARPROC*)&GetStatus = GetProcAddress(base, (LPCSTR)10001);
	*(FARPROC*)&RecvPacket1 = GetProcAddress(base, (LPCSTR)10009);
	*(FARPROC*)&SendPacket = GetProcAddress(base, (LPCSTR)10036);
	*(FARPROC*)&RecvPacket0 = GetProcAddress(base, (LPCSTR)10019);

}

void D2gfx::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	*(FARPROC*)&GetHWND = GetProcAddress(base, (LPCSTR)10078);
	*(FARPROC*)&HideWindow = GetProcAddress(base, (LPCSTR)10058);
	*(FARPROC*)&DrawRect = GetProcAddress(base, (LPCSTR)10062);

}

int D2gfx::IsHidden() {
	return *(int*)D2GFX_PTR(0x6FA9D65C);
}


unsigned int D2Client::GetPanelFlag(unsigned int idx) {
	if( idx < 0x26 ) {
		DWORD *fv = (DWORD*)D2CLIENT_PTR(0x6FBB2B58);
		return fv[idx];

	}

	return 0;
}

D2Unit* D2Client::GetCurrentPlayer() {
	return (D2Unit*)(*(DWORD*)D2CLIENT_PTR(0x6FBCC3D0));

}

D2Dialog* D2Client::GetNPCDialog() {
	return (D2Dialog*)(*(DWORD*)D2CLIENT_PTR(0x6FBCC18C));

}

void D2Client::GetWindowSize(int *sx, int *sy, int *center_rel, int *rcx, int *rcy) {
	*sx = (*(int*)D2CLIENT_PTR(0x6FB8C6E0));
	*sy = (*(int*)D2CLIENT_PTR(0x6FB8C6E4));
	*center_rel = (*(int*)D2CLIENT_PTR(0x6FBCC1D4));
	*rcx = (*(int*)D2CLIENT_PTR(0x6FBC21D0));
	*rcy = (*(int*)D2CLIENT_PTR(0x6FBC21CC));
}

int D2Client::GetWeaponWzNo() {
	return *(int*)D2CLIENT_PTR(0x6FBCBA58);
}

D2Unit* D2Client::GetUnit(unsigned int type, unsigned int id) {
	if(type >= 6) return 0;

	D2Unit **htb;
	if(type != 3) {
		htb = (D2Unit**)(D2CLIENT_PTR(0x6FBCA960) + (type << 9));
	} else {
		htb = (D2Unit**)(D2CLIENT_PTR(0x6FBC9D60) + (type << 9));
	}

	D2Unit *cur = htb[ (id & 0x7F) ];

	while(cur) {
		if(cur->id == id && cur->type == type) break;

		cur = cur->next0;
	}

	return cur;
}

D2PetRef* D2Client::GetPetRefList() {
	return (D2PetRef*)(*(DWORD*)D2CLIENT_PTR(0x06FBCB97C));
}

D2PlayerInfo* D2Client::GetPlayerInfo() {
	return (D2PlayerInfo*)(*(DWORD*)D2CLIENT_PTR(0x6FBCC080));
}

void D2Client::SetFocusOnUnit(unsigned int type, unsigned int id) {
	(*(DWORD*)D2CLIENT_PTR(0x6FBB5554)) = type;
	(*(DWORD*)D2CLIENT_PTR(0x6FBB5540)) = id;
	(*(DWORD*)D2CLIENT_PTR(0x6FBCC118)) = 1;

}

D2Unit* D2Client::GetCursorUnit() {
	return (D2Unit*)(*(DWORD*)D2CLIENT_PTR(0x6FBB1644));
}

unsigned int D2Client::GetPingMs() {
	return (*(unsigned int*)D2CLIENT_PTR(0x6FBB32BC));
}

wchar_t* D2Client::GetAttrName(unsigned int attr_id) {
	unsigned short l_id = *( (unsigned short*)D2CLIENT_PTR(0x6FBA9E68) + (attr_id & 0xFF) );
	if( !l_id ) return 0;

	return d2lang.GetUnicodeString(l_id);
}

int D2Client::GetEscKeyState() {
	int s0 = ( (int(__stdcall*)())D2CLIENT_PTR(0x6FAC7C90) )();

	int s1 = (*(unsigned int*)D2CLIENT_PTR(0x6FBB5490));

	return (s0 || s1);
}

int D2Client::GetBeltSize() {
	unsigned long buf[0x42];

	d2common.GetBeltData( *(int*)D2CLIENT_PTR(0x6FB8FD94), *(int*)D2CLIENT_PTR(0x6FBCB980), buf );

	return buf[0x01];
}

void* D2Client::GetDataTables() {
	DWORD dt = (*(DWORD*)D2CLIENT_PTR(0x6FB7E454));
	if(dt) return (void*)(*(DWORD*)dt);

	return 0;
}

void* D2Win::GetWndProcVar() {
	return (void*)D2WIN_PTR(0x6F93C734);
}

int D2Win::IsWndProcDisable() {
	return *(int*)D2WIN_PTR(0x6F93C738);
}

void* D2Win::GetWndStatePtr() {
	return (void*)D2WIN_PTR(0x6F8FFAEC);
}

void D2Win::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	*(FARPROC*)&GetStringLength = GetProcAddress(base, (LPCSTR)10132);
	*(FARPROC*)&D2DrawText = GetProcAddress(base, (LPCSTR)10001);
	*(FARPROC*)&SelectFont = GetProcAddress(base, (LPCSTR)10010);
}

void D2Common::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	//++AutoMap
	*(FARPROC*)&GetUnit1Data = GetProcAddress(base, (LPCSTR)10350);
	//--

	*(FARPROC*)&GetSkillStatus = GetProcAddress(base, (LPCSTR)10715);
	*(FARPROC*)&GetClassData4 = GetProcAddress(base, (LPCSTR)10887);
	*(FARPROC*)&GetRoomEx = GetProcAddress(base, (LPCSTR)10366);
	*(FARPROC*)&GetUnit4Price = GetProcAddress(base, (LPCSTR)10122);
	*(FARPROC*)&GetGoldMax = GetProcAddress(base, (LPCSTR)10941);
	*(FARPROC*)&IsTown = GetProcAddress(base, (LPCSTR)10438);
	*(FARPROC*)&IsDemon = GetProcAddress(base, (LPCSTR)10790);
	*(FARPROC*)&IsUndead = GetProcAddress(base, (LPCSTR)10455);
	*(FARPROC*)&GetBeltData = GetProcAddress(base, (LPCSTR)10225);
	*(FARPROC*)&GetNormalSkillLevel = GetProcAddress(base, (LPCSTR)10904);
	*(FARPROC*)&IsSpecialSkill = GetProcAddress(base, (LPCSTR)10146);
	*(FARPROC*)&GetMapLevelData = GetProcAddress(base, (LPCSTR)10301);

	*(FARPROC*)&AllocStatTable = GetProcAddress(base, (LPCSTR)10235);
	*(FARPROC*)&FreeStatTable = GetProcAddress(base, (LPCSTR)10049);
	*(FARPROC*)&FindStatTable = GetProcAddress(base, (LPCSTR)10611);
	*(FARPROC*)&FillStatTable = GetProcAddress(base, (LPCSTR)10911);


	_FindFreeGrid = D2COMMON_PTR(0x6FDB9280);
	FindFreeGrid = &D2Common_FindFreeGrid;

	_GetNextPosDelta = D2COMMON_PTR(0x6FD9A780);
	GetNextPosDelta = &D2Common_GetNextPosDelta;
}

D2StatTable* D2Common::CreateStatTable(D2Unit *target) {
	D2StatTable *src;
	D2Unit *child;
	D2StatTable *stb = AllocStatTable(target->alloc_flag, 0, 0, target->type, target->id);

	if(stb) {
		src = FindStatTable(target, 0x00, 0x40);
		if(src) FillStatTable(stb, src);

		src = FindStatTable(target, 0xAB, 0x40);
		if(src) FillStatTable(stb, src);

		if(target->inv_table && target->inv_table->magic == 0x01020304) {
			child = target->inv_table->head;
			while(child && child->type == 0x04) {
				src = FindStatTable(child, 0x00, 0x40);
				if(src) FillStatTable(stb, src);

				if(child->unit_ex4)
					child = child->unit_ex4->next_a;
				else
					child = 0;

			}
			
		}

	}

	return stb;
}

unsigned int D2Common::GetSkillLevel(D2Unit *me, D2Skill *skill) {
	unsigned int level = 0;
	if( !IsSpecialSkill(skill, 0, 0, &level, 0) ) {
		level = GetNormalSkillLevel(me, skill, 1);
	}

	return level;
}

unsigned int D2Common::GetSkillMana(D2Unit *me, D2Skill *skill) {
	unsigned int esi = 0;
	unsigned char *pd = (unsigned char*)skill->data;
	if( *(unsigned short*)(pd + 0x18A) || *(unsigned short*)(pd + 0x18C) ) {
		unsigned int level = GetSkillLevel(me, skill);
		if(level) {
			unsigned long eax = *(short*)(pd + 0x18C);
			unsigned long ecx = *(short*)(pd + 0x188);
			unsigned long edx = eax << (ecx & 0xFF);

			esi = ( (level - 1) * eax  + (unsigned long)(*(short*)(pd + 0x18A)) ) << (ecx & 0xFF);
			eax = (unsigned long)(*(short*)(pd + 0x186)) << 8;
			if(eax > esi) esi = eax;

		}

	}

	return esi;
}

int D2Common::GetPosition(D2Unit* unit, unsigned int *x, unsigned int *y) {
	if( !unit || !unit->path ) return 0;
	
	switch(unit->type) {
		case 2:
		case 4:
		case 5:
			{
				*x = unit->path2->x;
				*y = unit->path2->y;
			}
			break;

		default:
			{
				*x = unit->path0->x;
				*y = unit->path0->y;
			}
			break;
	}

	return 1;
}

unsigned int D2Common::CodeToClsId(unsigned int code) {
	DWORD data = *(DWORD*)D2COMMON_PTR(0x6FDF07EC);

	if(code == 0x2063656E) code = 0x2067656E;
	
	return d2fog.BinarySearch( (void *)data, code, 0);
}

void D2Fog::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	*(FARPROC*)&BinarySearch = GetProcAddress(base, (LPCSTR)10213);
	*(FARPROC*)&RecvPacket_Realm = GetProcAddress(base, (LPCSTR)10072);
}

void D2Lang::InitBase(HMODULE base) {
	D2ModuleBase::InitBase(base);

	*(FARPROC*)&GetUnicodeString = GetProcAddress(base, (LPCSTR)10005);

}

int D2Bnclient::GetPacketState(int packet_id) {
	return *(((int*)D2BNCLIENT_PTR(0x6FF3EBA0)) + packet_id);
}

unsigned int D2MCPClient::GetRealmLoginResCode() {
	return *(unsigned int*)D2MCPCLIENT_PTR(0x6FA2B1C8);
}

