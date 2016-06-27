// Simple test case for the monitor.  Non-veg patron won't take
// any veg slices if there's a vegetarian waiting.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for sleep/usleep
#include <stdbool.h>   // for bool

#include "buffet.h"

// General purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

/** Caitlyn tries to get two veg slices. */
void *caitlyn( void *arg ) {
  sleep( 2 );
  Slice names[ 2 ];
  if ( takeVeg( 2, names ) ) {
    printf( "Caitlyn gets: %s %s\n", names[ 0 ], names[ 1 ] );
  } else
    printf( "Caitlyn gets nothing\n" );

  return NULL;
}

/** Evangelina gets there next, and wants two slices of anything. */
void *evangelina( void *arg ) {
  sleep( 3 );
  Slice names[ 2 ];
  if ( takeAny( 2, names ) ) {
    printf( "Evangelina gets: %s %s\n", names[ 0 ], names[ 1 ] );
  } else
    printf( "Evangelina gets nothing\n" );

  return NULL;
}

/** Winnifred puts slices on the buffet, gradually. */
void *winnifred( void *arg ) {
  sleep( 1 );
  addPizza( 1, "cheese" );
  addPizza( 1, "meat" );

  sleep( 3 );
  addPizza( 1, "veggie" );

  sleep( 1 );
  addPizza( 1, "works" );

  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initBuffet();

  // Make a few server threads.
  pthread_t thread[ 3 ];

  if ( pthread_create( thread + 0, NULL, caitlyn, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, evangelina, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, winnifred, NULL ) != 0 )
    fail( "Can't make a thread we need.\n" );

  // When we're done, join with all the threads.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyBuffet();

  return 0;
}
