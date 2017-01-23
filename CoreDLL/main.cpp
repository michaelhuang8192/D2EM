#include "D2EM.h"
#include <stdio.h>

#define DMS_INTVAL 30

HINSTANCE current_dll_base;

void Initialize();
void Finalize();
int swap_imp_fptr(DWORD imp_addr, DWORD imp_size, DWORD offset, DWORD* oldval, DWORD val);

HMODULE __stdcall LoadLibraryA_Hook(LPCSTR lpLibFileName);
BOOL __stdcall BitBlt_Hook(HDC, int, int, int, int, HDC, int, int, DWORD);
int __stdcall SendPacket(int size, int type, const char *buf);
int __stdcall RecvPacket0(const char *buf, int size);
int __stdcall RecvPacket1(const char *buf, int size);
int __stdcall GetStatus();
void __stdcall UpdateThread(void *arg0);
int __stdcall HideWindow_Hook(int hidden);
int __stdcall SetCursorPos_Hook(int x, int y);
int __stdcall RecvPacket_Realm(void *ptr, char *buf, int size);
void __stdcall Sleep_Hook(DWORD ms);

static void (__stdcall *_update_thread)(void *arg0);

//++AutoMap
void __stdcall GetUnit1Data_Hook(D2Unit *unit, void *p1, void *p2, void *p3, void *p4, void *p5);
//--

extern unsigned int d2em_le_mode;
extern unsigned int d2em_automap;


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	switch(fdwReason) {
		case DLL_PROCESS_DETACH:
			{
				Finalize();
			}
			break;

		case DLL_PROCESS_ATTACH:
			{
				current_dll_base = hinstDLL;
				Initialize();
			}
			break;

	}

	return 1;
}

static void Initialize() {
	//Bnclient Is Loaded
	d2game.InitBase( GetModuleHandle(0) );
	d2net.InitBase( GetModuleHandle("D2Net") );
	d2gdi.InitBase( GetModuleHandle("D2Gdi") );
	d2gfx.InitBase( GetModuleHandle("D2gfx") );
	d2win.InitBase( GetModuleHandle("D2Win") );
	d2fog.InitBase( GetModuleHandle("Fog") );
	d2lang.InitBase( GetModuleHandle("D2Lang") );
	d2bnclient.InitBase( GetModuleHandle("Bnclient") );
	d2mcpclient.InitBase( GetModuleHandle("D2MCPClient") );
	
	Init_D2EM();

	DWORD tmp;
	swap_imp_fptr( (DWORD)d2game._base + 0x9000, 0x2000, 0x01B4, &tmp, (DWORD)&LoadLibraryA_Hook);
	swap_imp_fptr( (DWORD)d2gdi._base + 0x9000, 0x2000, 0x0080, &tmp, (DWORD)&BitBlt_Hook);
	swap_imp_fptr( (DWORD)d2win._base + 0x1A000, 0x4000, 0x00B4, &tmp, (DWORD)&HideWindow_Hook);
	swap_imp_fptr( (DWORD)d2mcpclient._base + 0x8000, 0x2000, 0x0010, &tmp, (DWORD)&RecvPacket_Realm);

}


static void Finalize() {
	Del_D2EM();

}

static int swap_imp_fptr(DWORD imp_addr, DWORD imp_size, DWORD offset, DWORD* oldval, DWORD val) {
	int ret = 0;

	DWORD old_protect;
	if( VirtualProtect( (LPVOID)imp_addr, imp_size, PAGE_READWRITE, &old_protect) ) {
		*oldval = *(DWORD*)(imp_addr + offset);
		*(DWORD*)(imp_addr + offset) = val;
		VirtualProtect( (LPVOID)imp_addr, imp_size, old_protect, &old_protect);
		ret = 1;
	}
	
	return ret;
}

static HMODULE __stdcall LoadLibraryA_Hook(LPCSTR lpLibFileName) {
	if( (DWORD)lpLibFileName == ( (DWORD)d2game._base + 0xA5A0 ) ) {

		HMODULE lib0 = GetModuleHandle("D2Client");
		HMODULE lib1 = LoadLibraryA(lpLibFileName);

		if( !lib0 && lib1 ) {
			d2client.InitBase( lib1 );
			d2common.InitBase( GetModuleHandle("D2Common") );
			
			DWORD tmp;
			swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x0B14, &tmp, (DWORD)&GetStatus);
			swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x0B18, &tmp, (DWORD)&RecvPacket1);
			swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x0B1C, &tmp, (DWORD)&SendPacket);
			swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x0B40, &tmp, (DWORD)&RecvPacket0);
			swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x0A14, (DWORD*)&_update_thread, (DWORD)&UpdateThread);

			swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x10FC, &tmp, (DWORD)&SetCursorPos_Hook);
			if(d2em_le_mode) swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x0F7C, &tmp, (DWORD)&Sleep_Hook);

			//++AutoMap
			if(d2em_automap) swap_imp_fptr( (DWORD)lib1 + 0xCE000, 0xD000, 0x05D8, &tmp, (DWORD)&GetUnit1Data_Hook);
			//--
		}

		return lib1;
	}

	return LoadLibraryA(lpLibFileName);
}

static int __stdcall SetCursorPos_Hook(int x, int y) {
	__dprintf("SetCursorPos(%x, %x)\r\n", x, y);
	return 1;
}

static int __stdcall HideWindow_Hook(int hidden) {
	__dprintf("HideWindow(%d), CurState:(%d), ", hidden, d2gfx.IsHidden() );

	HWND hwnd = d2gfx.GetHWND();
	if( hidden == 1 && !IsIconic(hwnd) ) {
		__dprintf("ShowWindow(%p, SW_MINIMIZE), ", hwnd);
		ShowWindow(hwnd, SW_MINIMIZE);//SW_SHOWMINIMIZED);
	}

	int ret = d2gfx.HideWindow(0);
	__dprintf("Done.\r\n");

	return ret;
}

//++AutoMap

static int __stdcall __am_get_monster_ps_color(D2Unit *unit, int lret, unsigned char *ps_color) {
	if( lret ) return 1;

	int ret = 0;
	if( unit && unit->unit_ex1 && (unit->flag & 4) ) {
		ret = 1;

		if( unit->unit_ex1->flag & 0x08 ) {
			*ps_color = 0x70;

		} else {
			*ps_color = 0x50;

		}

	}
	
	return ret;
}

static DWORD __ret_ip;

__declspec(naked) static void __stdcall __am_call_func_on_ret() {
	__asm {
		push ebx;
		push eax;
		push esi;
		call __am_get_monster_ps_color;
		mov ecx, __ret_ip;
		jmp ecx;
	}
}

static void __stdcall GetUnit1Data_Hook(D2Unit *unit, void *p1, void *p2, void *p3, void *p4, void *p5) {
	d2common.GetUnit1Data(unit, p1, p2, p3, p4, p5);

	if( *((DWORD*)&unit - 1) == (DWORD)d2client._base + 0x3F555 ) {
		__ret_ip = *( (DWORD*)&unit + 11 );
		*(volatile DWORD*)( (DWORD*)&unit + 11 ) = (DWORD)&__am_call_func_on_ret;
	}
}
//--

//static DWORD __tick = 0;
static void __stdcall UpdateThread(void *arg0) {
	_update_thread(arg0);
	YieldControl();
	//__dprintf("%d\r\n", GetTickCount() - __tick);
	//__tick = GetTickCount();
}

static BOOL __stdcall BitBlt_Hook(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop) {
	if(d2em_le_mode) *(unsigned int*)(d2win.GetWndStatePtr()) = 0;
	OnDraw(hdcSrc);

	return BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
}

static DWORD __last_sleep_hook = 0;
static void __stdcall Sleep_Hook(DWORD ms) {
	if( *(&ms - 1) != (DWORD)d2client._base + 0x6CFDC ) {
		Sleep(ms);
	} else {
		DWORD ums = GetTickCount() - __last_sleep_hook;
		if(ums < DMS_INTVAL)
			Sleep(DMS_INTVAL - ums);
		else
			Sleep(1);

		__last_sleep_hook = GetTickCount();
	}
}

static int __stdcall SendPacket(int size, int type, const char *buf) {
	int retsz = size;
	
	if( size <= 0 || OnSendPacket(size, type, buf) || type == 0 ) {
		retsz = (*d2net.SendPacket)(size, type, buf);

	}

	return retsz;
}

static int __stdcall RecvPacket0(const char *buf, int size) {
	int retsz = (*d2net.RecvPacket0)(buf, size);

	if(retsz <= 0) {
		
	} else {
		OnRecvPacket(retsz, 0, buf);

	}

	return retsz;
}

static int __stdcall RecvPacket1(const char *buf, int size) {
	//static DWORD __tick = GetTickCount();

	int retsz;

	do {
		retsz = (*d2net.RecvPacket1)(buf, size);

		if( retsz != -1 ){
			if( retsz <= 0 || OnRecvPacket(retsz, 1, buf) ) { (*d2client.ProcessPacket)(buf, retsz); }

			Sleep(0);
		}		

	} while(retsz != -1);

	if(d2em_le_mode) *(unsigned int*)(d2client.GetWndStatePtr()) = 0;
	//Sleep(16);
	//__dprintf("%d\r\n", GetTickCount() - __tick);
	//__tick = GetTickCount();
	//YieldControl();

	return -1;
}

static int __stdcall GetStatus() {
	int ret = (*d2net.GetStatus)();

	if(ret) {
		
	} else {
		OnExit();

	}

	return ret;
}

/*
static int __stdcall RecvPacket1(char *buf, int size) {
	int retsz = (*d2_hfuncs.RecvPacket1)(buf, size);

	if(retsz == -1) {
		OnYield();

	} else if(retsz > 0 && !OnRecvPacket(retsz, 1, buf) ) {
		retsz = 0;

	}

	return retsz;
}
*/

static int __stdcall RecvPacket_Realm(void *ptr, char *buf, int size) {
	int ret = d2fog.RecvPacket_Realm(ptr, buf, size);

	if(ret > 0) OnRecvPacket_Realm(buf, ret);

	return ret;
}
