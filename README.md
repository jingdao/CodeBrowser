CodeBrowser
===========
grep-like tool for searching source code files (or text files in general) 
for keywords. Builds up a hashtable of keywords to lines-of-text mappings 
to support efficient querying. Search time is O(1) but memory consumption
is O(n).

	Usage: CodeBrowser [filters,] directory_name
  		e.g. CodeBrowser '*.c,*.h' /source/code/folder/
       		     CodeBrowser 'string.h' /usr/include/

Features
--------
- color highlighting
- filter for certain filenames/extensions

Example
-------

	jingdao@jingdao-K45VD:~/cProjects/CodeBrowser$ ./CodeBrowser "*.h" /usr/include
	Reading from directory /usr/include ...
	Inserted 1264488 tokens from 378174 lines in 1629 files in 120 directories. Unique tokens: 115078
	>>strxfrm
    		3 occurences of 'strxfrm' found
		langinfo.h:237 This information is accessed by the strcoll and strxfrm functions.
		c++/4.8/bits/locale_classes.h:649 *  This function is a wrapper for strxfrm functionality.  It takes the
		string.h:150 extern size_t strxfrm (char *__restrict __dest,

	>>chdir
    		4 occurences of 'chdir' found
		fts.h:108 #define	FTS_DONTCHDIR	 0x01		/* don't chdir .. to the parent */
		X11/Xw32defs.h:15 #  define chdir	_chdir
		unistd.h:497 extern int chdir (const char *__path) __THROW __nonnull ((1)) __wur;
		unistd.h:957 terminal.  If NOCHDIR is zero, do `chdir ("/")'.  If NOCLOSE is zero,

	>>NULL
    		752 occurences of 'NULL' found
    		Too many results to display.

