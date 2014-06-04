#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_HASH_CONSTANT 5381
#define BASE_HASH_CONSTANT 0.618033988
#define STEP_HASH_CONSTANT 0.707106781
#define INITIAL_TABLE_SIZE 7

typedef struct {
	unsigned int size;
	unsigned int load;
	int* dummy;
	int* keys;
	void** entries;
} HashTable;

HashTable* InitHashTable();
void DeleteHashTable(HashTable* tb);
int InsertIntoHashTable(HashTable* tb, char* key, void* entry);
void* FindInHashTable(HashTable* tb, char* key);
int RemoveFromHashTable(HashTable* tb, char* key);