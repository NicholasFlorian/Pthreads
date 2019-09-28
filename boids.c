/* Boids using ASCII graphics
	-original NCurses code from "Game Programming in C with the Ncurses Library"
	 https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
	 and from "NCURSES Programming HOWTO"
	 http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
	-Boids algorithms from "Boids Pseudocode:
	 http://www.kfish.org/boids/pseudocode.html
*/ 


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#ifndef NOGRAPHICS
#include<unistd.h>
#include<ncurses.h>
#endif

#define DELAY 50000

	// default population size, number of boids
#define POPSIZE 50
	// maximum screen size, both height and width
#define SCREENSIZE 100
	// default number of iterations to run before exiting, only used
	// when graphics are turned off
#define ITERATIONS 1000

	// boid location (x,y,z) and velocity (vx,vy,vz) in boidArray[][]
#define BX 0
#define BY 1
#define BZ 2
#define VX 3
#define VY 4
#define VZ 5

#ifndef NOGRAPHICS
	// maximum screen dimensions
int max_y = 0, max_x = 0;
#endif

	// user defined number of boids to create in the population
int popsize;

	// location and velocity of boids
float **boidArray;
	// change in velocity is stored for each boid (x,y,z)
float **boidUpdate;

// timing
struct timespec startTime;
struct timespec endTime;
double elapsedTime;


void initBoids() {
int i;
	// calculate initial random locations for each boid, scaled based on the screen size
   for(i=0; i<popsize; i++) {
      boidArray[i][BX] = (float) (random() % SCREENSIZE); 
      boidArray[i][BY] = (float) (random() % SCREENSIZE); 
      boidArray[i][BZ] = (float) (random() % SCREENSIZE); 
      boidArray[i][VX] = 0.0; 
      boidArray[i][VY] = 0.0; 
      boidArray[i][VZ] = 0.0; 
   }
}

#ifndef NOGRAPHICS
int drawBoids() {
int c, i;
float multx, multy;

	// update screen maximum size
   getmaxyx(stdscr, max_y, max_x);

	// used to scale position of boids based on screen size
   multx = (float)max_x / SCREENSIZE;
   multy = (float)max_y / SCREENSIZE;

   clear();

	// display boids
   for (i=0; i<popsize; i++) {
      mvprintw((int)(boidArray[i][BX]*multy), (int)(boidArray[i][BY]*multx), "o");
   }

   refresh();

   usleep(DELAY);

	// read keyboard and exit if 'q' pressed
   c = getch();
   if (c == 'q') return(1);
   else return(0);
}
#endif

void rule1() {
int i;
float cx, cy, cz;

   cx = 0.0; cy = 0.0; cz = 0.0;

	// calculate centre of mass
	// calculated once and used for all updates in rule 1
   for(i=0; i<popsize; i++) {
      cx += boidArray[i][BX];
      cy += boidArray[i][BY];
      cz += boidArray[i][BZ];
   }
   cx /= popsize;
   cy /= popsize;
   cz /= popsize;

	// update velocity, move towards centre of mass
	// initial use of boidUpdate[][] so overwrite old values
   for(i=0; i<popsize; i++) {
      boidUpdate[i][BX] = (cx - boidArray[i][BX])/popsize;
      boidUpdate[i][BY] = (cy - boidArray[i][BY])/popsize;
      boidUpdate[i][BZ] = (cz - boidArray[i][BZ])/popsize;
   }

}

float distance(int i, int j) {
   return(sqrtf(
      powf(boidArray[i][BX] - boidArray[j][BX],2.0) + 
      powf(boidArray[i][BY] - boidArray[j][BY],2.0) + 
      powf(boidArray[i][BZ] - boidArray[j][BZ],2.0) ));
}


void rule2() {
int i, j;
float cx, cy, cz;

			// keep boids from overlapping
   for(i=0; i<popsize; i++) {
      cx = 0.0; cy = 0.0; cz = 0.0;
      for(j=0; j<popsize; j++) {
         if (i != j) {		// calculate when not the same boid
            if (distance(i,j) < 5.0) {
               cx = cx - (boidArray[j][BX] - boidArray[i][BX]);
               cy = cy - (boidArray[j][BY] - boidArray[i][BY]);
               cz = cz - (boidArray[j][BZ] - boidArray[i][BZ]);
            }
         }
      }
      boidUpdate[i][BX] += cx;
      boidUpdate[i][BY] += cy;
      boidUpdate[i][BZ] += cz;
   }
}

void rule3() {
int i;
float cx, cy, cz;

   cx = 0.0; cy = 0.0; cz = 0.0;

	// calculate average velocity
	// calculate once and use for all updates in rule 3
   for(i=0; i<popsize; i++) {
      cx += boidArray[i][VX];
      cy += boidArray[i][VY];
      cz += boidArray[i][VZ];
   }
   cx /= popsize;
   cy /= popsize;
   cz /= popsize;

	// update velocity, move towards centre of mass
   for(i=0; i<popsize; i++) {
      boidUpdate[i][BX] += (cx - boidArray[i][VX])/8.0;
      boidUpdate[i][BY] += (cy - boidArray[i][VY])/8.0;
      boidUpdate[i][BZ] += (cz - boidArray[i][VZ])/8.0;
   }

}

	// move the flock towards a point
void moveFlock() {
int i;
static int count = 0;
static int sign = 1;
float px, py, pz;


	// pull flock towards two points as the program runs
	// every 200 iterations change point that flock is pulled towards
   if (count % 200 == 0) {
      sign = sign * -1;
   } 
   if (sign == 1) {
	// move flock towards position (40,40,40)
      px = 40.0;
      py = 40.0;
      pz = 40.0;
   } else {
	// move flock towards position (60,60,60)
      px = 60.0;
      py = 60.0;
      pz = 60.0;
   }
	// add offset (px,py,pz) to each boid in order to pull it
	// towards the current target point
   for(i=0; i<popsize; i++) {
      boidUpdate[i][BX] += (px - boidArray[i][BX])/200.0;
      boidUpdate[i][BY] += (py - boidArray[i][BY])/200.0;
      boidUpdate[i][BZ] += (pz - boidArray[i][BZ])/200.0;
   }
   count++;

}

void moveBoids() {
int i;

   rule1();
   rule2();
   rule3();
   moveFlock();

	// move boids by calculating updated velocity and new position
   for (i=0; i<popsize; i++) {
	// update velocity for each boid
      boidArray[i][VX] += boidUpdate[i][BX];
      boidArray[i][VY] += boidUpdate[i][BY];
      boidArray[i][VZ] += boidUpdate[i][BZ];
	// update position for each boid
      boidArray[i][BX] += boidArray[i][VX];
      boidArray[i][BY] += boidArray[i][VY];
      boidArray[i][BZ] += boidArray[i][VZ];
   }
}

void allocateArrays() {
int i;

   boidArray = malloc(sizeof(float *) * popsize);
   for(i=0; i<popsize; i++)
      boidArray[i] = malloc(sizeof(float) * 6);

   boidUpdate = malloc(sizeof(float *) * popsize);
   for(i=0; i<popsize; i++)
      boidUpdate[i] = malloc(sizeof(float) * 3);

}

int main(int argc, char *argv[]) {
int i, count;
int argPtr;

	// set the default population size
   popsize = POPSIZE;
	// set number of iterations, only used for timing tests in boidspt
	// not used in curses version
   count = ITERATIONS;

	// read command line arguments for number of iterations and 
	// number of boids
   if (argc > 1) {
      argPtr = 1;
      while(argPtr < argc) {
         if (strcmp(argv[argPtr], "-i") == 0) {
            sscanf(argv[argPtr+1], "%d", &count);
            argPtr += 2;
         } else if (strcmp(argv[argPtr], "-c") == 0) {
            sscanf(argv[argPtr+1], "%d", &popsize);
            argPtr += 2;
         } else {
            printf("USAGE: %s <-i iterations> <-c pop_size>\n", argv[0]);
            printf(" iterations -the number of times the population will be updated\n");
            printf(" pop_size -the number of boids to create\n");
	    printf(" the number of iterations only affects the non-curses program boidspt\n");
	    printf(" the curses program exits when q is pressed\n");
            exit(1);
         }
      }
   }

	// allocate space for arrays to store boid position and velocity
   allocateArrays();


#ifndef NOGRAPHICS
	// initialize ncurses
   initscr();
   noecho();
   cbreak();
   timeout(0);
   curs_set(FALSE);
     // Global var `stdscr` is created by the call to `initscr()`
   getmaxyx(stdscr, max_y, max_x);
#endif

	// place boids in initial positions
   initBoids();

	// draw and move boids using ncurses 
	// do not calculate timing in this loop, ncurses will reduce performance
#ifndef NOGRAPHICS
   while(1) {
      if (drawBoids() == 1) break;
      moveBoids();
   }
#endif

	// calculate movement of boids but do not use ncurses to draw
#ifdef NOGRAPHICS
   printf("Number of iterations %d\n", count);
   printf("Number of boids %d\n", popsize);

   /*** Start timing here ***/
   clock_gettime(CLOCK_MONOTONIC, &startTime);

   for(i=0; i<count; i++) {
      moveBoids();
   }
   /*** End timing here ***/
   clock_gettime(CLOCK_MONOTONIC, &endTime);


   elapsedTime = (endTime.tv_sec - startTime.tv_sec);
   elapsedTime += (endTime.tv_nsec - startTime.tv_nsec) / 1000000000.0;
   
   printf("Time elapsed %lf\n", elapsedTime);

#endif

#ifndef NOGRAPHICS
	// shut down ncurses
   endwin();
#endif

}
