TARGET  := chkugenids
WARN    := -Wall
CFLAGS  := -O2 ${WARN}
LDFLAGS := -lc
CC      := gcc

C_SRCS    = $(wildcard *.c)
OBJ_FILES = $(C_SRCS:.c=.o)

all: ${TARGET}

%.o: %.c
	${CC} ${WARN} -c ${CFLAGS}  $< -o $@

${TARGET}: chkugenids.o
	${CC} ${WARN} -o $@ chkugenids.o ${LDFLAGS}

clean:
	rm -rf *.o ${TARGET}

mrproper: clean
	rm -rf *~
