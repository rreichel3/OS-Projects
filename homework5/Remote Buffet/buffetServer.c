#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include "buffet.h"

/** Port number used by my server */
#define PORT_NUMBER "26160"
/** Command for adding slices */
#define ADD_CMD "add"
/** Command for vegetarian slices */
#define VEG_CMD "veg"
/** Command for any slice */
#define ANY_CMD "any"
/** Max length for a slice name */
#define SLICE_NAME_LENGTH 10
/** Max length for a command */
#define MAX_CMD_LENGTH 3
/** Total max length */
#define MAX_LENGTH MAX_CMD_LENGTH+1+2+1+SLICE_NAME_LENGTH+1

/** Flag for if the server is running */
int runningSrv = 0;

int servSock;

// Print out an error message and exit.
static void fail( char const *message ) {
    fprintf( stderr, "%s\n", message );
    exit( 1 );
}
// Function to handle a bad command
void badCommand(FILE *fp) {
    fprintf( fp, "Unrecognized command\n" );
    fprintf( fp, "cmd> " );
    fflush( fp );
}
/** handle a client connection, close it when we're done. */
void *handleClient( void *i ) {
    int sock = *((int *) i);
    // Here's a nice trick, wrap a C standard IO FILE around the
    // socket, so we can communicate the same way we would read/write
    // a file.
    FILE *fp = fdopen( sock, "a+" );
    
    // Prompt the user for a command.
    fprintf( fp, "cmd> " );
    // Buffer to hold the command
    char cmd[ 11 ];
    // Scan in the next command and check if it is quit
    while ( fscanf( fp, "%4s", cmd ) == 1 &&
           strcmp( cmd, "quit" ) != 0 ) {
        // Declare an integer to store our count
        int count;
        // If the command is "add"
        if (strcmp(cmd, ADD_CMD) == 0) {
            // Allocate array to parse in the slice type
            char name[11];
            // If it parses correctly, add pizza
            if (fscanf( fp, "%d %10s", &count, name ) == 2 ) {
                if (!addPizza(count, name))
                    break;
            }
            else {
                // If bad command then run bad command
                badCommand(fp);
                continue;
            }
        }
        // If the command is "veg"
        else if (strcmp(cmd, VEG_CMD) == 0) {
            // Scan in the number
            if (fscanf( fp, "%d", &count) != 1) {
                badCommand(fp);
                break;
            }
            // Allocate an array to hold the slices
            Slice *slices = calloc(count+1, sizeof(Slice));
            // Take veggie slices. This may block
            if (!takeVeg(count, slices))
                break;
            // Print out the returned slices
            for (int i = 0; i < count; i++) {
                fprintf(fp, "%s ", slices[i]);
            }
            // Write a newline
            fprintf(fp, "\n");
        }
        // If the command is "any"
        else if (strcmp(cmd, ANY_CMD)  == 0) {
            // Scan in the count, if bad then run bad command
            if (fscanf( fp, "%d", &count) != 1) {
                badCommand(fp);
                break;
            }
            // Create an array of slices
            Slice *slices = calloc(count+1, sizeof(Slice));
            // Call take any. This might block
            if (!takeAny(count, slices))
                break;
            // Print out the array of slices that was returned
            for (int i = 0; i < count; i++) {
                fprintf(fp, "%s ", slices[i]);
            }
            // Write a newline
            fprintf(fp, "\n");
        }
        else {
            badCommand(fp);
            continue;
        }
        // Just echo the command back to the client.
        
        // Prompt the user for the next command.
        fprintf( fp, "cmd> " );
        // Flushhhhh
        fflush( fp );
    }
    
    // Close the connection with this client.
    // and free memory
    free(i);
    fclose( fp );
    return NULL;
}

/** Function to handle interrupts. Closes buffet, prints it out
    destroys the buffet, closes the socket and exits the program
 */
void interruptHandler(int nothing) {
    closeBuffet();
    printBuffet();
    destroyBuffet();
    runningSrv = 0;
    close( servSock );
    exit(0);
}

int main( int argc, char *argv[] ) {
    // Prepare a description of server address criteria.
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_INET;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;
    
    // Lookup a list of matching addresses
    struct addrinfo *servAddr;
    if ( getaddrinfo( NULL, PORT_NUMBER, &addrCriteria, &servAddr) )
        fail( "Can't get address info" );
    
    // Try to just use the first one.
    if ( servAddr == NULL )
        fail( "Can't get address" );
    
    // Create a TCP socket
    servSock = socket( servAddr->ai_family, servAddr->ai_socktype,
                          servAddr->ai_protocol);
    if ( servSock < 0 )
        fail( "Can't create socket" );
    
    // Bind to the local address
    if ( bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) != 0 )
        fail( "Can't bind socket" );
    
    // Tell the socket to listen for incoming connections.
    if ( listen( servSock, 5 ) != 0 )
        fail( "Can't listen on socket" );
    
    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);
    
    // Fields for accepting a client connection.
    struct sockaddr_storage clntAddr; // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);
    initBuffet();
    signal(SIGINT, interruptHandler);
    runningSrv = 1;
    while ( runningSrv ) {
        // Accept a client connection.
        int *sock = malloc(sizeof(*sock));
        *sock = accept( servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
        pthread_t thread;
        // Talk to this client.
        pthread_create(&thread, NULL, handleClient, sock);
        pthread_detach(thread);
    }
    
    
    return 0;
}


