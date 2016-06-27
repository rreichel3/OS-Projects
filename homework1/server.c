#include "common.h"
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// String for the OK status printout
#define STATUS_OK "OK"
// Null string to be used by program
#define NULL_STR ""

// Global to be checked to ensure we actually interrupt when we need to
static int keepGoing = 1;

// List of words, sorted in LRU order, with the most recently
// referenced word first.
char list[ LIST_CAP ][ WORD_MAX + 1 ];

// Number of words in the list.
int wordCount;

/** Print out an error message and exit. */
static void fail( char const *message ) {
    fprintf( stderr, "%s\n", message );
    exit( 1 );
}
/**Handler for sigint, changes the keepGoing variable to false*/
void sigintHandler(int dummy) {
    keepGoing = 0;
}
/**
 Find word within the word list.
 @param word to be sought
 @return Index of word. If word is not found return -1
 */
int findInList(char *word) {
    for (int i = 0; i < wordCount-1; i++) {
        if (strcmp(word, list[i]) == 0)
            return i;
    }
    return wordCount;
}
/**
 Add a word to the list, pushing the rest down if the word isn't in the list
 @param word The char array of the word to be added
 */
void addWord(char *word) {
    int index = findInList(word);
    for (int i = index; i > 0; i--) {
        strcpy(list[i], list[i-1]);
    }
    strcpy(list[0], word);
    if (wordCount < 5)
        wordCount++;
    
}
/** Main function that does the server's heavy lifting. 
    Initializes the queues, registers for SIGINT, and creates
    an event loop to handle new messages
    @param argc Count of arguments
    @param argv array of arguments
    @return Exit status
 */
int main( int argc, char *argv[] ) {
    // Remove both queues, in case, last time, this program terminated
    // abnormally with some queued messages still queued.
    mq_unlink( SERVER_QUEUE );
    mq_unlink( CLIENT_QUEUE );
    
    // Register with SIGINT
    signal(SIGINT, sigintHandler);
    
    // Prepare structure indicating maximum queue and message sizes.
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = MESSAGE_LIMIT;
    
    // Make both the server and client message queues.
    mqd_t serverQueue = mq_open( SERVER_QUEUE, O_RDONLY | O_CREAT, 0600, &attr );
    mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_WRONLY | O_CREAT, 0600, &attr );
    if ( serverQueue == -1 || clientQueue == -1 )
        fail( "Can't create the needed message queues" );
    
    
    char *buffer = malloc(sizeof(char)*(MESSAGE_LIMIT+1));
    // Repeatedly read and process client messages.
    
    while ( keepGoing ) {
        
        // Check for Message
        mq_receive(serverQueue, buffer, sizeof(char)*MESSAGE_LIMIT+1, NULL );
        // Check to see if we have sigint'ed
        if (!keepGoing) break;
        // Check to see if a null has been passed (meaning they are requesting query)
        if (buffer[0] != '\0') {
            addWord(buffer);
            mq_send(clientQueue, STATUS_OK, sizeof(char)*strlen(STATUS_OK)+1, 1);
        }
        // if just requesting list
        else {
            for (int i = 0; i < wordCount; i++) {
                // Copy into a buffer to be written to the message queue
                strcpy(buffer, list[i]);
                mq_send(clientQueue, buffer, sizeof(char)*(strlen(buffer)+1), 1);
            }
            mq_send(clientQueue, NULL_STR, sizeof(char)*(strlen(NULL_STR)+1), 1);
        }
        
    }
    // Print out the list once the loop terminates
    for (int i = 0; i < wordCount; i++) {
        fprintf(stdout, "%s\n", list[i]);
    }

    
    // Close our two message queues (and delete them).
    mq_close( clientQueue );
    mq_close( serverQueue );
    
    // Delete the message queues, this is for normal server termination.
    mq_unlink( SERVER_QUEUE );
    mq_unlink( CLIENT_QUEUE );
    
    return 0;
}
