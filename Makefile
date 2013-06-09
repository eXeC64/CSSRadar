SRC_FILES=src/main.c src/hook.c
cssradar: $(SRC_FILES)
	gcc $(SRC_FILES) -m32 -std=c99 -lcsfml-graphics -lcsfml-window -lcsfml-system -o cssradar 

clean:
	rm cssradar
