#ifndef __CDKEYMANAGER__
#define __CDKEYMANAGER__

#include <queue>

#define MAX_GAME_PER_HOUR 20
#define MIN_SEC_PER_GAME ( (3600 + MAX_GAME_PER_HOUR - 1) / MAX_GAME_PER_HOUR )
#define MIN_RCOUNT 10

typedef struct _CDKeyEntry {
	const char *key0;
	const char *key1;
	unsigned int *ts;
	unsigned int count;

} CDKeyEntry, *PCDKeyEntry;

struct _CDKeyCmpHP {
	bool operator()(const CDKeyEntry &a, const CDKeyEntry &b) const {
		if( !a.ts ) return true;
		if( !b.ts ) return false;

		return *a.ts > *b.ts;
	}
};

class CDKeyManager {
public:
	CDKeyManager();
	~CDKeyManager();

	int empty() { return _pq.empty(); }
	int size() { return _pq.size(); }
	int get_key(CDKeyEntry &cdkey);
	int put_key(const CDKeyEntry &cdkey);
	int add_key(const CDKeyEntry &cdkey);

private:
	std::priority_queue<CDKeyEntry, std::vector<CDKeyEntry>, _CDKeyCmpHP> _pq;

};

#endif
