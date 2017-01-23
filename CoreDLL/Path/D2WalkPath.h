#ifndef __D2WALKPATH__
#define __D2WALKPATH__

#include <windows.h>

typedef struct _WP_WT_VAL {
	unsigned int g;
	unsigned int h;
} WP_WT_VAL, *PWP_WT_VAL;

class D2WalkPath {
public:
	D2WalkPath(unsigned short *map, int sx, int sy);
	~D2WalkPath();

	DWORD FindWalkPath(POINT start, POINT end, LPPOINT buf, DWORD buf_size);

private:
	DWORD calc_path(POINT start, POINT end, LPPOINT buf, DWORD buf_size);
	static void CALLBACK check_point(int x, int y, LPARAM lp);
	
	PWP_WT_VAL get_wt(int x, int y) { return (_map_wt + y * _sx + x); }
	void set_wt(int x, int y, PWP_WT_VAL val) { *(_map_wt + y * _sx + x) = *val; }
	unsigned short get(int x, int y) { return *(_map + y * _sx + x); }
	void set(int x, int y, unsigned short val) { *(_map + y * _sx + x) = val; }
	int is_valid(int x, int y) { return x >= 0 && x < _sx && y >= 0 && y < _sy; }
	int is_x_valid(int x) { return x >= 0 && x < _sx; }
	int is_y_valid(int y) { return y >= 0 && y < _sy; }
	int is_on_edge(int x, int y) { return x <= 0 || x >= _sx - 1 || y <= 0 || y >= _sy - 1; }

	unsigned short *_map;
	PWP_WT_VAL _map_wt;
	int _sx;
	int _sy;
	int _chk_pt_result;
};

#endif

