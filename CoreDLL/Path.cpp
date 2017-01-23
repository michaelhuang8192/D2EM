#include "Path/D2TeleportPath.h"
#include "Path/D2WalkPath.h"
#include <stdio.h>

extern void __dprintf(const char *fmt, ...);
#define BLOCK_FLAG ( (1<<0)|(1<<10)|(1<<11)|(1<<12) )

static void clear_point(unsigned short *map, int sx, int sy, int x, int y) {
	*( map + y * sx + x ) = 0x00;

	x -= 1;
	if(x >= 0) *( map + y * sx + x ) = 0x00;
	x += 2;
	if(x < sx) *( map + y * sx + x ) = 0x00;

	x -= 1;
	y -= 1;
	if(y >= 0) *( map + y * sx + x ) = 0x00;
	y += 2;
	if(y < sy) *( map + y * sx + x ) = 0x00;
}


static const unsigned int mp[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
static void patch_map(const unsigned short *omap, unsigned short *map, int sx, int sy) {
	int px, py, p;
	for(int y = 1; y < sy - 1; y++) {
		for(int x = 1; x < sx - 1; x++) {
			if( !(*(omap + y * sx + x) & BLOCK_FLAG) ) {
				for(p = 0; p < 4; p++) {
					px = x + mp[p][0];
					py = y + mp[p][1];
					if( *(omap + py * sx + px) & BLOCK_FLAG ) break;
				}

				if(p == 4) *(map + y * sx + x) = 0x00;
			}
		} //end for x
	} //end for y

}

int get_teleport_path(unsigned short *map, int sx, int sy, POINT src, POINT dst, LPPOINT buf, DWORD size) {
	if(src.x == 0) src.x++;
	else if(src.x == sx - 1) src.x--;

	if(src.y == 0) src.y++;
	else if(src.y == sy - 1) src.y--;

	if(dst.x == 0) dst.x++;
	else if(dst.x == sx - 1) dst.x--;

	if(dst.y == 0) dst.y++;
	else if(dst.y == sy - 1) dst.y--;

	if(src.x < 0 || src.x >= sx || src.y < 0 || src.y >= sy) return 0;
	if(dst.x < 0 || dst.x >= sx || dst.y < 0 || dst.y >= sy) return 0;

	unsigned int _tick = GetTickCount();

	unsigned short *map_cp = (unsigned short *)malloc(sx * sy * sizeof(unsigned short) );
	memset(map_cp, 0x01, sx * sy * sizeof(unsigned short) );

	unsigned short src_val = *( map + src.y * sx + src.x );
	*( map + src.y * sx + src.x ) = 0x00;
	patch_map(map, map_cp, sx, sy);
	*( map + src.y * sx + src.x ) = src_val;
	clear_point(map_cp, sx, sy, dst.x, dst.y);

	/*
	FILE *fp = fopen("c:/pk.txt", "w");
	for(int y = 0; y < sy; y++) {
		for(int x = 0; x < sx; x++) {
			if( x == src.x && y == src.y ) {
				fputc('S', fp);
			} else if( x == dst.x && y == dst.y ) {
				fputc('D', fp);
			}
			else if( *(map_cp + y * sx + x) & 0x0100 )
				fputc('X', fp);
			else
				fputc(' ', fp);
		}
		fputc('\n', fp);
	}
	fclose(fp);
	*/

	D2TeleportPath p(map_cp, sx, sy);
	int ret = p.FindTeleportPath(src, dst, buf, size);

	free(map_cp);

	__dprintf("Teleport Path[%d], MS[%d]\r\n", ret, GetTickCount() - _tick);

	return ret;
}

int get_walk_path(unsigned short *map, int sx, int sy, POINT src, POINT dst, LPPOINT buf, DWORD size) {
	if(src.x == 0) src.x++;
	else if(src.x == sx - 1) src.x--;

	if(src.y == 0) src.y++;
	else if(src.y == sy - 1) src.y--;

	if(dst.x == 0) dst.x++;
	else if(dst.x == sx - 1) dst.x--;

	if(dst.y == 0) dst.y++;
	else if(dst.y == sy - 1) dst.y--;

	if(src.x < 0 || src.x >= sx || src.y < 0 || src.y >= sy) return 0;
	if(dst.x < 0 || dst.x >= sx || dst.y < 0 || dst.y >= sy) return 0;

	unsigned int _tick = GetTickCount();

	unsigned short *map_cp = (unsigned short *)malloc(sx * sy * sizeof(unsigned short) );
	memset(map_cp, 0x01, sx * sy * sizeof(unsigned short) );

	unsigned short src_val = *( map + src.y * sx + src.x );
	*( map + src.y * sx + src.x ) = 0x00;
	patch_map(map, map_cp, sx, sy);
	*( map + src.y * sx + src.x ) = src_val;
	clear_point(map_cp, sx, sy, dst.x, dst.y);

	D2WalkPath p(map_cp, sx, sy);
	int ret = p.FindWalkPath(src, dst, buf, size);

	free(map_cp);

	__dprintf("WalkPath Path[%d], MS[%d]\r\n", ret, GetTickCount() - _tick);

	return ret;
}
