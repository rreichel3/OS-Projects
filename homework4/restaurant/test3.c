// Simple test case for the monitor.  A server will have to wait if the
// buffet is too full.

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

/** Scot gets there first, with 9 slices. */
void *scot( void *arg ) {
  sleep( 1 );
  if ( addPizza( 9, "squid" ) ) {
    printf( "Scot is dropped off 9 slices.\n" );
  } else {
    printf( "Scot couldn't drop off everything.\n" );
  }

  return NULL;
}

/** Shirlee gets there next, but can't drop off all her slices. */
void *shirlee( void *arg ) {
  sleep( 2 );
  if ( addPizza( 12, "sausage" ) ) {
    printf( "Shirlee is dropped off 12 slices.\n" );
  } else {
    printf( "Shirlee couldn't drop off everything.\n" );
  }

  return NULL;
}

/** Marnie takes some slices after three seconds.  This will let shirlee finish. */
void *marnie( void *arg ) {
  sleep( 3 );
  Slice names[ 2 ];
  if ( takeAny( 2, names ) ) {
    printf( "Marnie gets: %s %s\n", names[ 0 ], names[ 1 ] );
  } else
    printf( "Marnie gets nothing\n" );

  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initBuffet();

  // Make a few server threads.
  pthread_t thread[ 3 ];

  if ( pthread_create( thread + 0, NULL, marnie, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, shirlee, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, scot, NULL ) != 0 )
    fail( "Can't make a thread we need.\n" );

  // When we're done, join with all the threads.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyBuffet();

  return 0;
}
