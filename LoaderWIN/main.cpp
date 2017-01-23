#include <windows.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include <stdio.h>
#include <time.h>
#include "D2EM.h"

static const char g_szClassName[] = "D2EM_WIN_CLASS";

struct HLV_CSD {
	const char* name;
	int width;
};

static const HLV_CSD HLV_Columns[] =
{
	{"#", 25}, 
	{"PID", 50},
	{"Account", 130},
	{"Status", 185},
	{"Stat", 100},
};

#define HLV_COLUMNS_SIZE (sizeof(HLV_Columns) / sizeof(HLV_Columns[0]))

EMPluginParam em_pparm;
WebUIConfig wconfig;
AccountConfig aconfig[ACONFIG_SIZE];
IFCDKeyEntry ifcdkey[IFCDKEY_SIZE];
int ifcdkey_count;

CDKeyManager cdkeymgr;

char current_app_dir[256];
char core_dll_fpath[256];
HWND _hlv, _hsb;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND CreateListView(HINSTANCE inst, HWND hwnd);
void parse_config_file(const char *filename);
void CALLBACK OnTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);
void load_cdkey(const char *filename);
void save_cdkey(const char *filename);
void release_cdkey();
void create_shm();
void free_shm();
void set_dlv_text(int num, int column, char *text);

extern void show_d2_window(int idx);
extern void open_d2_process(int idx);
extern void update_d2_process();

FILE *cfp;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//AllocConsole();
	//cfp = fopen("CONOUT$", "a");
	//setbuf(cfp, 0);

	//init
	GetModuleFileName(0, current_app_dir, sizeof(current_app_dir));
	*( strrchr(current_app_dir, '\\') ) = 0x00;

	char filename[256];
	strcpy(filename, current_app_dir);
	strcat(filename, "\\log\\clog.txt");
	cfp = fopen(filename, "a");
	fprintf(cfp, "+++Tid[%u]+++\n", GetCurrentThreadId());

	strcpy(filename, current_app_dir);
	strcat(filename, "\\D2EM.ini");
	parse_config_file(filename);

	strcpy(filename, current_app_dir);
	strcat(filename, "\\cdkey.ini");
	load_cdkey(filename);

	strcpy(core_dll_fpath, current_app_dir);
	strcat(core_dll_fpath, "\\D2EM.dll");

	create_shm();

	//win
	WNDCLASSEX wc;
	HWND hwnd;
    MSG msg;

	memset(&wc, 0x00, sizeof(wc) );
	wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_D2EM_ICON) ); //LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszMenuName	= NULL;
    wc.lpszClassName= g_szClassName;
	wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ID_D2EM_ICON) ); //LoadIcon(NULL, IDI_APPLICATION);

	if( !RegisterClassEx(&wc) ) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 1;
	}

	hwnd = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		g_szClassName,
		"D2EM v2.00 - Build On " __DATE__,
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_SIZEBOX ,
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 220,
		NULL, NULL, hInstance, NULL
		);

	_hsb = CreateWindowEx(
		0,
		STATUSCLASSNAME,
		0,
		WS_CHILD|WS_VISIBLE,
		0,0,0,0,
		hwnd,(HMENU)ID_D2EM_STATUSBAR,hInstance,NULL
		);

	_hlv = CreateListView(hInstance, hwnd);
	SetTimer(hwnd, ID_D2EM_TIMER, 1000, (TIMERPROC)(&OnTimerProc) );

	ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

	//load webui
	HANDLE h_webui_thread = 0;
	HMODULE h_webui_dll = 0;
	if(wconfig.enable) {
		char webui_dll_path[256];
		strcpy(webui_dll_path, current_app_dir);
		strcat(webui_dll_path, "\\WebUI.dll");
		h_webui_dll = LoadLibrary(webui_dll_path);
		if(h_webui_dll) {
			em_pparm.wc = &wconfig;
			em_pparm.ac_size = ACONFIG_SIZE;
			em_pparm.ac = aconfig;
			em_pparm.dlog = &__dlog;
			em_pparm.mwin = hwnd;
			em_pparm.lv = _hlv;
			em_pparm.sb = _hsb;
			em_pparm.evt_exit = CreateEvent(0, 1, 0, 0);
			LPTHREAD_START_ROUTINE tsr = (LPTHREAD_START_ROUTINE)GetProcAddress(h_webui_dll, "_webui_thread_proc@4");
			if( !tsr || !(h_webui_thread=CreateThread(0, 0, tsr, &em_pparm, 0, 0)) ) FreeLibrary(h_webui_dll);
		}
	}

	char sb_str[128];
	sprintf(sb_str, "[Total CDKeys: %d]  [WebUI: %d]", ifcdkey_count, h_webui_thread!=0);
	SendMessage(_hsb, SB_SETTEXT, 0, (LPARAM)sb_str);

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	//unload webui
	if(h_webui_thread && wconfig.enable) {
		SetEvent(em_pparm.evt_exit);
		WaitForSingleObject(h_webui_thread, INFINITE);
		CloseHandle(h_webui_thread);
		FreeLibrary(h_webui_dll);
		CloseHandle(em_pparm.evt_exit);
	}

	KillTimer(hwnd, ID_D2EM_TIMER);
	release_cdkey();
	save_cdkey(filename);

	free_shm();
	if(cfp) fclose(cfp);

	return (int)msg.wParam;
}

int get_cdkey_idx(const char *key0) {
	return ((unsigned long)key0 - (unsigned long)ifcdkey) / sizeof(ifcdkey[0]);
}

void __dlog(unsigned int num, const char *fmt, ...) {
	if(cfp) {
		char buf[512];

		va_list args;
		va_start(args, fmt);
		int retsz = _vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);

		if(retsz > 0) {
			buf[ sizeof(buf) - 1 ] = 0x00;
			char tstr[128];
			time_t ts = time(0);
			tm *tm = localtime(&ts);
			strftime(tstr, sizeof(tstr), "%x %X", tm);
			fprintf(cfp, "[%s][s%d]: %s\n", tstr, num, buf);
		}
	}
}

void CALLBACK OnTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime) {
	update_d2_process();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_WEBUI_CMD:
			{
				if(!wParam) open_d2_process((int)lParam);
			}
			break;

		case WM_NOTIFY:
			{
				if( ((LPNMHDR)lParam)->hwndFrom == _hlv ) {
					switch( ((LPNMHDR)lParam)->code ) {
						case NM_DBLCLK:
							{
								open_d2_process( ((LPNMITEMACTIVATE)lParam)->iItem );
							}
							break;

						case NM_RDBLCLK:
							{
								show_d2_window( ((LPNMITEMACTIVATE)lParam)->iItem );
							}
							break;

					}
				}
			}
			break;

		case WM_CLOSE:
            DestroyWindow(hwnd);
			break;

        case WM_DESTROY:
            PostQuitMessage(0);
			break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

HWND CreateListView(HINSTANCE inst, HWND hwnd) {
	INITCOMMONCONTROLSEX icex;
	memset(&icex, 0x00, sizeof(icex));
	icex.dwSize = sizeof(icex);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rcl, rcl2;
	GetClientRect(hwnd, &rcl);
	GetClientRect(_hsb, &rcl2);

	HWND hlv = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, 0, rcl.right - rcl.left, rcl.bottom - rcl.top - (rcl2.bottom - rcl2.top) + 2,
		hwnd, (HMENU)ID_D2EM_LV, inst, NULL);

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	for(int i = 0; i < HLV_COLUMNS_SIZE; i++) {
		lvc.iSubItem = i;

		lvc.cx = HLV_Columns[i].width;
		lvc.pszText = (LPSTR)( HLV_Columns[i].name );

		ListView_InsertColumn(hlv, i, &lvc);

	}

	ListView_SetExtendedListViewStyle(hlv, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	char buf[32];
	LV_ITEM lvi;
	memset(&lvi, 0x00, sizeof(lvi));
	lvi.mask = LVIF_TEXT;
	for(int n = 1; n < ACONFIG_SIZE; n++) {
		lvi.iItem = n - 1;
		lvi.iSubItem = 0;
		_snprintf(buf, sizeof(buf), "%d", n);
		lvi.pszText = buf;
		ListView_InsertItem(hlv, &lvi);

		if(aconfig[n].num) {
			lvi.iSubItem = 2;
			lvi.pszText = (char*)aconfig[n].user;
			int ret = SendMessage(hlv, LVM_SETITEM, 0, (LPARAM)(const LV_ITEM*)&lvi);
		}
	}

	return hlv;
}

void set_dlv_text(int num, int column, char *text) {
	if(num < 1 || num >= ACONFIG_SIZE || !aconfig[num].num) return;

	LV_ITEM lvi;
	memset(&lvi, 0x00, sizeof(lvi));

	lvi.mask = LVIF_TEXT;
	lvi.iItem = num - 1;
	lvi.iSubItem = column;
	lvi.pszText = text;

	SendMessage(_hlv, LVM_SETITEM, 0, (LPARAM)(const LV_ITEM*)&lvi);
}

static void parse_config_file(const char *filename) {
	char app_buf[256], key_buf[256], val_buf[256];
	if( !GetPrivateProfileString(0, 0, 0, app_buf, sizeof(app_buf), filename) ) return;

	PAccountConfig ac;
	char *app = (char*)app_buf;
	for(int app_len = strlen(app); app_len; app += app_len + 1, app_len = strlen(app)) {
		int t = 0;
		if( !strcmp("global", app) ) {
			ac = &aconfig[0];
			t = 1;
		} else if( app_len > 7 && !memcmp("account", app, 7) ) {
			int num = atoi(app + 7);
			if(num >= ACONFIG_SIZE || num < 1) continue;

			ac = &aconfig[num];
			memcpy(ac, &aconfig[0], sizeof(aconfig[0]) );
			ac->num = num;
			t = 1;
		} else if( !strcmp("webui", app) ) {
			t = 2;
		} else {
			continue;
		}
		
		if( !GetPrivateProfileString(app, 0, 0, key_buf, sizeof(key_buf), filename) ) continue;

		char *key = (char*)key_buf;
		for(int key_len = strlen(key); key_len; key += key_len + 1, key_len = strlen(key)) {
			if( !GetPrivateProfileString(app, key, 0, val_buf, sizeof(val_buf), filename) ) continue;

			if(t == 1) {
				if( !strcmp("user", key) ) {
					strncpy(ac->user, val_buf, sizeof(ac->user));
				} else if( !strcmp("pass", key) ) {
					strncpy(ac->pass, val_buf, sizeof(ac->pass));
				} else if( !strcmp("gminsec", key) ) {
					ac->gminsec = atoi(val_buf) * 1000;
				} else if( !strcmp("gmaxsec", key) ) {
					ac->gmaxsec = atoi(val_buf) * 1000;
				} else if( !strcmp("kmaxgame", key) ) {
					ac->kmaxgame = atoi(val_buf);
					if(!ac->kmaxgame || ac->kmaxgame > MAX_GAME_PER_KEY) ac->kmaxgame = MAX_GAME_PER_KEY;
				} else if( !strcmp("kminasec", key) ) {
					ac->kminasec = atoi(val_buf) * 1000;
				} else if( !strcmp("stoponrd", key) ) {
					ac->stoponrd = atoi(val_buf);
				} else if( !strcmp("cpos", key) ) {
					ac->cpos = atoi(val_buf);
					if(ac->cpos >= 8) ac->cpos = 0;
				} else if( !strcmp("gname", key) ) {
					strncpy(ac->gname, val_buf, sizeof(ac->gname));
				} else if( !strcmp("gpass", key) ) {
					strncpy(ac->gpass, val_buf, sizeof(ac->gpass));
				} else if( !strcmp("gdiff", key) ) {
					ac->gdiff = atoi(val_buf);
					if(ac->gdiff >= 3) ac->gdiff = 0;
				} else if( !strcmp("master", key) ) {
					ac->master = atoi(val_buf);
					if(ac->master < 1 || ac->master >= ACONFIG_SIZE) ac->master = ac->num;
				} else if( !strcmp("d2exec", key) ) {
					strncpy(ac->d2exec, val_buf, sizeof(ac->d2exec));
				} else if( !strcmp("script", key) ) {
					strncpy(ac->script, val_buf, sizeof(ac->script));
				} else {
					unsigned int vlen = strlen(val_buf);
					if(	ac->script_config_size + key_len + vlen + 2 <= sizeof(ac->script_config) ) {
						strncpy(&ac->script_config[ac->script_config_size], key, key_len + 1);
						ac->script_config_size += key_len + 1;
						strncpy(&ac->script_config[ac->script_config_size], val_buf, vlen + 1);
						ac->script_config_size += vlen + 1;
					}
				}

			} else if(t == 2) {
				if( !strcmp("enable", key) ) {
					wconfig.enable = atoi(val_buf);
				} else if( !strcmp("port", key) ) {
					wconfig.port = atoi(val_buf);
				} else if( !strcmp("user", key) ) {
					strncpy(wconfig.user, val_buf, sizeof(wconfig.user));
				} else if( !strcmp("pass", key) ) {
					strncpy(wconfig.pass, val_buf, sizeof(wconfig.pass));
				}

			}

		}//end for_key

	}//end for_app

}

static void load_cdkey(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if(!fp) return;
	
	CDKeyEntry cdkey;
	char buf[128];
	while( ifcdkey_count < IFCDKEY_SIZE && fgets(buf, sizeof(buf), fp) ) {
		int ret = sscanf(buf, "%16c,%16c,%lu,",
			ifcdkey[ifcdkey_count].key0,
			ifcdkey[ifcdkey_count].key1,
			&ifcdkey[ifcdkey_count].ts);

		if(ret != 3) continue;
		
		memset( &cdkey, 0x00, sizeof(cdkey) );
		cdkey.key0 = (const char*)ifcdkey[ifcdkey_count].key0;
		cdkey.key1 = (const char*)ifcdkey[ifcdkey_count].key1;
		cdkey.ts = &ifcdkey[ifcdkey_count].ts;
		cdkeymgr.add_key(cdkey);

		ifcdkey_count++;
	}

	fclose(fp);
}

static void save_cdkey(const char *filename) {
	if(!ifcdkey_count) return;

	FILE *fp = fopen(filename, "w");
	if(!fp) return;

	fputs("*******NewKey Format:key0,key1,0,*******\n\n", fp);

	time_t ts;
	char buf[256];
	int k = 0;
	while(k < ifcdkey_count) {
		ts = ifcdkey[k].ts;
		sprintf(buf, "%.16s,%.16s,%lu,#%d,%s",
			ifcdkey[k].key0,
			ifcdkey[k].key1,
			ifcdkey[k].ts,
			k,
			ctime( &ts ));

		fputs(buf, fp);
		k++;
	}

	fclose(fp);

}

static void create_shm() {
	char buf[64], d2exec[256];
	HANDLE fmh;
	PD2EM_SHM shm;
	PAccountConfig ac = &aconfig[1];
	for(int i = 1; i < ACONFIG_SIZE; i++, ac++) {
		if( !ac->num ) continue;

		sprintf(buf, "D2EM_%X_%02d", GetCurrentProcessId(), ac->num );
		fmh = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, 4096, buf);
		if(!fmh) {
			ac->num = 0;
			continue;
		}

		shm = (PD2EM_SHM)MapViewOfFile(fmh, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
		if(!shm) {
			ac->num = 0;
			CloseHandle(fmh);
			continue;
		}

		ac->ai.hshm = fmh;
		ac->ai.shm = shm;

		memset(shm, 0x00, 4096);
		shm->sid = ac->num;
		
		if( !aconfig[ac->master].num || !ac->master ) ac->master = ac->num;
		shm->master = ac->master;
		ac->master_ac = &aconfig[ac->master];

		if(ac != ac->master_ac) {
			strcpy(ac->gname, ac->master_ac->gname);
			strcpy(ac->gpass, ac->master_ac->gpass);
		}

		sprintf(shm->master_shm_name, "%X_%02d", GetCurrentProcessId(), ac->master);
		strcpy(shm->script, ac->script);
		memcpy(shm->script_config, ac->script_config, ac->script_config_size);
		shm->script_config_size = ac->script_config_size;

		sprintf(d2exec, "%s -s %s", ac->d2exec, &buf[5]);
		strncpy(ac->d2exec, d2exec, sizeof(ac->d2exec));
	}

}

static void free_shm() {
	PAccountConfig ac = &aconfig[1];
	for(int i = 1; i < ACONFIG_SIZE; i++, ac++) {
		if( !ac->num ) continue;

		UnmapViewOfFile(ac->ai.shm);
		CloseHandle(ac->ai.hshm);
	}
}

static void release_cdkey() {
	PAccountConfig ac = &aconfig[1];
	for(int i = 1; i < ACONFIG_SIZE; i++, ac++) {
		if( !ac->num || !ac->ai.cdkey.key0 ) continue;
	
		ac->ai.cdkey.count = ac->ai.cur_gcount;
		__dlog(ac->num, "PutKey#%d, UsedCount:%d", get_cdkey_idx(ac->ai.cdkey.key0), ac->ai.cdkey.count);
		cdkeymgr.put_key(ac->ai.cdkey);
	}
}

