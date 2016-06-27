// Simple test case for the monitor.  A non-veg patron can take
// veg slices, if they get there first.

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

/** Laurie tries to take two slices of anything. */
void *laurie( void *arg ) {
  sleep( 2 );
  Slice names[ 2 ];
  if ( takeAny( 2, names ) ) {
    printf( "Laurie gets: %s %s\n", names[ 0 ], names[ 1 ] );
  } else
    printf( "Laurie gets nothing\n" );

  return NULL;
}

/** Ladonna gets to the buffet and wants two veg slices, but laurie
    already took one. */
void *ladonna( void *arg ) {
  sleep( 3 );
  Slice names[ 2 ];
  if ( takeVeg( 2, names ) ) {
    printf( "Ladonna gets: %s %s\n", names[ 0 ], names[ 1 ] );
  } else
    printf( "Ladonna gets nothing\n" );

  return NULL;
}

/** Trent puts slices on the buffet, gradually. */
void *trent( void *arg ) {
  sleep( 1 );
  addPizza( 1, "veggie" );
  addPizza( 1, "ham" );

  sleep( 3 );
  addPizza( 1, "cheese" );

  sleep( 1 );
  addPizza( 1, "bacon" );

  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initBuffet();

  // Make a few server threads.
  pthread_t thread[ 3 ];

  if ( pthread_create( thread + 0, NULL, ladonna, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, laurie, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, trent, NULL ) != 0 )
    fail( "Can't make a thread we need.\n" );

  sleep( 6 );

  // ladonna will be stuck, since there's only one veg slice left.
  closeBuffet();

  // When we're done, join with all the threads.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyBuffet();

  return 0;
}
