// Name for the queue of messages going to the server.
#define SERVER_QUEUE "/rjreiche-server-queue"

// Name for the queue of messages going to the current client.
#define CLIENT_QUEUE "/rjreiche-client-queue"

// Maximum number of words the server can store (capacity)
#define LIST_CAP 5

// Maximum length of a word on our LRU-ordered list.
#define WORD_MAX 10

// Maximum length for a message in the queue
// (Long enough for a report of all the words)
#define MESSAGE_LIMIT 1024
