#ifndef __BUFFET_H__
#define __BUFFET_H__

#include <stdbool.h>

/** Type for the name of a slice of pizza. */
typedef char Slice[ 11 ];

/** Capacity of the buffet, number of slices it can hold. */
#define BUFFET_CAP 20

/** Interface for the Buffet monitor.  If I was programming in
    C++, this would all be wrapped up in a class.  Since we're using
    C, it's just a collection of functions, with an initialization
    function to init the state of the whole monitor. */

/** Initialize the monitor. Create any needed mutex and condition
    variables and initialize any static global variables the monitor
    needs to.  */
void initBuffet();

/** Destroy the monitor, freeing any resources it uses. */
void destroyBuffet();

/** Print the buffet to standard out */
void printBuffet();

/** Called by non-vegetarian patrons to request a desired number of
    slices.  The monitor will store the type of slice the patron gets,
    oldest slice first, in the given slices array.  This function will
    block the caller until they can have all the slices they want; it
    won't take any slices from the buffet until they can get
    everything they want.  If a vegetarian is waiting for slices, this
    will only take non-vegetarian slices (slices other than cheese and
    veggie).  Otherwise, this function will give them any type of
    slice.  If the buffet is closed, this function will return
    false. */
bool takeAny( int desired, Slice slices[] );

/** This function is like takeAny(), but it takes the oldest
    vegetarian slices and stores them in the given slices array.  As
    with takeAny(), it will block the caller until they can have all
    the slices they want, and once the buffet is closed, this function
    will return false immediately. */
bool takeVeg( int desired, Slice slices[] );

/** This function adds count slices, all of the given slice type,
    stype, to the buffet.  The buffet only has a capacity of 20
    slices, If there isn't enough room for all the slices, it will add
    as many as it can to the buffet and then block the calling thread
    until there's room for more (note that this is different fom the
    behavior of takeAny() an takeVeg(), where you don't take any
    slices until you can have all of them.  This function immediately
    returns false when the buffet is closed.*/
bool addPizza( int count, Slice stype );

/** This function marks the buffet as closed, causing threads blocked
    in takeAny(), takeVeg() and addPizza() to to immediately return
    false (and threads subsequently calling these function to
    immediately return false. */
void closeBuffet();

#endif
