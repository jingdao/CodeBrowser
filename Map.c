#include "Map.h"

Map* InitMap() {
	Map* mp = (Map*)malloc(sizeof(Map));
	//printf("malloc Map: %d\n",sizeof(Map));
	if (!mp) return NULL;
	mp->tb = InitHashTable();
	mp->size = 0;
	if (mp->tb) return mp;
	else {
		free(mp);
		return NULL;
	}
}

void DeleteMap(Map* mp) {
	if (!mp) return;
	unsigned int i;
	for (i=0; i<mp->tb->size; i++) {
		void* ls = mp->tb->entries[i];
		if (ls&&ls!=mp->tb->dummy) DeleteList(ls);	
	}
	DeleteHashTable(mp->tb);
	free(mp);
}

int AddToMap(Map* mp, char* key, void* value) {
	if (!mp) return 0;
	List* ls = FindInHashTable(mp->tb,key);
	if (!ls) {
		ls = InitList();
		if (!InsertIntoHashTable(mp->tb,key,ls)) {
			printf("Warning(Map): cannot insert %s into hash table!\n",key);
			return 0;
		}
		mp->size++;
	}
	if (!AppendToList(ls,value)) {
		printf("Warning(Map): cannot append to list!\n");
		return 0;
	}
	return 1;
}

List* GetListFromMap(Map* mp, char* key) {
	if (!mp) return NULL;
	List* ls = FindInHashTable(mp->tb,key);
	return ls; 
}
