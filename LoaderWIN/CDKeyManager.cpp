#include "CDKeyManager.h"
#include <time.h>

CDKeyManager::CDKeyManager() {
}

CDKeyManager::~CDKeyManager() {
}

int CDKeyManager::get_key(CDKeyEntry &cdkey) {
	if( _pq.empty() ) return 0;

	cdkey = _pq.top();
	cdkey.count = 0;
	_pq.pop();

	if( cdkey.ts ) {
		unsigned int ts = (unsigned int)time(0);
		if( *cdkey.ts <= ts ) {
			*cdkey.ts = ts;

		} else if( !_pq.empty() ) {
			unsigned int *pnts = _pq.top().ts;
			if(pnts) {
				unsigned int ct = (*pnts - *cdkey.ts) / MIN_SEC_PER_GAME;
				cdkey.count = ct > MIN_RCOUNT ? (ct > MAX_GAME_PER_HOUR ? MAX_GAME_PER_HOUR : ct) : MIN_RCOUNT;
			}
		}
	}

	return 1;
}

int CDKeyManager::put_key(const CDKeyEntry &cdkey) {
	if(cdkey.ts) {
		unsigned int ts = (unsigned int)time(0);
		unsigned int cts = *cdkey.ts + cdkey.count * MIN_SEC_PER_GAME;
		*cdkey.ts = ts > cts ? ts : cts;
	}

	_pq.push(cdkey);

	return 1;
}

int CDKeyManager::add_key(const CDKeyEntry &cdkey) {
	_pq.push(cdkey);
	return 1;
}
