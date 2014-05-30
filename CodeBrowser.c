#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include "magic.h"
#define TEST_SIZE 100000
#define BUFFER_SIZE 256
#define MAX_PRINT_LINES 100
#define FILE_NAME_COLOR "\x1B[94m"
#define LINE_NUMBER_COLOR "\x1B[96m"
#define NORMAL_COLOR "\x1B[0m"
#define HIGHLIGHT_COLOR "\x1B[91m"
#define SUMMARY_COLOR "\x1B[92m"
#define TIMING_MODE CLOCK_MONOTONIC
#include "Map.h"

typedef struct {
	char* path;
	int lineNumber;
} Entry;
enum parseMode {
	TEXT_MODE, SYMBOL_MODE, FILE_MODE
};
Map* allTokens = NULL;
HashTable* allLinesFromFile = NULL; //faster than Map for consecutive inserts on the same List
char* directory_name = NULL;
int countTokens = 0;
int countDirectories = 0;
int countLines = 0;
int countLibraries = 0;
int countExcluded = 0;
//unsigned int memData = 0;
//unsigned int memPointers = 0;
int currentMode = TEXT_MODE;
int recursiveMode = 0;
int ignoreComment = 0;
int maxPrintLines = MAX_PRINT_LINES;
struct timespec start;
struct timespec end;
magic_t cookie;
char* mime_type;

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

char* getMemoryRepr(char* buffer, unsigned int i) {
	if (i>=10000000) {
		sprintf(buffer,"%4uMB",i/1000000);
	} else if (i>=10000) {
		sprintf(buffer,"%4uKB",i/1000);
	} else {
		sprintf(buffer,"%4uB",i);
	}
	return buffer;
}

char* newString(char* c, unsigned int length) {
	char* res = malloc((length+1)*sizeof(char));
	//printf("malloc char: %d\n",(length+1)*sizeof(char));
	//memData+=(length+1)*sizeof(char);
	unsigned int i;
	memcpy(res,c,length);
	res[length]='\0';
	return res;
}

int matchString(char* filter, char* text) {
	if (*filter||*text) {
		if (*filter=='*'&&*(filter+1)!='\0'&&*text=='\0') return 0;
		if (*filter==*text) return matchString(filter+1,text+1);
		if (*filter=='*') return matchString(filter+1,text) || matchString(filter,text+1);
		return 0;
	} else return 1;
}

int filterLine(char* buffer) {
	/*while(*line) {
		if (*line==' ') line++;
		if (*line=='#') return 1;
		else return 0;
	} 
	return 0;*/
	/*int i;
	int startText=0;
	int endText=0;
	int whitespace=1;
	for (i=0;i<BUFFER_SIZE;i++) {
		if (!*buffer) break;
		if (whitespace&&!(*buffer==' '||*buffer=='\t'||*buffer=='\n')) whitespace=0;
		if (whitespace) startText++;
		buffer++;
	}
	if (whitespace) return -1;
	else return startText;*/
	return 0;
}

void findAllNames(char* key) {
	clock_gettime(TIMING_MODE,&start);
	List* ls = GetListFromMap(allTokens,key);
	if (ls) {
		if (ls->size>maxPrintLines) {
			printf("    %sQuery complete. Too many results (%d) to display.%s",SUMMARY_COLOR,ls->size,NORMAL_COLOR);
			return;
		}
		unsigned int i;
		for (i=0;i<ls->size;i++) {
			char* fi = GetListItem(ls,i); 
			if (!fi) printf("Warning: null pointer or index out of bounds!\n");
			else {
				printf("%s%s: %s%s%s\n",FILE_NAME_COLOR,fi,HIGHLIGHT_COLOR,key,NORMAL_COLOR);
			}
		}
		clock_gettime(TIMING_MODE,&end);
		unsigned int diff_usec = (end.tv_nsec-start.tv_nsec)/1000;
		printf("    %sQuery complete (%.3fms) %d occurences of '%s' found%s",
			SUMMARY_COLOR,(float)(diff_usec)/1000,ls->size,key,NORMAL_COLOR);
	} else {
		printf("    no occurence of '%s' found\n",key);
	}
}

void findSymbolLocations(char* key) {
	clock_gettime(TIMING_MODE,&start);
	List* ls = GetListFromMap(allTokens,key);
	if (ls) {
		if (ls->size>maxPrintLines) {
			printf("    %sQuery complete. Too many results (%d) to display.%s",SUMMARY_COLOR,ls->size,NORMAL_COLOR);
			return;
		}
		unsigned int i;
		for (i=0;i<ls->size;i++) {
			char* fi = GetListItem(ls,i); 
			if (!fi) printf("Warning: null pointer or index out of bounds!\n");
			else {
				printf("%s%s: %s%s%s\n",FILE_NAME_COLOR,fi,HIGHLIGHT_COLOR,key,NORMAL_COLOR);
			}
		}
		clock_gettime(TIMING_MODE,&end);
		unsigned int diff_usec = (end.tv_nsec-start.tv_nsec)/1000;
		printf("    %sQuery complete (%.3fms) %d occurences of '%s' found%s",
			SUMMARY_COLOR,(float)(diff_usec)/1000,ls->size,key,NORMAL_COLOR);
	} else {
		printf("    no occurence of '%s' found\n",key);
	}
}

void findAllOccurences(char* key) {
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);
	List* ls = GetListFromMap(allTokens,key);
	if (ls) {
		if (ls->size>maxPrintLines) {
			printf("    %sQuery complete. Too many results (%d) to display.%s",SUMMARY_COLOR,ls->size,NORMAL_COLOR);
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
		printf("    %sQuery complete (%.3fms) %d occurences of '%s' found%s",
			SUMMARY_COLOR,(float)(diff_usec)/1000,ls->size,key,NORMAL_COLOR);
	} else {
		printf("    no occurence of '%s' found\n",key);
	}
}

int parseNamesFromFile(char* fileName) {
	char* fullpath = realpath(fileName,NULL);
	if (!fullpath) {
		printf("Error getting path for file %s\n",fileName);
		return 0;
	}
	countTokens+=AddToMap(allTokens,fileName,fullpath);
}

int parseSymbolsFromFile(char* fileName) {
	FILE *f;
	char buffer[BUFFER_SIZE];
	char* token;
	char* fullpath;
	int validLibrary = 0;
	
	if (strncmp("application",magic_file(cookie,fileName),11)!=0) return 0;
	fullpath = realpath(fileName,NULL);
	if (!fullpath) {
		printf("Error getting path for file %s\n",fileName);
		return 0;
	}
	char* filepath = newString(fullpath+strlen(directory_name)+1,strlen(fullpath)-strlen(directory_name)-1);
	free(fullpath);
	sprintf(buffer,"nm -D --defined-only -f posix %s 2>/dev/null",fileName);
	f = popen(buffer,"r");
	if (f) {
		//printf("Reading file %s ......\n",fileName);
		while (fgets(buffer,BUFFER_SIZE,f)) {
			token = strchr(buffer,' ');
			if (token&&(*(token+1)=='T'||*(token+1)=='W'||*(token+1)=='B'||*(token+1)=='D')) {
				validLibrary=1;
				*token = '\0';
				//printf("%s\n",buffer);
				countTokens+=AddToMap(allTokens,buffer,filepath);
			}
		}
		pclose(f);
		countLibraries+=validLibrary;
		return 1;
	} else {
		printf("Error opening file %s\n",fileName);
		return 0;
	}
	
}

int parseFromFile(char* fileName) {
	FILE *pFile;
	char lineBuffer[BUFFER_SIZE];
	char token[BUFFER_SIZE];
	char* fullpath = NULL;

	if (strncmp("text",magic_file(cookie,fileName),4)!=0) return 0;
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
		while(fgets(lineBuffer , BUFFER_SIZE , pFile)) {
			//printf("Line %d: %s",lineNumber, string);
			int offset = filterLine(lineBuffer);
			if (offset>=0) {
				char* string=&(lineBuffer[offset]);
				Entry* thisLine = newEntry(filepath,lineNumber);
				unsigned int i;
				for (i=0;i<BUFFER_SIZE;i++) {
					if (!string[i]) break;
					else if (string[i]=='\n') {
						if (tokenIndex>0) {
							token[tokenIndex]='\0';
							tokenIndex=0;
							countTokens+=AddToMap(allTokens,token,thisLine);
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
			}
			lineNumber++;
		}
		fclose(pFile);
		InsertIntoHashTable(allLinesFromFile,filepath,linesFromFile);
		countLines+=lineNumber;
	}

	return 1;
} 

int parseFromDirectory(List* inc_filters, List* exc_filters) {
	struct dirent *current;
	DIR *d;
	d = opendir(".");
	if (!d) {
		printf("Cannot open current directory\n");
		return 0;
	}
	while(current = readdir(d)) {
		char* fn = current->d_name;
		if (strcmp(".",fn)==0||strcmp("..",fn)==0) continue;
		if (recursiveMode&&current->d_type==DT_DIR) {
			if (chdir(current->d_name)!=0) {
				printf("Cannot change directory to %s!\n",current->d_name);
			} else {
				//printf("Reading from directory %s ...\n",current->d_name);
				parseFromDirectory(inc_filters,exc_filters);
				if (chdir("..")) printf("Cannot go up from %s!\n",current->d_name); 
			}
			continue;
		} else if (current->d_type==DT_REG) {
			unsigned int i;
			if (exc_filters) {
				for (i=0;i<exc_filters->size;i++) {
					if(matchString(GetListItem(exc_filters,i),fn)) {
						countExcluded++; break;
					}
				}
			} 
			if (!exc_filters||i==exc_filters->size) {
				if (inc_filters) {
					for (i=0;i<inc_filters->size;i++) {
						if(matchString(GetListItem(inc_filters,i),fn)) {
							if (currentMode==SYMBOL_MODE) parseSymbolsFromFile(fn);
							else if (currentMode==FILE_MODE) parseNamesFromFile(fn);
							else parseFromFile(fn);
							break;
						}
					}
				} else {
					if (currentMode==SYMBOL_MODE) parseSymbolsFromFile(fn);
					else if (currentMode==FILE_MODE) parseNamesFromFile(fn);
					else parseFromFile(fn);
				}
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

void testMatchString() {
	printf("%d\n",matchString("*a*","abcde"));
	printf("%d\n",matchString("*e*","abcde"));
	printf("%d\n",matchString("**","abcde"));
	printf("%d\n",matchString("*","abcde"));
	printf("%d\n",matchString("a*c*e","abcde"));
	printf("%d\n",matchString("a*c*d","abcde"));
	printf("%d\n",matchString("libSDL*.so.*","libSDL-1.2.so.0.11.3"));
}

void printUsage() {
	printf("Usage: CodeBrowser [-i include] [-x exclude] [-m results] [-tsfrnm] [directory_name]\n"
				"  optns:  -i include files that match pattern (* as wildcard)\n"
				"          -x exclude files that match pattern (* as wildcard)\n"
				"          -m maximum number of search results to print (default 100)\n"
				"          -t text mode (default, read text files)\n"
				"          -s symbol mode (read symbols from shared objects)\n"
				"          -f file mode (search for file names)\n"
				"          -r recursively read from directories\n"
				"          -n ignore C-style comments '/*,*/,//'\n"
				"          -m ignore script-style comments '#'\n"
				"          directory_name (current directory is the default)\n" 
				"  e.g. CodeBrowser -i '*.c,*.h' /source/code/folder/\n"
				"       CodeBrowser -x 'string.h' /usr/include/\n"
				"       CodeBrowser -sri 'lib*.so*' /lib\n");
}

int main(int argc, char* argv[]) {
	int s;
	char *included = NULL;
	char *excluded = NULL;
	char* maxLines = NULL;
	List* inc_filters = NULL;
	List* exc_filters = NULL;
	char* selected_directory;
	char bf[BUFFER_SIZE];
	while ((s=getopt(argc,argv,"i:x:m:tsrnf"))!=-1) {
		switch (s) {
			case 't': currentMode = TEXT_MODE; break;
			case 's': currentMode = SYMBOL_MODE; break;
			case 'f': currentMode = FILE_MODE; break;
			case 'r': recursiveMode = 1; break;
			case 'n': ignoreComment = 1; break;
			case 'i': included = optarg; break;
			case 'x': excluded = optarg; break;
			case 'm': maxLines = optarg; break;
			case '?': printUsage(); return 1;
			default: printUsage(); return 1;
		}
	}
	if (optind<argc) selected_directory = argv[optind];
	else selected_directory = ".";
	
	//testMatchString();
	//testInsert();
	cookie = magic_open(MAGIC_MIME_TYPE);
	if (!cookie||magic_load(cookie,NULL)) {
		printf("Error loading magic number database\n");
		return 1;
	}
	allTokens= InitMap();
	if (currentMode==TEXT_MODE)
		allLinesFromFile = InitHashTable();

	if (included) {
		inc_filters = InitList();
		int i=0;
		char c;
		while (c = *included) {
			if (c==',') {
				if (i>0) AppendToList(inc_filters,newString(included-i,i));
				i=0;
			} else i++;
			included++;
		}
		if (i>0) AppendToList(inc_filters,newString(included-i,i));
	}
	if (excluded) {
		exc_filters = InitList();
		int i=0;
		char c;
		while (c = *excluded) {
			if (c==',') {
				if (i>0) AppendToList(exc_filters,newString(excluded-i,i));
				i=0;
			} else i++;
			excluded++;
		}
		if (i>0) AppendToList(exc_filters,newString(excluded-i,i));
	}
	if (maxLines) {
		maxPrintLines = atoi(maxLines);
		if (maxPrintLines<=0) maxPrintLines=MAX_PRINT_LINES;
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
	clock_gettime(TIMING_MODE,&start);
	if (!parseFromDirectory(inc_filters,exc_filters)) {
		printf("Cannot parse directory %s!\n",directory_name);
		return 1;
	}
	clock_gettime(TIMING_MODE,&end);
	unsigned long diff_usec = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_nsec-start.tv_nsec)/1000;
	if (currentMode==SYMBOL_MODE) {
		printf("Found %d symbols (%d unique).\n",countTokens,allTokens->size);
		printf("Inc. files: %d Exc. files: %d Dirs: %d\n",countLibraries,countExcluded,countDirectories);
		//memPointers+=sizeof(Map)+sizeof(HashTable)+sizeof(List)*allTokens->size+sizeof(void*)*allTokens->tb->size;
	} else if (currentMode==FILE_MODE) {
		printf("Found %d filenames (%d unique).\n",countTokens,allTokens->size);
		printf("Inc. files: %d Exc. files: %d Dirs: %d\n",countTokens,countExcluded,countDirectories);
		//memPointers+=sizeof(Map)+sizeof(List)*allTokens->size+sizeof(void*)*allTokens->tb->size;
	} else {
		printf("Found %d tokens (%d unique) in %d lines.\n",countTokens,allTokens->size,countLines);
		printf("Included: %d Excluded: %d Dirs: %d\n",allLinesFromFile->load,countExcluded,countDirectories);
		//memPointers+=sizeof(Map)+sizeof(HashTable)*2+sizeof(List)*(allTokens->size+allLinesFromFile->load)+sizeof(Entry)*countTokens+sizeof(void*)*(allTokens->tb->size+allLinesFromFile->size);
	}
	//printf("Memory estimate: %s (data) ",getMemoryRepr(bf,memData));
	//printf("%s (pointers)\n",getMemoryRepr(bf,memPointers));
	FILE *f = fopen("/proc/self/statm","r");
	unsigned int vmsize,vmrss;
	if(f){
		if (fscanf(f,"%u %u",&vmsize,&vmrss)==2) {
			printf("VmSize: %s ",getMemoryRepr(bf,vmsize*4000));
			printf("VmRSS: %s\n",getMemoryRepr(bf,vmrss*4000));
		}
		fclose(f);
	}
	printf("Parsing completed (%.3fms)\n",(double)(diff_usec)/1000);

	//writeStringListToHTL("test.htl",FindInHashTable(allLinesFromFile,"Map.h"));
	//readFromHTL("test.htl");
	printf(">>");
	while (fgets(bf,BUFFER_SIZE,stdin)) {
		if (strlen(bf)<=1) break;
		if (bf[strlen(bf)-1] == '\n') bf[strlen(bf)-1] = '\0';
		if (currentMode==SYMBOL_MODE) findSymbolLocations(bf);
		else if (currentMode==FILE_MODE) findAllNames(bf);
		else findAllOccurences(bf);
		printf("\n>>");
	}
	unsigned int i;
	for (i=0; i<allLinesFromFile->size; i++) {
		void* ls = allLinesFromFile->entries[i];

		if (ls&&ls!=allLinesFromFile->dummy) DeleteList(ls);	
	}
	DeleteHashTable(allLinesFromFile);
	DeleteMap(allTokens);

	return 0;
}
