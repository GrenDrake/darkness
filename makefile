CFLAGS=--std=c99 -Wall -pedantic -g
OBJS=darkness.o delve.o  actor.o  map.o map_generate.o  log.o random.o
TARGET=darkness

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -lncurses -o $(TARGET)

clean:
	$(RM) *.o $(TARGET)

.PHONY: all clean
