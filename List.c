#include "List.h"

List* InitList() {
	List* ls = (List*)malloc(sizeof(List));
	//printf("malloc List: %d\n",sizeof(List)+INITIAL_LIST_SIZE*sizeof(void*));
	if (!ls) return NULL;
	ls->entries = (void**)malloc(INITIAL_LIST_SIZE*sizeof(void*));
	if (!ls->entries) {
		free(ls);
		return NULL;
	}
	ls->maxSize = INITIAL_LIST_SIZE;
	ls->size = 0;
	// unsigned int i;
	// for (i = 0; i < ls->maxSize; i++) {
	// 	ls->entries[i] = NULL;
	// }
	memset(ls->entries,'\0',INITIAL_LIST_SIZE*sizeof(void*));
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
		void** newEntries = (void**)realloc(ls->entries,ls->maxSize*2*sizeof(void*));
		//printf("malloc double List: %d\n",ls->maxSize*2*sizeof(void*));
		if (!newEntries) {
			printf("Warning(List): Unable to resize table!\n");
			return 0;
		}
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

List* IntersectList(List* lsa, List* lsb) {
	if (!lsa||!lsb) {
		return NULL;
	}
	List* combined = InitList();
	//IMPROVE algorithm later
	//TODO intersect lists with repeats, color highlight correct word
	unsigned int i,j;
	for (i = 0; i < lsa->size; i++) {
		printf("lsa: %p\n",GetListItem(lsa,i));
	}
	for (i = 0; i < lsb->size; i++) {
		printf("lsb: %p\n",GetListItem(lsb,i));
	}
	for (i = 0; i < lsa->size; i++) {
		for (j = 0; j < lsb->size; j++) {
			void* p = GetListItem(lsa, i);
			if (GetListItem(lsb, j) == p) {
				AppendToList(combined, p);
			}
		}
	}
	for (i = 0; i < combined->size; i++) {
		printf("combined: %p\n",GetListItem(combined,i));
	}
	return combined;
}
