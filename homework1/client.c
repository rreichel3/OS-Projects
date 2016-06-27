//
//  client.c
//  Homework1
//
//  Created by rreichel on 1/26/16.
//  Copyright © 2016 robertReichel. All rights reserved.
//

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "common.h"

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

// Generic fail function. Takes a message to print out before failing.
static void fail( char const *message ) {
    fprintf( stderr, "%s\n", message );
    exit( 1 );
}

// Main function that does the heavy lifting for the client
// verifies user input, and acts based on their passed commands
int main( int argc, char *argv[] ) {
    // Server queue to be written to
    mqd_t serverQueue = mq_open( SERVER_QUEUE, O_WRONLY );
    // Client queue to read from
    mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_RDONLY );
    // If there are enough arguments for a reference command
    if (argc == REF_ARGC) {
        // Check that the command is valid
        if (strcmp(argv[1], REF_STR) != 0)
            fail("usage: ./client <command> [word]");
        // Initialize a buffer to parse the word in
        char *buffer = malloc(sizeof(char)*(MESSAGE_LIMIT+1));
        // Parse the user's word in
        if (sscanf(argv[2], "%s", buffer) != 1)
            fail("usage: ./client <command> [word]");
        // Check to make sure the MQ's have been succesffully connected to
        if ( serverQueue == -1 || clientQueue == -1 )
            fail( "Can't open the needed message queues" );
        // Write to the MQ the word the user provided
        mq_send(serverQueue, buffer, sizeof(char)*strlen(buffer)+1, 1);
        // Receive a resposnse from the server verifying addition to the list
        mq_receive(clientQueue, buffer, sizeof(char)*MESSAGE_LIMIT+1, NULL );
        // Print out the server's response
        printf("%s\n", buffer);
        // Close the queues
        mq_close( serverQueue );
        mq_close( clientQueue);
    }
    // If there are enough arguments for it to be a query command
    else if (argc == QUERY_ARGC) {
        // Compare the user's string against the string it should be
        if (strcmp(argv[1], QUERY_STR) != 0)
            fail("usage: ./client <command> [word]");
        // Ensure that the queues have initialized correctly
        if ( serverQueue == -1 || clientQueue == -1 )
            fail( "Can't open the needed message queues" );
        
        // Generate buffer to read/write to
        char *buffer = malloc(sizeof(char)*(MESSAGE_LIMIT+1));
        // Copy a null string into the buffer to request a query
        strcpy(buffer, NULL_STR);
        // Send the null string
        mq_send(serverQueue, buffer, sizeof(char)*strlen(buffer)+1, 1);
        // Initial receive. If theres anything in the list it'll print it out
        mq_receive(clientQueue, buffer, sizeof(char)*MESSAGE_LIMIT+1, NULL );
        // Loop until buffer has a null string (signaling end of transmission
        while (buffer[0] != '\0') {
            // Print out the message
            printf("%s\n", buffer);
            // Read in another message
            mq_receive(clientQueue, buffer, sizeof(char)*MESSAGE_LIMIT+1, NULL );
        }
        // Close the queues since we're done
        mq_close( serverQueue );
        mq_close( clientQueue);
        
    }
    else {
        // The user didn't correctly use the client program so we just
        // close what we've opened and fail.
        mq_close( serverQueue );
        mq_close( clientQueue);
        fail("usage: ./client <command> [word]");
    }
    

}
