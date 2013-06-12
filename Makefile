cssradar: main.c
	gcc main.c -m32 -std=c99 -w -lcsfml-graphics -lcsfml-window -lcsfml-system -o cssradar 

clean:
	rm cssradar
