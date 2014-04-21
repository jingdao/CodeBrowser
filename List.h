#include <stdio.h>
#include <stdlib.h>

#define INITIAL_LIST_SIZE 8

typedef struct {
	unsigned int maxSize;
	unsigned int size;
	void** entries;
} List;

List* InitList();
void DeleteList(List* ls);
int AppendToList(List* ls, void* entry);
void* GetListItem(List* ls, unsigned int index);