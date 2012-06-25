BIN=icos

all:
	gcc -Wall -O3 -o $(BIN) -lglut -lGLU $(BIN).c

clean:
	rm -f $(BIN)
