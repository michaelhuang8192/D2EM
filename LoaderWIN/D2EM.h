#ifndef __D2EM__
#define __D2EM__

#include <windows.h>
#include "res.h"
#include "CDKeyManager.h"
#include "D2EM_SHM.h"

typedef struct _AccountInstance {
	PROCESS_INFORMATION pi;
	HANDLE hshm;
	PD2EM_SHM shm;
	HWND hwnd;
	CDKeyEntry cdkey;

	unsigned int min_wnd;
	unsigned int tick_check;
	unsigned int last_tick;
	unsigned int last_tick_ts;

	unsigned int quit_ts;
	unsigned int realm_fail_count;

	unsigned int is_running;
	unsigned int state;
	unsigned int time_stamp;
	unsigned int delay_ms;
	unsigned int delay;
	unsigned int timeout_ms;
	unsigned int timeout;
	unsigned int last_error_status;

	unsigned int cur_gcount;
	unsigned int max_gcount;
	unsigned int gcount_ts;

	unsigned int gc_retry_count;
	unsigned int gn_seq_num;
	unsigned int gc_start_ts;
	unsigned int gc_fail_count;

	unsigned int o_gs_state;
	unsigned int o_gc_state;

	unsigned int last_seq_num;

	HANDLE cs_thread;
	unsigned int cs_thread_ts;
	int cs_thread_result;

} AccountInstance, *PAccountInstance;

typedef struct _AccountConfig {
	_AccountConfig *master_ac;
	AccountInstance ai;
	unsigned int num;
	unsigned int gminsec;
	unsigned int gmaxsec;
	unsigned int kmaxgame;
	unsigned int kminasec;
	unsigned int stoponrd;
	unsigned int cpos;
	unsigned int gdiff;
	unsigned int master;
	char user[32];
	char pass[32];
	char gname[32];
	char gpass[32];
	char d2exec[256];
	char script[256];
	char script_config[1024];
	unsigned int script_config_size;

} AccountConfig, *PAccountConfig;

typedef struct _WebUIConfig {
	unsigned int enable;
	unsigned int port;
	char user[32];
	char pass[32];

} WebUIConfig, *PWebUIConfig;

extern AccountConfig aconfig[ACONFIG_SIZE];

typedef struct _IFCDKeyEntry {
	char key0[16];
	char key1[16];
	unsigned int ts;

} IFCDKeyEntry, *PIFCDKeyEntry;

#define IFCDKEY_SIZE ( 4096 * 4 / sizeof(IFCDKeyEntry) )
extern IFCDKeyEntry ifcdkey[IFCDKEY_SIZE];

#define MAX_GAME_PER_KEY 20

int get_cdkey_idx(const char *key0);
void __dlog(unsigned int num, const char *fmt, ...);

typedef struct _EMPluginParam {
	WebUIConfig *wc;
	unsigned int ac_size;
	AccountConfig *ac;
	void (__cdecl *dlog)(unsigned int num, const char *fmt, ...);
	HWND mwin;
	HWND lv;
	HWND sb;
	HANDLE evt_exit;

} EMPluginParam, *PEMPluginParam;

#define WM_WEBUI_CMD (WM_APP + 99)

#endif
