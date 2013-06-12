SRC_FILES=src/main.c
cssradar: $(SRC_FILES)
	gcc $(SRC_FILES) -m32 -std=c99 -w -lcsfml-graphics -lcsfml-window -lcsfml-system -o cssradar 

clean:
	rm cssradar
