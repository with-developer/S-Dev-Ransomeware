all:ransomeware

ransomeware:main.c
	gcc -o ransomeware.exe main.c

clean:
	del ransomeware.exe