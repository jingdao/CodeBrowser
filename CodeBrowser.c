#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#define TEST_SIZE 100000
#define BUFFER_SIZE 256
#define MAX_PRINT_LINES 100
#define FILE_NAME_COLOR "\x1B[94m"
#define LINE_NUMBER_COLOR "\x1B[96m"
#define NORMAL_COLOR "\x1B[0m"
#define HIGHLIGHT_COLOR "\x1B[91m"
#define SUMMARY_COLOR "\x1B[92m"
#include "Map.h"


typedef struct {
	char* path;
	int lineNumber;
} Entry;

Map* allTokens;
HashTable* allLinesFromFile; //faster than Map for consecutive inserts on the same List
char* directory_name = NULL;
int countTokens = 0;
int countDirectories = 0;
int countLines = 0;
struct timespec start;
struct timespec end;

Entry* newEntry(char* path, int lineNumber) {
	Entry *e = malloc(sizeof(Entry));
	//printf("malloc Entry: %d\n",sizeof(Entry));
	if (!e) return NULL;
	e->path = path;
	e->lineNumber = lineNumber;
	return e;
}

char* getRandomString() {
	char* res = malloc(10*sizeof(char));
	int i;
	for (i=0;i<9;i++) {
		res[i]=rand()%24+65;
	}
	res[9]='\0';
	return res;
}

char* newString(char* c, unsigned int length) {
	int firstNonWhitespace=0;
	unsigned int i;
	for (i=0;i<length;i++) {
		if (c[i]==' '||c[i]=='\t') firstNonWhitespace++;
		else break;
	}
	char* res = malloc((length-firstNonWhitespace+1)*sizeof(char));
	//printf("malloc char: %d\n",(length-firstNonWhitespace+1)*sizeof(char));
	for (i=0;i<length-firstNonWhitespace;i++) res[i]=c[i+firstNonWhitespace];
	res[length-firstNonWhitespace]='\0';
	return res;
}

int matchString(char* filter, char* text) {
	while(*filter&&*text) {
		if (*filter=='*') {
			if (*(filter+1)==*text) filter++;
			else text++;
		}
		else if(*filter==*text) {
			filter++; text++;
		} else return 0;
	}
	if (*filter=='*') filter++;
	return *filter==*text;
}

void findAllOccurences(char* key) {
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);
	List* ls = GetListFromMap(allTokens,key);
	if (ls) {
		if (ls->size>MAX_PRINT_LINES) {
			printf("\n    %sQuery complete. Too many results (%d) to display.%s\n\n",SUMMARY_COLOR,ls->size,NORMAL_COLOR);
			return;
		}
		unsigned int i;
		for (i=0;i<ls->size;i++) {
			Entry* li = GetListItem(ls,i); 
			List* linesFromFile = FindInHashTable(allLinesFromFile,li->path);
			char* line = GetListItem(linesFromFile,li->lineNumber-1); //decrement lineNumber by 1 because of zero-indexing
			if (!li||!line||!linesFromFile) printf("Warning: null pointer or index out of bounds!\n");
			else {
				unsigned int formattedLineLength = strlen(line)+strlen(HIGHLIGHT_COLOR)+strlen(NORMAL_COLOR)+3;
				char* formattedLine = malloc(formattedLineLength*sizeof(char));
				char* token = malloc(BUFFER_SIZE*sizeof(char));
				char* leftSubstring = malloc(BUFFER_SIZE*sizeof(char));
				char* rightSubstring = malloc(BUFFER_SIZE*sizeof(char));
				unsigned int i,tokenIndex=0;
				for (i=0;i<strlen(line);i++) {
					if (!line[i]||line[i]=='\n') break;
					else if (line[i]=='_'||line[i]>='A'&&line[i]<='Z'||
						line[i]>='a'&&line[i]<='z'||tokenIndex>0&&line[i]>='0'&&line[i]<='9') {
						token[tokenIndex++]=line[i];
					} else if (tokenIndex>0) {
						token[tokenIndex]='\0';
						if (strcmp(key,token)==0) break;
						else tokenIndex=0;
					}
				}
				strncpy(leftSubstring,line,i-tokenIndex); leftSubstring[i-tokenIndex]='\0';
				strcpy(rightSubstring,line+i);
				snprintf(formattedLine,formattedLineLength,"%s%s%s%s%s",leftSubstring,HIGHLIGHT_COLOR,key,NORMAL_COLOR,rightSubstring);
				printf("%s%s:%s%d%s %s\n",FILE_NAME_COLOR,li->path,LINE_NUMBER_COLOR,li->lineNumber,NORMAL_COLOR,formattedLine);
				free(formattedLine);
				free(token);
				free(leftSubstring);
				free(rightSubstring);
			}
		}
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&end);
		unsigned int diff_usec = (end.tv_nsec-start.tv_nsec)/1000;
		printf("\n    %sQuery complete (%.3fms) %d occurences of '%s' found%s\n\n",
			SUMMARY_COLOR,(float)(diff_usec)/1000,ls->size,key,NORMAL_COLOR);
	} else {
		printf("    no occurence of '%s' found\n",key);
	}
}

int parseFromFile(char* fileName) {
	FILE *pFile;
	char string[BUFFER_SIZE];
	char token[BUFFER_SIZE];
	char* fullpath = NULL;

	fullpath = realpath(fileName,NULL);
	if (!fullpath) {
		printf("Error getting path for file %s\n",fileName);
		return 0;
	}
	char* filepath = newString(fullpath+strlen(directory_name)+1,strlen(fullpath)-strlen(directory_name)-1);
	free(fullpath);
	List* linesFromFile = InitList();
	pFile = fopen(fileName , "r");
	if (!pFile) {
		printf("Error opening file %s\n",fileName);
		return 0;
	} else {
		int lineNumber=1;
		int tokenIndex=0;
		//printf("Reading file %s ......\n",fileName);
		while(fgets(string , BUFFER_SIZE , pFile)) {
			//printf("Line %d: %s",lineNumber, string);
			unsigned int i;
			for (i=0;i<BUFFER_SIZE;i++) {
				if (!string[i]) break;
				else if (string[i]=='\n') {
					if (tokenIndex>0) {
						token[tokenIndex]='\0';
						tokenIndex=0;
						countTokens+=AddToMap(allTokens,token,newEntry(filepath,lineNumber));
					}
					break;
				} else if (string[i]=='_'||string[i]>='A'&&string[i]<='Z'||
					string[i]>='a'&&string[i]<='z'||tokenIndex>0&&string[i]>='0'&&string[i]<='9') {
					token[tokenIndex++]=string[i];
				} else if (tokenIndex>0) {
					token[tokenIndex]='\0';
					//printf("%s ",token);
					tokenIndex=0;
					countTokens+=AddToMap(allTokens,token,newEntry(filepath,lineNumber));
				}
			}
			//printf("\n");
			char* ns = newString(string,i);
			AppendToList(linesFromFile,ns);
			lineNumber++;
		}
		fclose(pFile);
		InsertIntoHashTable(allLinesFromFile,filepath,linesFromFile);
		countLines+=lineNumber;
	}

	return 1;
} 

int parseFromDirectory(List* filters) {
	struct dirent *current;
	DIR *d;
	d = opendir(".");
	if (!d) {
		printf("Cannot open current directory\n");
		return 0;
	}
	while(current = readdir(d)) {
		char* fn = current->d_name;
		if (matchString(".",fn)||matchString("..",fn)) continue;
		if (current->d_type==DT_DIR) {
			if (chdir(current->d_name)!=0) {
				printf("Cannot change directory to %s!\n",current->d_name);
			} else {
				//printf("Reading from directory %s ...\n",current->d_name);
				parseFromDirectory(filters);
				chdir("..");
			}
			continue;
		} else if (current->d_type==DT_REG) {
			if (filters) {
				unsigned int i;
				for (i=0;i<filters->size;i++) {
					if(matchString(GetListItem(filters,i),fn)) {
						if (!parseFromFile(fn)) {
							printf("Cannot parse file %s!\n",fn);
						}
						break;
					}
				}
			} else if (!parseFromFile(fn)) {
				printf("Cannot parse file %s!\n",fn);
				return 0;
			}
		}
	}
	closedir(d);
	countDirectories++;
	return 1;
}

void testInsert() {
	HashTable* tb = InitHashTable();
	char* key = "foo";
	Entry* first = newEntry("", 1);
	Entry* second = newEntry("", 2);
	if (InsertIntoHashTable(tb, key, first)) {
		printf("successfully inserted %s\n", key);
	}
	else {
		printf("insert failed for %s!\n", key);
	}
	printf("try to insert same key\n");
	if (InsertIntoHashTable(tb, key, second)) {
		printf("successfully inserted %s\n", key);
	}
	else {
		printf("insert failed for %s!\n", key);
	}
	printf("try to insert same object\n");
	if (InsertIntoHashTable(tb, "bar", second)) {
		printf("successfully inserted %s\n", key);
	}
	else {
		printf("insert failed for %s!\n", key);
	}
	printf("try to insert multiple keys\n");
	Entry* arr[TEST_SIZE];
	char* str[TEST_SIZE];
	int count = 0;
	srand(10000);
	int i;
	for (i = 0; i < TEST_SIZE; i++) {
		str[i] = getRandomString();
		arr[i] = newEntry("",i);
		count+=InsertIntoHashTable(tb,str[i], arr[i]);
	}
	printf("Inserted %d items. Final load: %d/%d\n", count,tb->load,tb->size);
	printf("Try to find inserted keys \n");
	count = 0;
	for (i = 0; i < TEST_SIZE; i++) {
		if (FindInHashTable(tb, str[i]) == arr[i]) count++;
	}
	printf("Found %d items.\n",count);
	printf("try to delete multiple keys\n");
	count = 0;
	for (i = 0; i < TEST_SIZE; i+=2) {
		count += RemoveFromHashTable(tb, str[i]);
	}
	printf("Deleted %d items. Final load: %d/%d\n", count, tb->load, tb->size);
	printf("Try to find inserted keys \n");
	count = 0;
	for (i = 0; i < TEST_SIZE; i++) {
		if (FindInHashTable(tb, str[i]) == arr[i]) count++;
	}
	printf("Found %d items.\n", count);
	free(first);
	free(second);
	for (i = 0; i < TEST_SIZE; i++) {
		free(arr[i]);
	}
	DeleteHashTable(tb);
}


int main(int argc, char* argv[]) {
	if (argc < 2 || argc > 3) {
		printf("Usage: CodeBrowser [filters,] directory_name\n  e.g. CodeBrowser '*.c,*.h' /source/code/folder/\n       CodeBrowser 'string.h' /usr/include/\n");
		return 1;
	}

	//testInsert();
	allTokens = InitMap();
	allLinesFromFile = InitHashTable();
	List* filters;
	char* selected_directory;

	if (argc==2) {
		filters=NULL;
		selected_directory=argv[1];
	} else {
		filters = InitList();
		int i=0;
		char c;
		while (c = *(argv[1])) {
			if (c==',') {
				if (i>0) AppendToList(filters,newString(argv[1]-i,i));
				i=0;
			} else i++;
			argv[1]++;
		}
		if (i>0) AppendToList(filters,newString(argv[1]-i,i));
		selected_directory=argv[2];
	}

	directory_name = realpath(selected_directory,NULL);
	if (!directory_name) {
		printf("Error getting path for directory %s\n",selected_directory);
		return 0;
	}
	if (chdir(directory_name)!=0) {
		printf("Cannot change directory to %s!\n",directory_name);
		return 1;
	}
	printf("Reading from directory %s ...\n",directory_name);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);
	if (!parseFromDirectory(filters)) {
		printf("Cannot parse directory %s!\n",directory_name);
		return 1;
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&end);
	unsigned long diff_usec = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_nsec-start.tv_nsec)/1000;
	printf("Parsing completed (%.3fms)\n",(double)(diff_usec)/1000);
	printf("Inserted %d tokens (%d unique) from %d lines, %d files, %d directories.\n",countTokens,allTokens->size,countLines,allLinesFromFile->load,countDirectories);

	char bf[BUFFER_SIZE];
	printf(">>");
	while (fgets(bf,BUFFER_SIZE,stdin)) {
		if (bf[strlen(bf)-1] == '\n') bf[strlen(bf)-1] = '\0';
		findAllOccurences(bf);
		printf("\n>>");
	}
	unsigned int i;
	for (i=0; i<allLinesFromFile->size; i++) {
		void* ls = allLinesFromFile->entries[i];
		if (ls&&ls!=allLinesFromFile->dummy) DeleteList(ls);	
	}
	DeleteHashTable(allLinesFromFile);
	DeleteMap(allTokens);

	//printf("Using input path: %s\n", argv[1]);
	return 0;
}
