//
//  buffet.c
//  restaurant
//  Program to model a pizza buffet, solving a slightly more complex
//  version of the producer/consumer problem with pizza slices!
//  Created by rreichel on 3/17/16.
//
//

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include "buffet.h"

// Mutex for the whole buffet
pthread_mutex_t editBuffet;
// Empty condition used if there are no slices left
pthread_cond_t emptyBuffet;
// Full condition, used if no space in buffet
pthread_cond_t fullBuffet;
// veg condition, used if there are no more vegetarian slices
pthread_cond_t vegBuffet;
// Bool to represent if a vegetarian is waiting
bool vegWait;
// Bool to represent that the buffet is still open
bool running;

// This struct represents the buffet tables that hold
// the slices, and has some build in accounting
typedef struct {
    Slice pizza[BUFFET_CAP];
    int vegCount;
    int totalCount;
} buffet;
// The buffet we will use. Everything initializes to zero.
buffet buffetH;

// Initialization function that, well, initializes the necessary mutexs/conditions
void initBuffet(){
    pthread_mutex_init(&editBuffet, NULL);
    pthread_cond_init(&emptyBuffet, NULL);
    pthread_cond_init(&fullBuffet, NULL);
    pthread_cond_init(&vegBuffet, NULL);
    vegWait = false;
    running = true;
    
}

void printBuffet() {
    for (int i = 0; i < buffetH.totalCount; i++) {
        printf("%s\n", buffetH.pizza[i]);
    }
}

/** Destroy the monitor, freeing any resources it uses. */
void destroyBuffet() {
    pthread_mutex_destroy(&editBuffet);
    pthread_cond_destroy(&emptyBuffet);
    pthread_cond_destroy(&fullBuffet);
    pthread_cond_destroy(&vegBuffet);
}

/** Remove a slice at the given index, shifting the remaining slices left */
void removeAtIdx(int index) {
    for (int i = index+1; i < buffetH.totalCount; i++) {
        strcpy(buffetH.pizza[i-1], buffetH.pizza[i]);
    }
}

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
bool takeAny( int desired, Slice slices[] ){
    // Acquire lock
    pthread_mutex_lock(&editBuffet);
    // If not running anymore, signal other threads, release lock and return false
    if (!running){
        pthread_cond_signal(&emptyBuffet);
        pthread_mutex_unlock(&editBuffet);
        return false;
    }
    // While we have don't enough to feed the person loop
    while (vegWait ? buffetH.totalCount-buffetH.vegCount < desired : buffetH.totalCount < desired) {
        // If not running anymore, signal other threads, release lock and return false
        if (!running){
            pthread_cond_signal(&emptyBuffet);
            pthread_mutex_unlock(&editBuffet);
            return false;
        }
        // Wait since we ran through the loop
        pthread_cond_wait(&emptyBuffet, &editBuffet);
    }
    // Initialize a counter and an index
    int count = 0;
    int index = 0;
    // If there are vegetarians waiting then take this branch
    if (vegWait) {
        // Add to the user's plate
        while (count < desired) {
            // If the slice isn't vegan then copy and remove, updating counters/index
            if (strcmp(buffetH.pizza[index], "cheese") != 0
                && strcmp(buffetH.pizza[index], "veggie") != 0) {
                strcpy(slices[count], buffetH.pizza[index]);
                removeAtIdx(index);
                count++;
                index--;
            }
            index++;
        }
        // Update the total count on the buffet
        buffetH.totalCount -= desired;
    }
    else {
        // Add any slice to the users plate until they have what they desire
        while (count < desired) {
            // Bool to see if a vegetarian slice has been removed.
            bool vegRemove = false;
            strcpy(slices[count], buffetH.pizza[index]);
            // If vegetarian slices removed then set that flag
            if (strcmp(buffetH.pizza[index], "cheese") == 0
                || strcmp(buffetH.pizza[index], "veggie") == 0)
                vegRemove = true;
            // Remove at index
            removeAtIdx(index);
            // Decrement total count
            buffetH.totalCount--;
            // If we removed a vegetarian slice, decrement vegCount
            if (vegRemove)
                buffetH.vegCount--;
            count++;
        }
    }
    pthread_cond_signal(&fullBuffet);
    pthread_mutex_unlock(&editBuffet);
    return true;
}

/** This function is like takeAny(), but it takes the oldest
 vegetarian slices and stores them in the given slices array.  As
 with takeAny(), it will block the caller until they can have all
 the slices they want, and once the buffet is closed, this function
 will return false immediately. */
bool takeVeg( int desired, Slice slices[] ){
    pthread_mutex_lock(&editBuffet);
    // If not running anymore, signal other threads, release lock and return false
    if (!running) {
        pthread_cond_signal(&vegBuffet);
        pthread_mutex_unlock(&editBuffet);
        return false;
    }
    // While we dont have enough vegetarian slices
    while (buffetH.vegCount < desired) {
        vegWait = true;
        // If not running anymore, signal other threads, release lock and return false
        if (!running) {
            pthread_cond_signal(&vegBuffet);
            pthread_mutex_unlock(&editBuffet);
            return false;
        }
        // Wait
        pthread_cond_wait(&vegBuffet, &editBuffet);
    }
    // Variables used in the adding of slices to a plate
    vegWait = false;
    int count = 0;
    int index = 0;
    // While we don't have the number we desired
    while (count < desired) {
        // Make sure the slice is vegetarian
        if (strcmp(buffetH.pizza[index], "cheese") == 0 ||
            strcmp(buffetH.pizza[index], "veggie") == 0) {
            strcpy(slices[count], buffetH.pizza[index]);
            // Remove at an index
            removeAtIdx(index);
            // Update counters
            index--;
            count++;
        }
        index++;
    }
    // UPdate the counts in the struct
    buffetH.totalCount -= desired;
    buffetH.vegCount -= desired;
    // Signal other threads
    pthread_cond_signal(&fullBuffet);
    pthread_mutex_unlock(&editBuffet);
    return true;
}

/** This function adds count slices, all of the given slice type,
 stype, to the buffet.  The buffet only has a capacity of 20
 slices, If there isn't enough room for all the slices, it will add
 as many as it can to the buffet and then block the calling thread
 until there's room for more (note that this is different fom the
 behavior of takeAny() an takeVeg(), where you don't take any
 slices until you can have all of them.  This function immediately
 returns false when the buffet is closed.*/
bool addPizza( int count, Slice stype ){
    pthread_mutex_lock(&editBuffet);
    // Boolean representing if the pizza being added is vegetarian
    bool vegetarian = false;
    // Logic to set vegetarian
    if (strcmp(stype, "cheese") == 0 ||
        strcmp(stype, "veggie") == 0) {
        // Yay, we're adding veggie pizza!
        vegetarian = true;
    }

    // While we still have pizza to add
    while (count != 0) {
        // If its not running then signal, unlock and return false
        if (!running){
            // Signal other threads
            pthread_cond_signal(&fullBuffet);
            // Unlock our mutex
            pthread_mutex_unlock(&editBuffet);
            return false;
        }
        // If we're at the cap, then wait
        if (buffetH.totalCount == BUFFET_CAP) {
            pthread_cond_wait(&fullBuffet, &editBuffet);
            // restart the while loop since things might have changed
            continue;
        }
        // Copy the type into the pizza tray
        strcpy(buffetH.pizza[buffetH.totalCount], stype);
        // Increment the total count of slices
        buffetH.totalCount++;
        // If vegetarian, increment the vegetarian count
        if (vegetarian)
            buffetH.vegCount++;
        // Decrement our count
        count--;
    }
    // All are added, signal empty buffet
    pthread_cond_signal(&emptyBuffet);
    // If vegetarian slices are added, signal those guys too
    if (vegetarian)
        pthread_cond_signal(&vegBuffet);
    // Release the mutex
    pthread_mutex_unlock(&editBuffet);
    return true;
    
}

/** This function marks the buffet as closed, causing threads blocked
 in takeAny(), takeVeg() and addPizza() to to immediately return
 false (and threads subsequently calling these function to
 immediately return false. */
void closeBuffet(){
    // Acquire lock
    pthread_mutex_lock(&editBuffet);
    // Set running to false
    running = false;
    // Signal all of the conditions we have running
    pthread_cond_signal(&fullBuffet);
    pthread_cond_signal(&emptyBuffet);
    pthread_cond_signal(&vegBuffet);
    pthread_mutex_unlock(&editBuffet);
}