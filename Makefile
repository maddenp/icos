BIN=icos

all:
	gcc -Wall -O3 -o $(BIN) $(BIN).c -lglut -lGL -lGLU -lm

clean:
	$(RM) $(BIN)
