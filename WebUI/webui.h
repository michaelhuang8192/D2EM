#ifndef __D2EM_WEBUI__
#define __D2EM_WEBUI__

#include <windows.h>


typedef struct _CDKeyEntry {
	const char *key0;
	const char *key1;
	unsigned int *ts;
	unsigned int count;

} CDKeyEntry, *PCDKeyEntry;

#define MAX_ACCOUNT 8
#define ACONFIG_SIZE (1 + MAX_ACCOUNT)

struct __MS_SharedSection {
	unsigned int seq_num;
	unsigned int data[4];
};

typedef struct _D2EM_SHM {
	unsigned int sid;
	unsigned long tick;
	unsigned int gs_state;
	unsigned int gc_state;
	
	char script[256];
	char script_config[1024];
	unsigned int script_config_size;

	unsigned int game_count;
	unsigned int srate;

	unsigned int le_mode;

	char master_shm_name[64];
	unsigned int seq_num;
	unsigned int master;
	struct __MS_SharedSection ms_ss[ACONFIG_SIZE];

} D2EM_SHM, *PD2EM_SHM;

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
	struct _AccountConfig *master_ac;
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

enum CS_PROC {
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
	CS_MAX,
};

#define WM_WEBUI_CMD (WM_APP + 99)

#endif
