
all: boids boidspt data test

boids: boids.c
	gcc boids.c -o boids -lncurses -lm 

boidspt: boids.c
	gcc boids.c -o boidspt -lm -DNOGRAPHICS


# project makes
data: data.c
	gcc data.c -o data -pthread -lncurses -lm -DNOGRAPHICS 

test: test.c
	gcc test.c -o test -pthread -lncurses -lm -DNOGRAPHICS 

clean: 
	rm boids boidspt data
