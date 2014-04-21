#include "List.h"

List* InitList() {
	List* ls = malloc(sizeof(List));
	//printf("malloc List: %d\n",sizeof(List)+INITIAL_LIST_SIZE*sizeof(void*));
	if (!ls) return NULL;
	ls->entries = malloc(INITIAL_LIST_SIZE*sizeof(void*));
	if (!ls->entries) {
		free(ls);
		return NULL;
	}
	ls->maxSize = INITIAL_LIST_SIZE;
	ls->size = 0;
	for (unsigned int i = 0; i < ls->maxSize; i++) {
		ls->entries[i] = NULL;
	}
	return ls;
}

void DeleteList(List* ls) {
	if (ls) {
		if (ls->entries) free(ls->entries);
		free(ls);
	}
}

int AppendToList(List* ls, void* entry) {
	if (!ls) return 0;
	char* ns = entry;
	if (ls->maxSize==ls->size) {
		//printf("Now doubling size of list\n");
		void** newEntries = malloc(ls->maxSize*2*sizeof(void*));
		//printf("malloc double List: %d\n",ls->maxSize*2*sizeof(void*));
		if (!newEntries) {
			printf("Warning(List): Unable to resize table!\n");
			return 0;
		}
		for (unsigned int i=0;i<ls->size;i++) {
			newEntries[i]=ls->entries[i];
		}
		for (unsigned int i=ls->size;i<ls->size*2;i++) {
			newEntries[i]=NULL;
		}
		free(ls->entries);
		ls->entries = newEntries;
		ls->maxSize *= 2;
	}
	ls->entries[ls->size]=entry;
	ls->size++;
	return 1;
}

void* GetListItem(List* ls, unsigned int index) {
	if (!ls||index>=ls->size) return NULL;
	return ls->entries[index];
}
