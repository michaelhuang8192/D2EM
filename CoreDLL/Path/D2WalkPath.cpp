#include "D2WalkPath.h"
#include <queue>
#include <stack>
#include <math.h>

#define MAX_WP_DIST (25 * 100)
#define MIN_WP_DIST (15 * 100)

static const char _rng_tbl[][2] =
{
	{-1, +0}, //0
	{-1, +1}, //1
	{+0, +1}, //2
	{+1, +1}, //3
	{+1, +0}, //4
	{+1, -1}, //5
	{+0, -1}, //6
	{-1, -1}, //7
};

static const char _rng_idx[][2] = {
	{7, 2}, //0
	{7, 4}, //1
	{1, 4}, //2
	{1, 6}, //3
	{3, 6}, //4
	{3, 0}, //5
	{5, 0}, //6
	{5, 2}, //7

};

static const unsigned int _rng_wt[] = {
	100, 142, 100, 142, 100, 142, 100, 142
};

#define RNG_TBL_SIZE (sizeof(_rng_tbl) / sizeof(_rng_tbl[0]))

D2WalkPath::D2WalkPath(unsigned short *map, int sx, int sy) {
	_map = map;
	_sx = sx;
	_sy = sy;

	_map_wt = (PWP_WT_VAL)malloc(sx * sy * sizeof(WP_WT_VAL));
}

D2WalkPath::~D2WalkPath() {
	free(_map_wt);

}

typedef struct _MapPoint {
	unsigned int t_dist;
	int x;
	int y;
	unsigned short si;
	unsigned short ei;

} MapPoint, *PMapPoint;

struct MapPointCMP {
	bool operator()(const MapPoint &a, const MapPoint &b) const {
		return a.t_dist >= b.t_dist;
	}
};

static unsigned int get_dist(int sx, int sy, int dx, int dy) {
	return (unsigned int)(sqrt(double((dy - sy) * (dy - sy) + (dx - sx) * (dx - sx))) * 100);
}

//A*
DWORD D2WalkPath::FindWalkPath(POINT start, POINT end, LPPOINT buf, DWORD buf_size) {
	if(start.x == end.x && start.y == end.y) return 0;
	
	std::priority_queue<MapPoint, std::vector<MapPoint>, MapPointCMP> qs;
	MapPoint pt, *pp;
	int s_idx, e_idx, r_idx, px, py, x, y;
	unsigned short m_val;
	WP_WT_VAL *pwt_val, *pwt_val_n;
	unsigned int g;

	pwt_val = get_wt(start.x, start.y);
	pwt_val->g = 0;
	pwt_val->h = get_dist(start.x, start.y, end.x, end.y);

	pt.x = start.x;
	pt.y = start.y;
	pt.si = 0;
	pt.ei = 0;
	qs.push(pt);

	while( !qs.empty() ) {
		pp = &qs.top();
		px = pp->x;
		py = pp->y;
		s_idx = pp->si;
		e_idx = pp->si < pp->ei ? pp->ei: pp->ei + RNG_TBL_SIZE;
		qs.pop();

		m_val = get(px, py);
		if(m_val & 0x04) continue;
		set(px, py, m_val | 0x04);
	
		if(px == end.x && py == end.y) return calc_path(start, end, buf, buf_size);

		pwt_val = get_wt(px, py);
		for(; s_idx < e_idx; s_idx++) {
			r_idx = s_idx % RNG_TBL_SIZE;
			x = px + _rng_tbl[r_idx][0];
			y = py + _rng_tbl[r_idx][1];
			if( !is_valid(x, y) ) continue;

			m_val = get(x, y);
			if(m_val & 0x05) continue;

			g = pwt_val->g + _rng_wt[r_idx];
			pwt_val_n = get_wt(x, y);
			if(m_val & 0x02) {
				if(g >= pwt_val_n->g) continue;
				m_val &= 0x0FFF;
			} else {
				pwt_val_n->h = get_dist(x, y, end.x, end.y);
				m_val |= 0x02;
			}

			pwt_val_n->g = g;
			pt.t_dist = pwt_val_n->g + pwt_val_n->h;
			pt.x = x;
			pt.y = y;
			pt.si = _rng_idx[r_idx][0];
			pt.ei = _rng_idx[r_idx][1];
			set(x, y, m_val | ((-_rng_tbl[r_idx][0])&0x03)<<12 | ((-_rng_tbl[r_idx][1])&0x03)<<14 );
			qs.push(pt);
		}

	}

	return 0;
}

DWORD D2WalkPath::calc_path(POINT start, POINT end, LPPOINT buf, DWORD buf_size) {
	unsigned short m_val;
	DWORD ret = 0;
	int x, y, lx, ly, dist;
	std::stack<POINT> _stk_pt;	
	POINT pt, ptt;
	int d_x, d_y, t_d_x, t_d_y;

	x = end.x;
	y = end.y;
	d_x = d_y = 255;
	dist = 0;
	while(x != start.x || y != start.y) {
		lx = x;
		ly = y;
		m_val = get(x, y);
		x += ((m_val>>12)&0x03) == 0x03 ? -1 : ((m_val>>12)&0x03);
		y += ((m_val>>14)&0x03) == 0x03 ? -1 : ((m_val>>14)&0x03);
		t_d_x = lx - x;
		t_d_y = ly - y;

		if(d_x != t_d_x || d_y != t_d_y) {
			d_x = t_d_x;
			d_y = t_d_y;
		} else {
			dist += _rng_wt[ d_x + d_y + 3 ];
			if(dist <= MAX_WP_DIST) continue;
		}

		dist = _rng_wt[ d_x + d_y + 3 ];
		pt.x = lx;
		pt.y = ly;
		_stk_pt.push(pt);
	}

	buf[ret].x = lx = start.x;
	buf[ret].y = ly = start.y;
	ret++;
	while( !_stk_pt.empty() ) {
		pt = _stk_pt.top();
		_stk_pt.pop();

		if(get_dist(lx, ly, pt.x, pt.y) < MIN_WP_DIST) {
			while( !_stk_pt.empty() ) {
				ptt = _stk_pt.top();
				if(get_dist(pt.x, pt.y, ptt.x, ptt.y) >= MIN_WP_DIST) break;
				if(get_dist(lx, ly, ptt.x, ptt.y) > MAX_WP_DIST) break;
				
				_chk_pt_result = 1;
				LineDDA(lx, ly, ptt.x, ptt.y, check_point, (LPARAM)this);
				if(!_chk_pt_result) break;

				pt = ptt;
				_stk_pt.pop();
			}
		}

		buf[ret].x = lx = pt.x;
		buf[ret].y = ly = pt.y;
		ret++;

		if(ret >= buf_size) break;
	}

	return ret;
}

void CALLBACK D2WalkPath::check_point(int x, int y, LPARAM lp) {
	D2WalkPath *wp = (D2WalkPath*)lp;
	if(wp->_chk_pt_result && wp->is_valid(x, y) && (wp->get(x, y) & 0x01)) wp->_chk_pt_result = 0;
}

