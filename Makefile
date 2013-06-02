BIN=icos

all:
	gcc -Wall -O3 -o $(BIN) $(BIN).c -lglut -lGLU

clean:
	rm -f $(BIN)
