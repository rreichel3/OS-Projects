/**
 This program uses shared memory to maintain a list of words, submitted by the user
 using a reference command. If the shared memory doesn't yet exist, it creates it
 then handles the user's input. Also the list can be queried using a query command
 to get the entire list in its current state.
 @author Robert Reichel
 @file lru.c
 */
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

// Maximum number of words we're willing to store (capacity)
#define LIST_CAP 5

// Maximum length of a word on our list.
#define WORD_MAX 10

// Number of arguments for a reference command
#define REF_ARGC 3
// Number of arguments for the query command
#define QUERY_ARGC 2
//A null string to be used in various places
#define NULL_STR ""
// String that represents the query command
#define QUERY_STR "query"
// String that represents the reference command
#define REF_STR "reference"

typedef struct {
    /* List of words, in LRU order. */
    char list[ LIST_CAP ][ WORD_MAX + 1 ];
    /* Count of words in the list. */
    int wordCount;
    
} Queue;

/**
 Generic fail function that just prints out the messages passed to it
 then exits.
 @param message Message to be written to stderr
 */
static void fail( char const *message ) {
    fprintf( stderr, "%s\n", message );
    exit( 1 );
}
/**
 Main method that starts a new shared memory segment if it doesnt exist
 then handles the user's request either printing out the current list or
 adding a referenced word to the list
 @param argc The count of arguments
 @param argv array of strings passed in via the command line
 @return exit status
 */
int main( int argc, char *argv[] ) {
    //Create a key for our shared memory using my home directory
    key_t key = ftok("/afs/unity.ncsu.edu/users/r/rjreiche", 1);
    //Create the shared memory space if it doesn't already exist
    int shmid = shmget(key, sizeof(Queue), 0666 | IPC_CREAT);
    if (shmid == -1)
        fail("Can't create or access shared memory");
    // Map the shared memory space to a queue struct pointer
    Queue *q = (Queue *) shmat(shmid, 0, 0);

    //If there are enough arguments to be a reference command
    if (argc == REF_ARGC) {
        
        char *buffer = malloc(sizeof(char)*(WORD_MAX+1));
        // Check that the command is valid
        if (strcmp(argv[1], REF_STR) != 0)
            fail("usage: ./lru <command> [word]");
        // Initialize a buffer to parse the word in
        // Parse the user's word in
        if (sscanf(argv[2], "%s", buffer) != 1)
            fail("usage: ./lru <command> [word]");
        int index = q->wordCount-1;
        //Try to find the string in our list. If found, set the index to use
        int found = 0;
        for (int i = 0; i < q->wordCount; i++) {
            if (strcmp(buffer, q->list[i]) == 0) {
                index = i-1;
                found = 1;
                break;
            }
        }
        // Fix an offset ONLY if we arent at the max capacity
        if (index != LIST_CAP-1) {
            index++;
        }
        // Copy strings towards the bottom of the list
        for (int i = index; i > 0; i--) {
            strcpy(q->list[i], q->list[i-1]);
        }
        // Copy the new word to the top of the list
        strcpy(q->list[0], buffer);
        // If we aren't at the max, increment our count.
        if (q->wordCount < 5 && !found)
            q->wordCount++;
        //Disconnect from the shared memory
        shmdt(q);
        // Free the buffer we allocated earlier
        free(buffer);
    }
    // If there are enough arguments for it to be a query command
    else if (argc == QUERY_ARGC) {
        // Compare the user's string against the string it should be
        if (strcmp(argv[1], QUERY_STR) != 0)
            fail("usage: ./lru <command> [word]");
        for (int i = 0; i < q->wordCount; i++) {
            printf("%s\n", q->list[i]);
        }
        shmdt(q);


    }
    else {
        // The user didn't correctly use the program so we just
        // close what we've opened and fail.
        shmdt(q);

        fail("usage: ./lru <command> [word]");
    }

    
    
    
    
    return 0;
}
