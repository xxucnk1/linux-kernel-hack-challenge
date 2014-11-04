.SUFFIXES:

CC = gcc -c  -std=c99 -fno-inline-functions-called-once -fno-guess-branch-probability -fdump-rtl-expand -g -gdwarf-2 -mno-sdata -mno-toc -G0 -fno-common -mcpu=8548 -mfloat-gprs=double -mspe -mabi=spe -Wall -O1 -DBIG_ENDIAN -D_BIG_ENDIAN -D_BREADBOARD_EXECUTION  -fno-builtin-memcpy -Wmissing-declarations -Wmissing-prototypes -MMD -o -pedantic 

LD = ld

OBJECTS  =  time_a60hz.o test_time_a60hz.o

INCLUDE_PATH = 

PROGRAM = test_time_a60hz.c.exe

all: ${PROGRAM}

${PROGRAM}: ${OBJECTS}
	$(LD) -o ${PROGRAM} --start-group ${OBJECTS}  -ltest_extensions -Llib -lprf -lrt --end-group

time_a60hz.o:
	$(CC)  -c time_a60hz.c -o $@

test_time_a60hz.o:
	$(CC)  -c test_time_a60hz.c ${INCLUDE_PATH} -o $@

time_types.o:
	$(CC)  -c time_types.c ${INCLUDE_PATH} -o $@

clean:
	rm -f *.o *.exe
