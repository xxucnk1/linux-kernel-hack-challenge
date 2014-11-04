all: clean
	@echo "Compiling prf_lib.c"
	@/p/avionics-tools/eris/linux-tools/bin/powerpc-eabi-gcc -O02 -Wall -c libprf.c -o libprf.o -lrt 
	@echo "Creating static library libprf.a"
	@/p/avionics-tools/eris/linux-tools/bin/powerpc-eabi-ar -cvq libprf.a libprf.o > /dev/null
	@rm libprf.o

clean: 
	@echo "Cleaning prf_lib"
	@rm -rf libprf.o libprf.a libprf.so
