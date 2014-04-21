CC = gcc
CFLAGS = -std=c99 
SOURCES = Map.c List.c HashTable.c
ALL_SOURCES = CodeBrowser.c $(SOURCES)
HEADERS = Map.h List.h HashTable.h
SHARED_OBJECT = libMap.so
SHARED_OBJECT_FLAGS = -shared -fPIC
OBJ = Map.o List.o HashTable.o
ALL_OBJ = CodeBrowser.o $(OBJ)
EXECUTABLE = CodeBrowser

all: $(ALL_SOURCES) $(EXECUTABLE)

shared: $(SOURCES) $(HEADERS) 
	$(CC) $(CFLAGS) -o $(SHARED_OBJECT) $(SOURCES) $(SHARED_OBJECT_FLAGS)

$(EXECUTABLE): $(ALL_OBJ)
	$(CC) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(EXECUTABLE) $(SHARED_OBJECT)
