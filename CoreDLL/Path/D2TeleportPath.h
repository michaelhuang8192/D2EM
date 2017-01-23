#ifndef __D2TELEPORTPATH__
#define __D2TELEPORTPATH__

#include <windows.h>

class D2TeleportPath {
public:
	D2TeleportPath(unsigned short *map, int sx, int sy);
	~D2TeleportPath();
	DWORD FindTeleportPath(POINT start, POINT end, LPPOINT buf, DWORD buf_size);

private:
	unsigned short get(int x, int y) { return *(_map + y * _sx + x); }
	void set(int x, int y, unsigned short val) { *(_map + y * _sx + x) = val; }
	int is_valid(int x, int y) { return x >= 0 && x < _sx && y >= 0 && y < _sy; }
	int is_x_valid(int x) { return x >= 0 && x < _sx; }
	int is_y_valid(int y) { return y >= 0 && y < _sy; }
	int is_on_edge(int x, int y) { return x <= 0 || x >= _sx - 1 || y <= 0 || y >= _sy - 1; }

	unsigned short *_map;
	int _sx;
	int _sy;
};

#endif

