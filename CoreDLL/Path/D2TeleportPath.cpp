#include "D2TeleportPath.h"
#include <deque>

static const char _rng_tbl[] = {0, 8, 11, 14, 16, 18, 19, 21, 22, 23, 24, 25, 26, 27, 28, 28, 29, 30, 30, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 35, 34, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 32, 32, 32, 31, 31, 30, 30, 29, 28, 28, 27, 26, 25, 24, 23, 22, 21, 19, 18, 16, 14, 11, 8, 0 };
static const char _crp_tbl[] = {-1, 0,   +1, 0,   0, -1,   0, +1};

D2TeleportPath::D2TeleportPath(unsigned short *map, int sx, int sy) {
	_map = map;
	_sx = sx;
	_sy = sy;

}

D2TeleportPath::~D2TeleportPath() {
}

DWORD D2TeleportPath::FindTeleportPath(POINT start, POINT end, LPPOINT buf, DWORD buf_size) {
	std::deque<POINT> qs, qt, qp;
	POINT pt, ps;
	int x, y, _x, _y;
	unsigned short m_val;
	//int ret = 0;
	int lvl = 0;

	if(start.x == end.x && start.y == end.y) return 0;

	set(start.x, start.y, 0x0400);
	qs.push_back(start);

	while(1) {
		if( qs.empty() ) {
			while( !qt.empty() ) {
				pt = qt.front();
				qt.pop_front();
				x = pt.x & 0xFFFF; y = pt.y & 0xFFFF;
				if( !is_on_edge(x, y) && get(x, y - 1) & 0x0E00 && get(x, y + 1) & 0x0E00 && get(x - 1, y) & 0x0E00 && get(x + 1, y) & 0x0E00 ) {
					set(x, y, 0x0200); continue;
				}
				qs.push_back(pt);
			}

			lvl++;
			if( qs.empty() ) break;
		}

		ps = qs.front();
		qs.pop_front();
		qp.push_back(ps);
		_x = ps.x & 0xFFFF;
		_y = ps.y & 0xFFFF;
		set(_x, _y, 0x0800 | lvl);

		//in range
		if( end.x - _x <= 35 && end.x - _x >= -35 && end.y - _y <= _rng_tbl[end.x - _x + 35] && end.y - _y >= -_rng_tbl[end.x - _x + 35]) {
			DWORD count = 1;
			buf[0].x = start.x;
			buf[0].y = start.y;

			if(lvl >= buf_size) {
				while(lvl >= buf_size) {
					_x = ((unsigned int)ps.x) >> 16;
					_y = ((unsigned int)ps.y) >> 16;
					while(1) {
						ps = qp.back();
						qp.pop_back();
						if(_x == (ps.x & 0xFFFF) && _y == (ps.y & 0xFFFF) ) break;
					}
					lvl--;
				}

			} else if(lvl < buf_size - 1) {
				buf[lvl + 1].x = end.x;
				buf[lvl + 1].y = end.y;
				count = 2;
			}

			count += lvl;
			while(lvl > 0) {
				buf[lvl].x = _x;
				buf[lvl].y = _y;
				_x = ((unsigned int)ps.x) >> 16;
				_y = ((unsigned int)ps.y) >> 16;
				while(1) {
					ps = qp.back();
					qp.pop_back();
					if(_x == (ps.x & 0xFFFF) && _y == (ps.y & 0xFFFF) ) break;
				}
				lvl--;
			}

			return count;
		}

		int bval[] = {35, 35, 35, 35};
		int xi, yi;
		for(int c = 0; c < 4; c++) {
			xi = _crp_tbl[ (c << 1) + 0 ];
			yi = _crp_tbl[ (c << 1) + 1 ];
			x = _x + xi;
			y = _y + yi;
			for(int r = 1; r <= 68; r++, x += xi, y += yi) {
				if( !is_valid(x, y) ) continue;

				m_val = get(x, y);
				if( m_val & 0x0C00 ) {
					if( (m_val & 0x00FF) < lvl ) {
						bval[c] = (r - 1) / 2;
					} else if( (m_val & 0x00FF) == lvl ) {
						bval[c] = (r + ( (m_val & 0x0400) ? 0 : -1 ) ) / 2;
					}

					break;
				}

			} //end loop_r
		}

		int x_s, x_e, y_s, y_e, max_x, max_y;

		y_s = -bval[2];
		y_e = +bval[3];
		y = _y + y_s;
		for(; y_s <= y_e; y_s++, y++) {
			if( !is_y_valid(y) ) continue;

			max_x = _rng_tbl[y_s + 35];
			x_s = max(-bval[0], -max_x);
			x_e = min(+bval[1], +max_x);
			x = _x + x_s;

			for(; x_s <= x_e; x_s++, x++) {
				if( !is_x_valid(x) ) continue;

				m_val = get(x, y);
				if(m_val & 0x0F00) continue;

				max_y = _rng_tbl[x_s + 35];
				if(x_s != +max_x && x_s != -max_x && y_s != +max_y && y_s != -max_y) {
					if( !is_on_edge(x, y) && !(get(x, y - 1) & 0x0100 || get(x, y + 1) & 0x0100 || get(x - 1, y) & 0x0100 || get(x + 1, y) & 0x0100) ) {
						set(x, y, 0x0200); continue;
					}
				}

				pt.x = x | (_x << 16);
				pt.y = y | (_y << 16);
				qt.push_back(pt);
				set(x, y, 0x0400 | (lvl + 1));
			}
		}

	} //end main_loop

	return 0;
}
