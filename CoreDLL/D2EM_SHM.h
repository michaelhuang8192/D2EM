#ifndef __D2EM_SHM__
#define __D2EM_SHM__

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
	__MS_SharedSection ms_ss[ACONFIG_SIZE];

} D2EM_SHM, *PD2EM_SHM;

#endif

