/* Boids using ASCII graphics
   -original NCurses code from "Game Programming in C with the Ncurses Library"
   https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
   and from "NCURSES Programming HOWTO"
   http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
   -Boids algorithms from "Boids Pseudocode:
   http://www.kfish.org/boids/pseudocode.html
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


// include
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<pthread.h>
#include<time.h>

// graphics
#ifndef NOGRAPHICS
#include<unistd.h>
#include<ncurses.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


// delay time
#define DELAY 50000

// default population size, number of boids
#define POPSIZE 50

// maximum screen size, both height and width
#define SCREENSIZE 100

// default number of iterations to run before exiting, only used
// when graphics are turned off
#define ITERATIONS 1000

// default number of threads to run
#define THREADS 4

// boid location (x,y,z) and velocity (vx,vy,vz) in boidArray[][]
#define BX 0
#define BY 1
#define BZ 2
#define VX 3
#define VY 4
#define VZ 5

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


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

// the number of tasks
int threadsize;
// the number of boids per thread
double tasksize;
// the threads to be used
pthread_t* threadArray;
// array of splits
int** splitArray;

// timing
struct timespec startTime;
struct timespec endTime;
double elapsedTime;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


// intial boids
void initBoids() {
   
   // variables
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

// draw boids
#ifndef NOGRAPHICS
int drawBoids() {
   
   // variables
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

// rule 1
void *rule1(void* data) {
   
   // variables
   int i;
   float cx, cy, cz;

   // thread variables
   int min;
   int max;
   int* positions;


   cx = 0.0; cy = 0.0; cz = 0.0;

   // assign
   positions = (int*)data;
   min = positions[0];
   max = positions[1];
   

   // calculate centre of mass
   // calculated once and used for all updates in rule 1
   for(i=min; i<max; i++) {
      cx += boidArray[i][BX];
      cy += boidArray[i][BY];
      cz += boidArray[i][BZ];
   }
   cx /= popsize;
   cy /= popsize;
   cz /= popsize;

   // update velocity, move towards centre of mass
   // initial use of boidUpdate[][] so overwrite old values
   for(i=min; i<max; i++) {
      boidUpdate[i][BX] = (cx - boidArray[i][BX])/popsize;
      boidUpdate[i][BY] = (cy - boidArray[i][BY])/popsize;
      boidUpdate[i][BZ] = (cz - boidArray[i][BZ])/popsize;
   }

   return NULL;
}

// distance
float distance(int i, int j) {
   
   // calculate distance by squaring planar distances
   return(sqrtf(
      powf(boidArray[i][BX] - boidArray[j][BX],2.0) +
      powf(boidArray[i][BY] - boidArray[j][BY],2.0) +
      powf(boidArray[i][BZ] - boidArray[j][BZ],2.0) ));
}

// rule 2
void *rule2(void* data) {
   
   // variables
   int i, j;
   float cx, cy, cz;
   
   // thread variables
   int min;
   int max;
   int* positions;


   // assign
   positions = (int*)data;
   min = positions[0];
   max = positions[1];


   // keep boids from overlapping
   for(i=min; i<max; i++) {
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

   return NULL;
}

// rule 3
void *rule3(void* data) {
   
   // variables
   int i;
   float cx, cy, cz;

   int min;
   int max;
   int* positions;


   // assign
   positions = (int*)data;
   min = positions[0];
   max = positions[1];


   cx = 0.0; cy = 0.0; cz = 0.0;

   // calculate average velocity
   // calculate once and use for all updates in rule 3
   for(i=min; i<max; i++) {
      cx += boidArray[i][VX];
      cy += boidArray[i][VY];
      cz += boidArray[i][VZ];
   }
   cx /= popsize;
   cy /= popsize;
   cz /= popsize;

   // update velocity, move towards centre of mass
   for(i=min; i<max; i++) {
      boidUpdate[i][BX] += (cx - boidArray[i][VX])/8.0;
      boidUpdate[i][BY] += (cy - boidArray[i][VY])/8.0;
      boidUpdate[i][BZ] += (cz - boidArray[i][VZ])/8.0;
   }

   return NULL;
}


// move the flock towards a point
void *moveFlock(void* data) {
   
   // variables
   int i;
   static int count = 0;
   static int sign = 1;
   float px, py, pz;

   // thread variables
   int min;
   int max;
   int* positions;


   // assign
   positions = (int*)data;
   min = positions[0];
   max = positions[1];


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
   for(i=min; i<max; i++) {
      boidUpdate[i][BX] += (px - boidArray[i][BX])/200.0;
      boidUpdate[i][BY] += (py - boidArray[i][BY])/200.0;
      boidUpdate[i][BZ] += (pz - boidArray[i][BZ])/200.0;
   }
   count++;

   return NULL;
}

void *updateBoids(void* data) {

   // variables 
   int i;
   
   // thread variables
   int min;
   int max;
   int* positions;

   
   // assign
   positions = (int*)data;
   min = positions[0];
   max = positions[1];

   //printf("STARTING   %d %d\n", min, max);

   for (i = min; i < max; i++) {
      
      // update velocity for each boid
      boidArray[i][VX] += boidUpdate[i][BX];
      boidArray[i][VY] += boidUpdate[i][BY];
      boidArray[i][VZ] += boidUpdate[i][BZ];
      
      // update position for each boid
      boidArray[i][BX] += boidArray[i][VX];
      boidArray[i][BY] += boidArray[i][VY];
      boidArray[i][BZ] += boidArray[i][VZ];
   }

   //printf("COMPLETING %d %d\n", min, max);

   return NULL;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


// move boids
void moveBoids() {

   // variables
   int i;


   // all functions follow thesame pattern where each rule is multi threaded
   // but each rule has its own thread barrier.
   //
   // this is due to the nature of the some of the rule, rule 2 could potentially
   // interfere with the code the rule 1 if they were to run at the same time.
   // 
   // a thread barrier for each rule ensures that we dont run into any
   // overlap while keeping the system multi threaded.


   // rule 1
   for(i = 0; i < threadsize; i++)
      pthread_create(&threadArray[i], NULL, rule1, splitArray[i]);

   for(i = 0; i < threadsize; i++)
      pthread_join(threadArray[i], NULL);


   // rule 2
   for(i = 0; i < threadsize; i++)
      pthread_create(&threadArray[i], NULL, rule2, splitArray[i]);
   
   for(i = 0; i < threadsize; i++)
      pthread_join(threadArray[i], NULL);


   // rule 3
   for(i = 0; i < threadsize; i++)
      pthread_create(&threadArray[i], NULL, rule3, splitArray[i]);

   for(i = 0; i < threadsize; i++)
      pthread_join(threadArray[i], NULL);
   

   // move flock
   for(i = 0; i < threadsize; i++)
      pthread_create(&threadArray[i], NULL, moveFlock, splitArray[i]);

   for(i = 0; i < threadsize; i++)
      pthread_join(threadArray[i], NULL);


   // update boids
   for(i = 0; i < threadsize; i++)
      pthread_create(&threadArray[i], NULL, updateBoids, splitArray[i]);

   for(i = 0; i < threadsize; i++)
      pthread_join(threadArray[i], NULL);
      
}


// allocate arrays
void allocateArrays() {
   
   // variables
   int i;

   boidArray = malloc(sizeof(float *) * popsize);
   for(i=0; i<popsize; i++)
      boidArray[i] = malloc(sizeof(float) * 6);

   boidUpdate = malloc(sizeof(float *) * popsize);
   for(i=0; i<popsize; i++)
      boidUpdate[i] = malloc(sizeof(float) * 3);

}

// allocate threads
void allocateThreads() {

   // variabels
   int sum;
   int cur;


   // assign
   threadArray = malloc(sizeof(pthread_t) * threadsize);

   // malloc for the splits
   splitArray = malloc(sizeof(int*) * (threadsize));
   for(int i = 0; i < threadsize; i++)
      splitArray[i] = malloc(sizeof(int) * 2);
   

   // calculate the number of splits based on 
   tasksize = (double)popsize / (double)threadsize;

   // generate the split until the second last split was created
   sum = 0;
   cur = 0;
   splitArray[0][0] = 0;
   for(int i = 0; i < popsize && cur < threadsize - 1; i++){

      // increase the sum until its the same or larger than tasksize
      sum++;

      // catch a split position
      if((double)sum >= tasksize){

         // store position and iterate
         splitArray[cur++][1] = i;
         splitArray[cur][0] = i;

         // reset the sum
         sum = 0;
      }
   }

   // assign the last position to our array
   splitArray[threadsize - 1][1] = popsize -1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


// main function
int main(int argc, char *argv[]) {
   
   // variables
   int i;
   int count;
   int argPtr;


   // assign intial values
   // set the default population size
   popsize = POPSIZE;
   
   // set number of iterations, only used for timing tests in boidspt
   // not used in curses version
   count = ITERATIONS;

   // set the number of threads to use
   threadsize = THREADS;


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
         } else if (strcmp(argv[argPtr], "-t") == 0) {
            sscanf(argv[argPtr+1], "%d", &threadsize);
            argPtr += 2;
         } else {
            printf("USAGE: %s <-i iterations> <-c pop_size> <-t threads>\n", argv[0]);
            printf("\n");
            printf(" //\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\n");
            printf("\n");
            printf("   iterations -the number of times the population will be updated\n");
            printf("\n");
            printf("   pop_size -the number of boids to create\n");
            printf("   the number of iterations only affects the non-curses program boidspt\n");
            printf("   the curses program exits when q is pressed\n");
            printf("\n");
            printf("   threads -the number of threads to use for the application\n");
            printf("   make sure that you a valid number of threads. \n");
            printf("\n");
            printf(" //\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\\\\//\n\n");
            exit(1);
         }
      }
   }


   // allocate space for theads and set up slitting
   allocateThreads();

   // allocate space for arrays to store boid position and velocity
   allocateArrays();


   // intialize graphics 
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
      if (drawBoids() == 1) break; // run until the user hits q
      moveBoids();
   }
#endif

   // calculate movement of boids but do not use ncurses to draw
#ifdef NOGRAPHICS

   // print results
   printf("Number of boids per thread %lf\n", tasksize);
   printf("Thread Data Ranges:\n");
   for(int i = 0; i < threadsize; i++)
      printf("\tthread 1: [%d][%d] \t~ %d\n", 
         splitArray[i][0], 
         splitArray[i][1],
         splitArray[i][0] - splitArray[i][1]);
   
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
