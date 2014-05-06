CodeBrowser
===========
grep-like tool for searching source code files (or text files in general) 
for keywords. Builds up a hashtable of keywords to lines-of-text mappings 
to support efficient querying. Search time is O(1) but memory consumption
is O(n).

	Usage: CodeBrowser [-i include] [-x exclude] [-f file] [-m results] [-tsrn] [directory_name]
	  optns:  -i include files that match pattern (* as wildcard)
	          -x exclude files that match pattern (* as wildcard)
		  -m maximum number of search results to print (default 100)
	          -t text mode (default, read text files)
	          -s symbol mode (read symbols from shared objects)
	          -f file mode (search for file names)
	          -r recursively read from directories
	          -n ignore comments
	          directory_name (current directory is the default)
 	 e.g. CodeBrowser -i '*.c,*.h' /source/code/folder/
	       CodeBrowser -x 'string.h' /usr/include/
	       CodeBrowser -sri 'lib*.so*' /lib
	       CodeBrowser -rf /home	
Features
--------
- color highlighting
- filter filenames/extensions to be included or excluded
- optionally read directories recursively
- symbol mode allows querying objects symbols instead of plain text
- ignore comments (experimental)

Example
-------

	jingdao:CodeBrowser$ ./CodeBrowser -i "*.h" /usr/include
	Reading from directory /usr/include ...
	Found 131161 tokens (15526 unique) in 30498 lines.
	Included: 122 Excluded: 0 Dirs: 1
	Memory estimate: 1066KB (data) 2877KB (pointers)
	Parsing completed (303.010ms)
	
	>>strxfrm
	langinfo.h:237 This information is accessed by the strcoll and strxfrm functions.
	string.h:150 extern size_t strxfrm (char *__restrict __dest,
	    Query complete (0.077ms) 2 occurences of 'strxfrm' found
	
	>>chdir
	fts.h:108 #define	FTS_DONTCHDIR	 0x01		/* don't chdir .. to the parent */
	unistd.h:497 extern int chdir (const char *__path) __THROW __nonnull ((1)) __wur;
	unistd.h:957 terminal.  If NOCHDIR is zero, do `chdir ("/")'.  If NOCLOSE is zero,
	    Query complete (0.063ms) 3 occurences of 'chdir' found
	
	>>NULL
	    Query complete. Too many results (159) to display.
