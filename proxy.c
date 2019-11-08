#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "csapp.h"
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_LEN 2000

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static char *http_version_hdr = "HTTP/1.0";
int sd;
struct sockaddr_in addr;

struct http_header;
struct http_header{
	char* name;
	char* value;
	struct http_header* next;
};
typedef struct http_header http_header;

typedef struct{
	char* type;
	char* url;
	char* version;
	http_header* next;
} http_request;


//DONT NEED? open_listenfd does this for me??
// int init_socket(int port){
//     printf("CREATE SOCKET");
//     //create the socket
//     sd = socket( AF_INET, SOCK_STREAM, 0 );
//     if(sd < 0){
//         printf("%s\n", "Your socket sux");
//         return -1;
//     }

//     //set socket addr
//     memset(&addr, 0, sizeof(addr));
//     addr.sin_family = AF_INET;
// 	addr.sin_port = htons((u_short) port);
// 	addr.sin_addr.s_addr = INADDR_ANY;

//     //bind the socket
//     if (bind(sd, (struct sockaddr *) &addr, sizeof(addr)) < 0){
// 		printf("%s\n", "Do you even bind bruh?");
// 		return -1;
// 	}

//     //now we listen
//     if( listen( sd, 100 ) == -1 ) {
//         printf("%s\n", "Listen Linda");
//         return -1;
//     }

//     //made it
//     return 0;
// }

http_request* parseReq(char* buffer){
	http_request* request = malloc(sizeof(http_request));
	char* buffer_rest = buffer;
	char* line = strtok_r(strdup(buffer), "\r\n", &buffer_rest);
	char* line_rest = line;

	char* type = strtok_r(strdup(line), " ", &line_rest);
	request->type = type;//TODO need to malloc?

	char* url = strtok_r(NULL, " ", &line_rest);
	request->url = url;

	char* version = strtok_r(NULL, " ", &line_rest);
	request->version = version;

	http_header* current = NULL;

	while (line = strtok_r(NULL, "\r\n", &buffer_rest)){
		line_rest = line;

		char* name = strtok_r(strdup(line), ": ", &line_rest);
		//printf("%s:%s\n", name, line_rest);
		http_header* header = malloc(sizeof(http_header));
		header->name = name;
		header->value = line_rest;//TODO malloc string?
		header->next = NULL;
		if (current == NULL){
			request->next = header;
			current = header;
		}
		else{
			current->next = header;
			current = header;
		}
		//printf("%s\n", line);
	}

	printf("%s\n", "PARSED");
	return request;
}


void *handle_connection( void *fdp){ //STUB. TODO finish
    printf("MADE THREAD\n");
    int fd = *((int *)fdp);
    pthread_detach(pthread_self);
    char buffer[MAX_LEN];
    free(fdp);
    
    printf("ZAI\n");

    ///////////copied code to read and write
    //TODO this should be in thread
    int total_bytes_read = 0, bytes_read;
    do {
        bytes_read = read( fdp, buffer + total_bytes_read, 4096 - total_bytes_read );

        if( bytes_read == -1 ){
        printf( "read error\n" );
        exit( -1 );
        }

        total_bytes_read += bytes_read;
    } while( total_bytes_read < 4 || !at_end( buffer + total_bytes_read - 4 ) ); 
            // resource identifer has form /<number>+<number>

    http_request* request = parseReq(buffer);
	if (request == NULL){
		printf("%s\n", "Invalid HTTP request");
		close(fdp);
		return;
	}


    // resource identifier starts 4 bytes in
    // <number>+<number> starts 5 bytes in
    char *resource = buffer + 5, *p = resource;

    while( *p != ' ' ) {
        ++p;
    }

    *p = '\0';

    printf("resoure: \n");
    printf( "%s\n", resource );
    printf("done");

    int n, m;

    sscanf( resource, "%d+%d", &n, &m );

    sprintf( buffer, "HTTP/1.1 200 thumbsup\r\nContent-Encoding: text/plain\r\nContent-Length: %d\r\n\r\n%d", 2 , n + m );

    int total_bytes_written = 0, bytes_to_write = strlen( buffer ), bytes_written;

    do {
        bytes_written = write( fdp, buffer + total_bytes_written, bytes_to_write - total_bytes_written );

        if( bytes_written == -1 ) {
        printf( "write error\n" );
        exit( -1 );
        }

        total_bytes_written += bytes_written;
    } while( total_bytes_written < bytes_to_write );

    close(fd);
    return 0;
}


int main(int argc, char** argv)
{
    printf("Begin main\n");
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid; 

    //Check args
    if (argc < 2){
        printf("Wrong args: %s \n", argv[0]);
        exit(0);
    }

    //run proxy
    printf("RUN PROXYHh\n");
    listenfd = open_listenfd(argv[1]);
    while (1) {
        clientlen=sizeof(struct sockaddr_storage);
	    connfdp = malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
	    *connfdp = accept(listenfd, (SA *) &clientaddr, &clientlen); //line:conc:echoservert:endmalloc
	    pthread_create(&tid, NULL, handle_connection, connfdp);
    }

    return 0;
}

int at_end( char *pointer ) {
    printf("END FUNCT\n");
    return( pointer[0] == '\r' &&
        pointer[3] == '\n' );
}

void createRequest(char** argv){
    
}
