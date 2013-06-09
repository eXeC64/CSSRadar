SRC_FILES=src/main.c src/hook.c
superstrike64: $(SRC_FILES)
	gcc $(SRC_FILES) -m32 -std=c99 -lcsfml-graphics -lcsfml-window -lcsfml-system -o superstrike64 

clean:
	rm superstrike64
