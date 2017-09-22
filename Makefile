libhttp-parser.so:	http_parser.o
	gcc -shared $^ -o libhttp-parser.so
http_parser.o:	http_parser.c
	gcc -fPIC -c http_parser.c -std=c11 -Wall -ansi -Wextra
