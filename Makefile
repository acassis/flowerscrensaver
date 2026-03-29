all: flower.c
	gcc flower.c -o flower -lGL -lGLU -lglut -lm

clean: flower
	rm flower
