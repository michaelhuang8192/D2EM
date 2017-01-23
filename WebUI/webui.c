#include "webui.h"
#include <stdio.h>

const char TBL_HDR[] = ""
"<html>\r\n"
"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>\r\n"
"<body>\r\n"
"<form method=\"get\" action=\"?\">\r\n"
"<table align=\"center\" border=\"1\" style=\"border-collapse: collapse; font-size:12pt; color:#669999\" bordercolor=\"#C0C0C0\">\r\n"
	"<tr bgcolor=\"#E9D3BE\" style=\"color:#336699\">\r\n"
		"<th width=\"20\">#</th>\r\n"
		"<th width=\"50\">PID</th>\r\n"
		"<th width=\"120\">Account</th>\r\n"
		"<th width=\"200\">Status</th>\r\n"
		"<th width=\"100\">Stat</th>\r\n"
		"<th width=\"80\"></th>\r\n"
	"</tr>";

const char TBL_CNT[] = "\r\n"
	"<tr>\r\n"
		"<td align=\"center\">%d</td>\r\n"
		"<td>%d</td>\r\n"
		"<td>%s</td>\r\n"
		"<td>%s</td>\r\n"
		"<td>%u:%0.2f%%</td>\r\n"
		"<td align=\"center\"><a href=\"?s=%d\">%s</a></td>\r\n"
	"</tr>";

const char TBL_FTR[] = "\r\n"
"</table>\r\n"
"</form>\r\n"
"<a href=\"?\"><p align=\"center\" style=\"font-size:12pt; color:#336699\">[Refresh]</p></a>\r\n"
"</body>\r\n"
"</html>";

const char* STATUS_STR[] = {
	"",
	"Creating Process[%us]",
	"CreateProcess::Waiting",
	"Logging In",
	"LogIn::Waiting",
	"Selecting Character",
	"SelectCharacter::Waiting",
	"Creating Game[%us]",
	"CreateGame::Waiting",
	"InGame[%s%u:%s]",
	"InGame[%s%u:%s]",
	"Waiting For Master",
	"Closing Process",
	"Waiting To Create Process",
	"Waiting To Close Process",
};

const char* get_cs_status(PAccountConfig ac, char *b) {
	unsigned int state = ac->ai.state;
	if(state < 1 && state >= CS_MAX) return STATUS_STR[0];

	if(state == CS_PRE_MONITOR_GAME || state == CS_MONITOR_GAME) {
		sprintf(b, STATUS_STR[state], ac->gname, ac->ai.gn_seq_num, ac->gpass);

	} else if(state == CS_CREATE_GAME) {
		sprintf(b, STATUS_STR[state], (ac->ai.delay_ms + 999) / 1000);

	} else if(state == CS_CREATE_PROCESS) {
		sprintf(b, STATUS_STR[state], (ac->ai.delay_ms + 999) / 1000);

	} else {
		return STATUS_STR[state];

	}

	return b;
}

void get_response(PEMPluginParam pparm, int s_idx, char *buf, int size) {
	int slen, i;
	int cur_len = 0;
	char cnt[512], status[128];
	PAccountConfig ac = &pparm->ac[1];
	const int ac_size = (int)pparm->ac_size;

	buf[0] = 0x00;
	if( sizeof(TBL_HDR) + sizeof(TBL_FTR) - 1 > size ) return;

	if(s_idx >= 1 && s_idx < ac_size) SendMessage(pparm->mwin, WM_WEBUI_CMD, 0, s_idx - 1);

	memcpy(buf, TBL_HDR, sizeof(TBL_HDR) - 1);
	cur_len += sizeof(TBL_HDR) - 1;

	for(i = 1; i < ac_size; i++, ac++) {
		if(!ac->num) continue;

		slen = sprintf(cnt, TBL_CNT, ac->num,
			ac->ai.pi.dwProcessId,
			ac->user,
			get_cs_status(ac, status),
			ac->ai.shm->game_count,
			(float)(ac->ai.shm->srate) / 100.0,
			ac->num,
			"Start/Stop");

		if( slen <= 0 || (unsigned int)(cur_len + slen) > size - sizeof(TBL_FTR) ) break;

		memcpy(&buf[cur_len], cnt, slen);
		cur_len += slen;
	}

	memcpy(&buf[cur_len], TBL_FTR, sizeof(TBL_FTR));

}
