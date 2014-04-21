#include <stdio.h>
#include <stdlib.h>
#include "List.h"
#include "HashTable.h"

typedef struct {
	unsigned int size;
	HashTable* tb;
} Map;


Map* InitMap();
void DeleteMap(Map* mp);
int AddToMap(Map* mp, char* key, void* value);
List* GetListFromMap(Map* mp,char* key);