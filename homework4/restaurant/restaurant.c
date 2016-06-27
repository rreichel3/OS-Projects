// Multi-threaded program to simulate servers and patrons at a pizza buffet restaurant.

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

/** Mary serves cheese pizza. */
void *mary( void *arg ) {
  while ( addPizza( 6, "cheese" ) )
    // Rest for a moment, then add some slices.
    usleep( rand() % 150 );

  return NULL;
}

/** Nelle serves pepperoni pizza. */
void *nelle( void *arg ) {
  while ( addPizza( 8, "pepperoni" ) )
    // Rest for a moment, then add some slices.
    usleep( rand() % 200 );

  return NULL;
}

/** John serves veggie pizza. */
void *john( void *arg ) {
  while ( addPizza( 3, "veggie" ) )
    // Rest for a moment, then add some slices.
    usleep( rand() % 75 );

  return NULL;
}

/** Genevive serves works pizza. */
void *genevive( void *arg ) {
  while ( addPizza( 4, "works" ) )
    // Rest for a moment, then add some slices.
    usleep( rand() % 100 );

  return NULL;
}

/** Laaverne eats any kind of pizza, 2 slices at a time */
void *laverne( void *arg ) {
  Slice names[ 2 ];
  while ( takeAny( 2, names ) ) {
    printf( "Laverne gets: %s %s\n", names[ 0 ], names[ 1 ] );

    // Rest for a moment, then have some more.
    usleep( rand() % 50 );
  }

  return NULL;
}

/** Michal eats vegetarian pizza, 3 slices at a time */
void *michal( void *arg ) {
  Slice names[ 3 ];
  while ( takeVeg( 3, names ) ) {
    printf( "Michal gets: %s %s %s\n", names[ 0 ], names[ 1 ], names[ 2 ] );

    // Rest for a moment, then have some more.
    usleep( rand() % 75 );
  }

  return NULL;
}

/** Kelsey eats vegetarian pizza, 1 slice at a time */
void *kelsey( void *arg ) {
  Slice names[ 1 ];
  while ( takeVeg( 1, names ) ) {
    printf( "Kelsey gets: %s\n", names[ 0 ] );

    // Rest for a moment, then have some more.
    usleep( rand() % 25 );
  }

  return NULL;
}

/** Daron eats any kind of pizza, 2 slices at a time */
void *daron( void *arg ) {
  Slice names[ 2 ];
  while ( takeAny( 2, names ) ) {
    printf( "Daron gets: %s %s\n", names[ 0 ], names[ 1 ] );

    // Rest for a moment, then have some more.
    usleep( rand() % 50 );
  }

  return NULL;
}

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initBuffet();

  // Make a few server threads.
  pthread_t thread[ 8 ];

  if ( pthread_create( thread + 0, NULL, mary, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, nelle, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, john, NULL ) != 0 ||
       pthread_create( thread + 3, NULL, genevive, NULL ) != 0 ||
       pthread_create( thread + 4, NULL, laverne, NULL ) != 0 ||
       pthread_create( thread + 5, NULL, michal, NULL ) != 0 ||
       pthread_create( thread + 6, NULL, kelsey, NULL ) != 0 ||
       pthread_create( thread + 7, NULL, daron, NULL ) != 0 )
    fail( "Can't make a thread we need.\n" );

  // Buffet is open for 10 seconds, eat fast.
  sleep( 10 );
  closeBuffet();

  // When we're done, join with all the threads.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyBuffet();

  return 0;
}
