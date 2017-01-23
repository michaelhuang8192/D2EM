#include <windows.h>
#include <stdio.h>
#include "mongoose.h"
#include "webui.h"

extern void get_response(PEMPluginParam pparm, int s_idx, char *buf, int size);

static void mg_default_callback(struct mg_connection *conn,
								const struct mg_request_info *request_info,
								void *user_data)
{
	const char *p;
	char res[4096];
	int s_idx = 0;
	if( request_info->query_string && (p=strstr(request_info->query_string, "s=")) )
		s_idx = atoi(p + 2);

	mg_printf(conn, "%s", "HTTP/1.1 200 OK\r\n");
	mg_printf(conn, "%s", "Content-Type: text/html\r\n\r\n");

	get_response( (PEMPluginParam)user_data, s_idx, res, sizeof(res) );
	mg_printf(conn, "%s", res);
}

__declspec(dllexport) DWORD WINAPI webui_thread_proc(LPVOID lpParameter)
{
	struct mg_context *ctx;
	PEMPluginParam pparm = (PEMPluginParam)lpParameter;
	PWebUIConfig wc = pparm->wc;
	char s_port[32];
	itoa( (int)wc->port, s_port, 10 );

	ctx = mg_start();
	mg_set_conn_req_callback(ctx, &mg_default_callback, lpParameter, wc->user, wc->pass);
	mg_set_option(ctx, "ports", s_port);
	WaitForSingleObject(pparm->evt_exit, INFINITE);
	mg_stop(ctx);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	return 1;
}
