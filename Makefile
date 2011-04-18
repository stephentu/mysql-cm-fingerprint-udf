all:
	gcc -Wall -O2 -I/usr/include/mysql -c cmfp.c -o cmfp.o
	ld -shared -o libcmfp.so cmfp.o 
clean:
	rm -f *.so *.o
	
