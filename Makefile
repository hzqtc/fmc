SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

LIBS = -ljson -lcurl
CFLAGS = -Wall

all: fmc

debug: CFLAGS += -g
debug: fmc

release: CFLAGS += -O2
release: fmc

fmc: ${OBJ}
	gcc ${CFLAGS} -o $@ $^ ${LIBS}

%.o: %.c
	gcc ${CFLAGS} -c $<

clean:
	-rm *.o
