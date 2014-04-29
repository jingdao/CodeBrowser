#include "HashTable.h"

int insert(HashTable* tb, int intKey, void* entry);

HashTable* InitHashTable() {
	HashTable* tb = malloc(sizeof(HashTable));
	//printf("malloc HashTable: %d\n",sizeof(HashTable)+sizeof(int)+INITIAL_TABLE_SIZE*(sizeof(void*)+sizeof(int)));
	if (!tb) return NULL;
	//claims a section of the heap to be used as dummy variable
	tb->dummy = malloc(sizeof(int));
	if (!tb->dummy) {
		free(tb);
		return NULL;
	}
	tb->keys = malloc(INITIAL_TABLE_SIZE*sizeof(int));
	if (!tb->keys) {
		free(tb);
		free(tb->dummy);
		return NULL;
	}
	tb->entries = malloc(INITIAL_TABLE_SIZE*sizeof(void*));
	if (!tb->entries) {
		free(tb);
		free(tb->dummy);
		free(tb->keys);
		return NULL;
	}
	tb->size = INITIAL_TABLE_SIZE;
	tb->load = 0;
	unsigned int i;
	for (i = 0; i < tb->size; i++) {
		tb->keys[i] = -1;
		tb->entries[i] = NULL;
	}
	return tb;
}

int getIntKey(char* inputString) {
	//djb2 algorithm http://www.cse.yorku.ca/~oz/hash.html
	int h = STRING_HASH_CONSTANT;
	unsigned int c;
	while ((c = *inputString++))
		h = (h << 5) + h + c;
	if (h<0) h=-h;
	return h;
}

unsigned int baseHash(unsigned int size, int hashKey) {
	return (unsigned int)(size*((BASE_HASH_CONSTANT*hashKey) - (int)(BASE_HASH_CONSTANT*hashKey)));
}

unsigned int stepHash(unsigned int size, int hashKey) {
	//need to decrease size by one and add one at the end becase stepHash must not return 0
	return (unsigned int)((size - 1)*((STEP_HASH_CONSTANT*hashKey) - (int)(STEP_HASH_CONSTANT*hashKey))) + 1;
}

void DeleteHashTable(HashTable* tb) {
	if (tb) {
		if (tb->dummy) free(tb->dummy);
		if (tb->keys) free(tb->keys);
		if (tb->entries) free(tb->entries);
		free(tb);
	}
}

void DoubleHashTable(HashTable* tb) {
	if (!tb) return;
	//printf("Now doubling size of hash table\n");
	//int newSize = tb->size*2;
	int newSize = (tb->size+1)*2-1; //approximate doubling with chance of prime number
	int* newKeys = malloc(newSize * sizeof(int));
	void** newEntries = malloc(newSize*sizeof(void*));
	//printf("malloc double HashTable: %d\n",newSize * sizeof(int)+sizeof(void*));
	int* oldKeys = tb->keys;
	void** oldEntries = tb->entries;
	unsigned int oldSize = tb->size;
	if (!newEntries||!newKeys) {
		printf("Warning(HashTable): Unable to resize table!\n");
		return;
	}
	tb->keys = newKeys;
	tb->entries = newEntries;
	tb->size =newSize;
	tb->load = 0;
	unsigned int i;
	for (i = 0; i < tb->size; i++) {
		tb->keys[i] = -1;
		tb->entries[i] = NULL;
	}
	for (i = 0; i < oldSize; i++) {
		if (oldEntries[i] && oldEntries[i] != tb->dummy)
			insert(tb, oldKeys[i], oldEntries[i]);
	}
	free(oldKeys);
	free(oldEntries);
	
}

int insert(HashTable* tb, int intKey, void* entry) {
	int h = baseHash(tb->size, intKey);
	int step = stepHash(tb->size, intKey);
	unsigned int i;
	for (i = 0; i < tb->size; i++) {
		//printf("key: %d hash: %d\n", intKey, h);
		if (!tb->entries[h] || tb->entries[h] == tb->dummy) {
			tb->keys[h] = intKey;
			tb->entries[h] = entry;
			tb->load++;
			if (tb->load>tb->size / 4) {
				//printf("current load: %u/%u\n", tb->load, tb->size);
				DoubleHashTable(tb);
			}
			return 1;
		}
		else if (tb->keys[h] == intKey){
			return 0;
		}
		else {
			h += step;
			h %= tb->size;
		}
	}
	printf("Table overload (HashTable)!\n");
	return 0;
}

int InsertIntoHashTable(HashTable* tb, char* key, void* entry) {
	if (!tb) return 0;
	int intKey = getIntKey(key);
	return insert(tb, intKey, entry);
}

void* FindInHashTable(HashTable* tb, char* key) {
	if (!tb) return NULL;
	int intKey = getIntKey(key);
	int h = baseHash(tb->size, intKey);
	int step = stepHash(tb->size, intKey);
	unsigned int i;
	for (i = 0; i < tb->size; i++) {
		//printf("key: %d hash: %d\n", intKey, h);
		if (!tb->entries[h]) {
			return NULL;
		}
		else if (tb->keys[h] == intKey){
			return tb->entries[h];
		}
		else {
			h += step;
			h %= tb->size;
		}
	}
	printf("Table overload (HashTable)!\n");
	return NULL;
}

int RemoveFromHashTable(HashTable* tb, char* key) {
	if (!tb) return 0;
	int intKey = getIntKey(key);
	int h = baseHash(tb->size, intKey);
	int step = stepHash(tb->size, intKey);
	unsigned int i;
	for (i = 0; i < tb->size; i++) {
		//printf("key: %d hash: %d\n", intKey, h);
		if (!tb->entries[h]) {
			return 0;
		}
		else if (tb->entries[h]!=tb->dummy && tb->keys[h] == intKey ){
			tb->entries[h]=tb->dummy;
			tb->load--;
			return 1;
		}
		else {
			h += step;
			h %= tb->size;
		}
	}
	printf("Table overload (HashTable)!\n");
	return 0;
}

