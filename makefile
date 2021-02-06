LEX=flex
YACC=bison
CC=gcc
CP=g++

# set PATH=C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;F:\MinGW\mingw32\bin;c:\GnuWin32\bin;
#	$(CP) -std=c++11 y.tab.cpp cJSON.c -g -ly -lfl -o scc

scc:y.tab.cpp cJSON.c lex.yy.cpp

lex.yy.o:lex.yy.cpp y.tab.hpp
	$(CC) -std=c++11 -c lex.yy.cpp
	
y.tab.o:y.tab.cpp def.h ast.h semantics.h translate.hpp interprete.h
	$(CC) -std=c++11 -c y.tab.cpp
	
cJSON.o:cJSON.c cJSON.h
	$(CC) -std=c++11 -c cJSON.c
	
y.tab.cpp y.tab.hpp:smallc.y header.h def.h ast.h semantics.h translate.hpp interprete.h
	$(YACC) -oy.tab.cpp smallc.y -v -d

lex.yy.cpp:smallc.l header.h def.h ast.h semantics.h translate.hpp interprete.h
	$(LEX) -olex.yy.cpp smallc.l

clean:
	rm -f *.o
