DEPS = encoder.h
CFLAGS = -Wall -c
DFLAGS = 

.PHONY: all debug sanitize clean
all: streamer visor

streamer: streamer.o encoder.o
	gcc -o $@ streamer.o encoder.o $(DFLAGS) -lpthread

visor: visor.o
	gcc -o $@ visor.o $(DFLAGS) -lpthread

encoder.o: encoder.c $(DEPS)
	gcc $(CFLAGS) encoder.c $(DFLAGS)

streamer.o: streamer.c $(DEPS)
	gcc $(CFLAGS) streamer.c $(DFLAGS)

visor.o: visor.c
	gcc $(CFLAGS) visor.c $(DFLAGS)

debug: DFLAGS = -g
debug: clean all

sanitize: DFLAGS = -fsanitize=address,undefined
sanitize: clean all

clean:
	rm -rf streamer visor *.o
