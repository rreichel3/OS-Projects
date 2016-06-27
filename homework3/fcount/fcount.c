#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Print out an error message and exit.
static void fail( char const *message ) {
    fprintf( stderr, "%s\n", message );
    exit( 1 );
}

// Maximum number of values we can store.  The list of values is allocated statically,
// so workers can start working on values as soon as we read them in.
#define INIT_SIZE 10

// List of all values read from standard input.
long *vList;

// Number of values in vList.
int vCount = 0;

int vSize = INIT_SIZE;

// Target number of factors to look for.
int fTarget;

// Total number of values we've found that have the target number of factors.
int total = 0;
// Value to hold the current working index in the list
int currentIndex = 0;
// Semaphore to lock the total count
sem_t totalSem;
// Semaphore to lock the current index
sem_t indexSem;
sem_t arySem;
// Value to represent whether or not we are still reading values
int readingValues = 1;

// Used to expand the array, doubles the capacity - doesn't do any check to see if it needs resizing.

void expandAry() {
    sem_wait(&arySem);
    vList = realloc(vList, sizeof(long)*vSize*2);
    vSize = vSize*2;
    sem_post(&arySem);
}

// Read the list of points from standard input.
void readValues() {
    // Read long values until we can't read any more.
    long v;
    while ( scanf( "%ld", &v ) == 1 ) {
        if ( vCount == vSize)
            expandAry();
        
        // Add the point we just read to the end of the list.
        vList[ vCount ] = v;
        vCount++;
    }
    printf("Finished reading values");
    readingValues = 0;
}
// Checks to see if there are more numbers to process, returning the number to process
    // or -1 if there are no more numbers to process
long getWork() {
    
    // Do until we break
    while (1) {
        // If we are no longer reading and we've read all in the list, break
        if (readingValues == 0 && currentIndex == vCount)
            break;
        // Acquire index and array locks
        sem_wait(&indexSem);
        sem_wait(&arySem);
        // Check to see if we can grab more
        if (currentIndex != vCount) {
            // Grab next value, increment the index and release locks/return
            long num = vList[currentIndex];
            currentIndex++;
            sem_post(&indexSem);
            sem_post(&arySem);
            return num;
        }
        // Release locks
        sem_post(&arySem);
        sem_post(&indexSem);
        
    }
    // Return -1 once theres no more list
    return -1;
}
/** Start routine for each worker. */
void *workerRoutine( void *arg ) {
    long val = getWork();
    while (val != -1) {
        int fcount = 0;
        // Check factors up to the square root of the number we're
        // factoring.
        long f = 2;
        while (f * f < val) {
            if (val % f == 0)
                // Count both f and vList[ j ] / f as factors.
                fcount += 2;
            f++;
        }
        // If the number is a perfect square, it gets one more
        // factor.
        if (f * f == val)
            fcount++;
        
        // Count this value, if it has the desired number of factors.
        if (fcount == fTarget) {
            sem_wait(&totalSem);
            total++;
            sem_post(&totalSem);
        }

        val = getWork();
    }

    return NULL;
}

int main( int argc, char *argv[] ) {
    int workers = 4;
    if ( argc != 3 ||
        sscanf( argv[ 1 ], "%d", &fTarget ) != 1 ||
        sscanf( argv[ 2 ], "%d", &workers ) != 1 ||
        workers < 1 )
        fail( "usage: fcount <factors> <workers>" );
    // Initialize the semaphore to protect the total
    sem_init(&totalSem, 0, 1);
    // Initialize the semaphore for the index
    sem_init(&indexSem, 0, 1);
    // Initialize the semaphore for the array
    sem_init(&arySem, 0, 1);
    // Allocate the array of values
    vList = (long *) malloc( INIT_SIZE * sizeof( long ) );
    
    
    // Make each of the workers.
    pthread_t worker[ workers ];
    for ( int i = 0; i < workers; i++ )
              pthread_create(&worker[i], NULL, workerRoutine, NULL);
    // Go and read values since we've created our threads
    readValues();
    
    // Wait until all the workers finish.
    for ( int i = 0; i < workers; i++ )
              pthread_join(worker[i], NULL);
    // Print out the totals
    printf( "Total: %d\n", total );
    free(vList);
    return 0;
}
