// Test to make sure patrons only take slices if they can get all the
// slices they want.

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

/** Sade gets there first and starts adding slices. */
void *sade( void *arg ) {
  // One slice, which Dorothea should get.
  sleep( 1 );
  addPizza( 1, "asbestos" );
  addPizza( 1, "silt" );

  // Two slices, which should go to neville.
  sleep( 3 );
  addPizza( 1, "anchovies" );
  sleep( 1 );
  addPizza( 1, "pepperoni" );

  return NULL;
}

/** Neville gets there next, but can't have the three slices he wants, so he waits */
void *neville( void *arg ) {
  sleep( 2 );
  Slice names[ 3 ];
  if ( takeAny( 3, names ) ) {
    printf( "Neville gets: %s %s %s\n", names[ 0 ], names[ 1 ], names[ 2 ] );
  } else
    printf( "Neville gets nothing\n" );

  return NULL;
}

/** Dorothea gets there next.  She only wants two slices, so she can have them right away. */
void *dorothea( void *arg ) {
  sleep( 3 );
  Slice names[ 1 ];
  if ( takeAny( 1, names ) ) {
    printf( "Dorothea gets: %s\n", names[ 0 ] );
  } else
    printf( "Dorothea gets nothing\n" );

  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initBuffet();

  // Make a few server threads.
  pthread_t thread[ 3 ];

  if ( pthread_create( thread + 0, NULL, dorothea, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, neville, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, sade, NULL ) != 0 )
    fail( "Can't make a thread we need.\n" );

  // When we're done, join with all the threads.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyBuffet();

  return 0;
}
