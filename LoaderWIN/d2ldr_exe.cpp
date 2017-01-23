#include <stdio.h>
#include <windows.h>

#define D2_BASE 0x00400000
#define D2_ECALL (D2_BASE + 0x1238)

#define D2SECTIONNAME ".d2code"

typedef struct _d2data_t {
	char d2gfx[8];
	char d2win[8];

	char bncache[16];

	char cls0[16];
	char cls1[16];
	char key0[16];
	char key1[16];

	char ecall[8];

	unsigned long _d2launch_;
	unsigned long _bnclient_;

	unsigned long _LoadLibrary0_;
	unsigned long _LoadLibrary1_;

	unsigned long _LoadLibrary0_L_;
	unsigned long _LoadLibrary1_L_;

	unsigned long _cdkw_;

	char dllnz[256];

} d2data_t;

d2data_t*  __stdcall get_data();
void __stdcall d2_ecall_hook() ;

//d2data_t d2dat = {"D2gfx", "D2win"};
//char d2exe[1024] = "Game.exe";

//void parse_cmdline();

/*
int main() {
	parse_cmdline();

	//Create Process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO) );
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION) );

	if( !CreateProcess(0, d2exe, 0, 0, 0, CREATE_SUSPENDED, 0, 0, &si, &pi) ) {
		MessageBox(0, "Can't Create Process", 0, 0);
		return 0;
	}
	
	sprintf(d2dat.cls0, "D%08X", pi.dwProcessId);
	sprintf(d2dat.cls1, "D%08X", pi.dwProcessId);
	//sprintf(d2dat.bncache, "%08X", pi.dwProcessId);
	sprintf(d2dat.bncache, "bnc%04hX.dat", pi.dwProcessId);

	//
	unsigned long d2_ecall_offset = (unsigned long)(&d2_ecall_hook) & 0x00000FFF;
	unsigned long d2_sect_base = (unsigned long)(&d2_ecall_hook) & 0xFFFFF000;

	if( !ReadProcessMemory(pi.hProcess, (LPCVOID)D2_ECALL, (LPVOID)&d2dat.ecall, 5, 0) ) {
		MessageBox(0, "Can't Read From Process", 0, 0);
		TerminateProcess(pi.hProcess, 0);
		return 0;
	}
	
	LPVOID d2sb_remote = VirtualAllocEx(pi.hProcess, 0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	unsigned char new_call[8];
	new_call[0] = 0xe9;
	*(unsigned long*)(new_call + 1) = (unsigned long)d2sb_remote + d2_ecall_offset - (D2_ECALL + 5);

	WriteProcessMemory(pi.hProcess, d2sb_remote, (LPCVOID)d2_sect_base, 0x1000, 0);
	WriteProcessMemory(pi.hProcess, (LPVOID)( (unsigned long)d2sb_remote + 0x1000 - sizeof(d2data_t) ), (LPCVOID)&d2dat, sizeof(d2data_t), 0);

	WriteProcessMemory(pi.hProcess, (LPVOID)D2_ECALL, (LPCVOID)&new_call, 5, 0);

	ResumeThread(pi.hThread);

	//getchar();
	//TerminateProcess(pi.hProcess, 0);

	return (int)pi.dwProcessId;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return main();
}
*/

void init_d2_data(d2data_t &d2dat, const char *key0, const char *key1, const char *dllnz) {
	memset(&d2dat, 0x00, sizeof(d2data_t) );
	memcpy(d2dat.d2gfx, "D2gfx", 6);
	memcpy(d2dat.d2win, "D2win", 6);
	if(key0) memcpy(d2dat.key0, key0, sizeof(d2dat.key0) );
	if(key1) memcpy(d2dat.key1, key1, sizeof(d2dat.key1) );
	if(dllnz) strncpy(d2dat.dllnz, dllnz, sizeof(d2dat.dllnz) );

}

unsigned int create_d2_process(PROCESS_INFORMATION &pi, char *d2exe, const char *key0, const char *key1, const char *dllnz) {
	d2data_t d2dat;
	init_d2_data(d2dat, key0, key1, dllnz);

	//Create Process
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO) );
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION) );

	char d2_exe_cmd[MAX_PATH], d2_exe_dir[MAX_PATH];
	strcpy(d2_exe_cmd, d2exe);
	strcpy(d2_exe_dir, d2exe);

	char *p_d2_exe_dir = 0;
	char *schr = strrchr(d2_exe_dir, '\\');
	if(schr) {
		*schr = 0x00;
		p_d2_exe_dir = (char*)d2_exe_dir;
	}

	if( !CreateProcess(0, d2_exe_cmd, 0, 0, 0, CREATE_SUSPENDED, 0, p_d2_exe_dir, &si, &pi) ) {
		//MessageBox(0, "Can't Create Process", 0, 0);
		return 0;
	}
	
	sprintf(d2dat.cls0, "D%08X", pi.dwProcessId);
	sprintf(d2dat.cls1, "D%08X", pi.dwProcessId);
	sprintf(d2dat.bncache, "bnc%04hX.dat", pi.dwProcessId);

	//
	unsigned long d2_ecall_offset = (unsigned long)(&d2_ecall_hook) & 0x00000FFF;
	unsigned long d2_sect_base = (unsigned long)(&d2_ecall_hook) & 0xFFFFF000;

	if( !ReadProcessMemory(pi.hProcess, (LPCVOID)D2_ECALL, (LPVOID)&d2dat.ecall, 5, 0) ) {
		//MessageBox(0, "Can't Read From Process", 0, 0);
		TerminateProcess(pi.hProcess, 0);
		return 0;
	}
	
	LPVOID d2sb_remote = VirtualAllocEx(pi.hProcess, 0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	unsigned char new_call[8];
	new_call[0] = 0xe9;
	*(unsigned long*)(new_call + 1) = (unsigned long)d2sb_remote + d2_ecall_offset - (D2_ECALL + 5);

	WriteProcessMemory(pi.hProcess, d2sb_remote, (LPCVOID)d2_sect_base, 0x1000, 0);
	WriteProcessMemory(pi.hProcess, (LPVOID)( (unsigned long)d2sb_remote + 0x1000 - sizeof(d2data_t) ), (LPCVOID)&d2dat, sizeof(d2data_t), 0);

	WriteProcessMemory(pi.hProcess, (LPVOID)D2_ECALL, (LPCVOID)&new_call, 5, 0);

	ResumeThread(pi.hThread);

	return pi.dwProcessId;

}
/*
int get_args(char *cmdline, char **argv, int size) {
	if(!size) return 0;
	
	int argc = 0;
	int f_dq = 0;
	int f_dr = 1;
	char *cur_argv = 0;
	char chr;

	do {
		chr = *cmdline;

		if( chr == '"' || (!f_dq && (chr == 0x00 || chr ==' ' || chr =='\t')) ) {
			if(!f_dr) {
				*cmdline = 0x00;
				argv[argc] = cur_argv;
				argc++;
				f_dr = 1;

				if(argc >= size) break;

			}

			if(chr == '"') f_dq ^= 1;

		} else if(f_dr) {
			cur_argv = cmdline;
			f_dr = 0;

		}

		cmdline++;
	} while(chr != 0x00);

	return argc;
}

//no buffer chk
void parse_cmdline() {

	char path[256];
	GetModuleFileName(0, path, sizeof(path));
	char *sp = strrchr(path, '\\');

	if(sp) {
		sp[0] = 0x00;
		SetCurrentDirectory(path);
	}

	char *wexe = d2exe + strlen(d2exe);
	char cmdline[1024];
	char *argv[32];

	strcpy(cmdline, GetCommandLine() );
	int argc = get_args(cmdline, argv, 32);

	for(int i = 1; i < argc; i++) {

		if( !strcmp(argv[i], "-key") && (i + 1< argc) ) {
			FILE *fp;
			fp = fopen(argv[++i], "r");
			if( fp ) {
				char buf[32];

				//key0
				fgets(buf, sizeof(buf), fp);
				memcpy(d2dat.key0, buf, 16);
				//printf("key0:%s:\r\n", buf);

				//key1
				fgets(buf, sizeof(buf), fp);
				memcpy(d2dat.key1, buf, 16);
				//printf("key1:%s:\r\n", buf);

				fclose(fp);

			} else
				MessageBox(0, "Can't Open KeyFile", 0, 0);

		} else if( !strcmp(argv[i], "-dll") && (i + 1< argc) ) {
			strcpy(d2dat.dllnz, argv[++i]);

		} else {
			*wexe = ' ';
			strcpy(wexe + 1, argv[i]);
			wexe += strlen(argv[i]) + 1;

		}

	}

}

*/

#pragma code_seg(D2SECTIONNAME)

#define DeRefLongPtr(P0) ( *(unsigned long*)(P0) )

typedef HMODULE (__stdcall *MessageBox_FUNC_T)(HWND, LPCSTR, LPCSTR, UINT);
#define D2_MessageBox ( (MessageBox_FUNC_T)DeRefLongPtr(D2_BASE + 0x91FC) )

typedef HMODULE (__stdcall *GetModuleHandle_FUNC_T)(LPCSTR);
#define D2_GetModuleHandle ( (GetModuleHandle_FUNC_T)DeRefLongPtr(D2_BASE + 0x916C) )

typedef BOOL (__stdcall *VirtualProtect_FUNC_T)(LPVOID, DWORD, DWORD, PDWORD);
#define D2_VirtualProtect ( (VirtualProtect_FUNC_T)DeRefLongPtr(D2_BASE + 0x9108) )

typedef LONG (__stdcall *InterlockedExchange_FUNC_T)(LPLONG, LONG);
#define D2_InterlockedExchange ( (InterlockedExchange_FUNC_T)DeRefLongPtr(D2_BASE + 0x9100) )

typedef HMODULE (__stdcall *LoadLibrary_FUNC_T)(LPCSTR);
#define LoadLibrary_0_Addr (D2_BASE + 0x91B4)
#define D2_LoadLibrary ( (LoadLibrary_FUNC_T)DeRefLongPtr(LoadLibrary_0_Addr) )

typedef BOOL (__stdcall *CloseHandle_FUNC_T)(HANDLE hObject);
typedef BOOL (__stdcall *DeleteFileA_FUNC_T)(LPCSTR lpFileName);

typedef int (__cdecl *atexit_func_t)(unsigned long);


void __stdcall memcpy_b(void *dst, void *src, int size);
void __stdcall patch();
HMODULE __stdcall LoadLibrary_Hook0(LPCSTR nz);
HMODULE __stdcall LoadLibrary_Hook1(LPCSTR nz);
int __stdcall cdkey_hook();
void __cdecl del_bncache() ;


__declspec(naked) void __stdcall d2_ecall_hook() {
	__asm {
		pushad;
		pushfd;

		cld;
		call patch;

		popfd;
		popad;

		mov eax, D2_ECALL;
		jmp eax;
	}

}

__declspec(naked) d2data_t* __stdcall get_data() {
	__asm {
		mov eax, [esp];
		and eax, 0xFFFFF000;
		add eax, 0x1000 - SIZE d2data_t;
		retn;
	}

}

__declspec(naked) void __stdcall memcpy_b(void *dst, void *src, int sz) {
	__asm {
		push ebp;
		mov ebp, esp;
		push ecx;
		push esi;
		push edi;

		mov ecx, sz;
		mov esi, src;
		mov edi, dst;
		rep movsb;

		pop edi;
		pop esi;
		pop ecx;
		pop ebp;
		retn 0x0c;
	}

}


void __stdcall patch() {
	d2data_t *dat = get_data();

	//restore the call
	DWORD old_pt = 0;
	D2_VirtualProtect((LPVOID)(D2_BASE + 0x1000), 0x8000, PAGE_EXECUTE_READWRITE, &old_pt);
	memcpy_b( (void*)D2_ECALL, (void*)dat->ecall, 5);
	D2_VirtualProtect((LPVOID)(D2_BASE + 0x1000), 0x8000, old_pt, &old_pt);

	HMODULE d2gfx_base = D2_GetModuleHandle(dat->d2gfx);

	memcpy_b( (void*)( (unsigned long)d2gfx_base + 0x10CE0 ), (void*)dat->cls0, 9 );
	memcpy_b( (void*)( (unsigned long)d2gfx_base + 0x10CF0 ), (void*)dat->cls1, 9 );

	old_pt = 0;
	D2_VirtualProtect((LPVOID)(D2_BASE + 0x9000), 0x2000, PAGE_READWRITE, &old_pt);
	dat->_LoadLibrary0_ = *(unsigned long*)(LoadLibrary_0_Addr);
	*(unsigned long*)(LoadLibrary_0_Addr) = ((unsigned long)dat & 0xFFFFF000) + ((unsigned long)&LoadLibrary_Hook0 & 0x00000FFF);
	D2_VirtualProtect((LPVOID)(D2_BASE + 0x9000), 0x2000, old_pt, &old_pt);

}

HMODULE __stdcall LoadLibrary_Hook0(LPCSTR nz) {
	d2data_t *dat = get_data();

	if( (long)nz == (long)(D2_BASE + 0xA574) ) {

		if( D2_InterlockedExchange((LPLONG)&dat->_LoadLibrary0_L_, dat->_LoadLibrary0_) != (long)dat->_LoadLibrary0_ ) { //lock
			DWORD old_pt = 0;
			D2_VirtualProtect((LPVOID)(D2_BASE + 0x9000), 0x2000, PAGE_READWRITE, &old_pt);
			D2_InterlockedExchange( (long*)LoadLibrary_0_Addr, dat->_LoadLibrary0_); //exchange only
			D2_VirtualProtect((LPVOID)(D2_BASE + 0x9000), 0x2000, old_pt, &old_pt);

			HMODULE d2launch = ((LoadLibrary_FUNC_T)dat->_LoadLibrary0_)(nz);
			if( d2launch ) {
				dat->_d2launch_ = (unsigned long)d2launch;

				old_pt = 0;
				D2_VirtualProtect((LPVOID)((unsigned long)d2launch + 0x1C000), 0x4000, PAGE_READWRITE, &old_pt);
				dat->_LoadLibrary1_ = D2_InterlockedExchange( (long*)((unsigned long)d2launch + 0x1C35C),
					((unsigned long)dat & 0xFFFFF000) + ((unsigned long)&LoadLibrary_Hook1 & 0x00000FFF) ); //exchange only
				D2_VirtualProtect((LPVOID)((unsigned long)d2launch + 0x1C000), 0x4000, old_pt, &old_pt);
			}

			return d2launch;
		}
		
	}

	return ((LoadLibrary_FUNC_T)dat->_LoadLibrary0_)(nz);
}

HMODULE __stdcall LoadLibrary_Hook1(LPCSTR nz) {
	d2data_t *dat = get_data();

	if( (long)nz == (long)(dat->_d2launch_ + 0x1D6E4) ) {

		if( D2_InterlockedExchange((LPLONG)&dat->_LoadLibrary1_L_, dat->_LoadLibrary1_) != (long)dat->_LoadLibrary1_ ) { //lock
			DWORD old_pt = 0;
			D2_VirtualProtect((LPVOID)(dat->_d2launch_ + 0x1C000), 0x4000, PAGE_READWRITE, &old_pt);
			D2_InterlockedExchange( (long*)(dat->_d2launch_ + 0x1C35C), dat->_LoadLibrary1_); //exchange only
			D2_VirtualProtect((LPVOID)(dat->_d2launch_ + 0x1C000), 0x4000, old_pt, &old_pt);

			HMODULE bnclient = ((LoadLibrary_FUNC_T)dat->_LoadLibrary1_)(nz);
			if( bnclient ) {
				dat->_bnclient_ = (unsigned long)bnclient;


				( (atexit_func_t) (dat->_bnclient_ + 0x1534) )(
						((unsigned long)dat & 0xFFFFF000) + ((unsigned long)&del_bncache & 0x00000FFF) );


				old_pt = 0;
				D2_VirtualProtect((LPVOID)((unsigned long)bnclient + 0x19000), 0x4000, PAGE_READWRITE, &old_pt);

				memcpy_b( (void*)((unsigned long)bnclient + 0x1AB0D), (void*)(dat->bncache), 11);

				if( dat->key0[0] && dat->key1[1] ) {
					dat->_cdkw_ = D2_InterlockedExchange( (long*)((unsigned long)bnclient + 0x19030),
						((unsigned long)dat & 0xFFFFF000) + ((unsigned long)&cdkey_hook & 0x00000FFF) ); //exchange only
				}

				D2_VirtualProtect((LPVOID)((unsigned long)bnclient + 0x19000), 0x4000, old_pt, &old_pt);

				//d2 starts normally, load dlls
				if( dat->dllnz[0] )
					((LoadLibrary_FUNC_T)dat->_LoadLibrary1_)(dat->dllnz);

			}

			return bnclient;
		}
		
	}

	return ((LoadLibrary_FUNC_T)dat->_LoadLibrary1_)(nz);
}

int __stdcall patch_cd_keys(unsigned long ret_ip) {
	d2data_t *dat = get_data();

	if(dat->_bnclient_ + 0x165D4 == ret_ip) {
		//D2_MessageBox(0, dat->d2win, 0, 0);
		unsigned long key0 = *(unsigned long*)(dat->_bnclient_ + 0x1F0DC);
		unsigned long key1 = *(unsigned long*)(dat->_bnclient_ + 0x1F0E4);

		if(key0 && key1) {
			memcpy_b( (void*)key0, dat->key0, 16);
			memcpy_b( (void*)key1, dat->key1, 16);
		}

	}


	return ( (int(__stdcall *)()) dat->_cdkw_ )();
}

__declspec(naked) int __stdcall cdkey_hook() {
	__asm {
		push [esp];
		call patch_cd_keys;
		retn;
	}

}

void __cdecl del_bncache() {
	d2data_t *dat = get_data();

	HANDLE fh = (HANDLE)DeRefLongPtr(dat->_bnclient_ + 0x1DDC4);
	if( (unsigned long)fh != (unsigned long)-1) {
		( (CloseHandle_FUNC_T) DeRefLongPtr(dat->_bnclient_ + 0x19068) )(fh);
		
	}

	( (DeleteFileA_FUNC_T) DeRefLongPtr(dat->_bnclient_ + 0x1906C) )( dat->bncache );

}

#pragma code_seg()
