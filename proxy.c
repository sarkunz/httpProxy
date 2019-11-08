#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "csapp.h"
#include <string.h>
#include <time.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static char *http_version_hdr = "HTTP/1.0";
int sd;
struct sockaddr_in addr;
char buffer[MAX_OBJECT_SIZE];

int init_socket(int port){
    //create the socket
    sd = socket( AF_INET, SOCK_STREAM, 0 );
    if(sd < 0){
        printf("%s\n", "Your socket sux");
        return -1;
    }

    //set socket addr
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short) port);
	addr.sin_addr.s_addr = INADDR_ANY;

    //bind the socket
    if (bind(sd, (struct sockaddr *) &addr, sizeof(addr)) < 0){
		printf("%s\n", "Do you even bind bruh?");
		return -1;
	}

    //now we listen
    if( listen( sd, 100 ) == -1 ) {
        printf("%s\n", "Listen Linda");
        return -1;
    }

    //made it
    return 0;
}

void *handle_connection( void *fdp){ //STUB. TODO finish
    printf("MADE THREAD\n");
    int fd = *((int *)fdp);
    pthread_detach(pthread_self);
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

    // parse the request
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

}

int run_proxy_run(int port){
    printf("RUN PROXYH\n");
     if(init_socket(port) < 0){
        printf("%s%i%s\n", "Screwed up socket for (", port, ")");
		return -1;
    }

    //create threads here
    //TODO this should parse req

    //init cache here

    //accept incoming connections
    socklen_t clientLen;
	struct sockaddr_in clientAddr;
	
    pthread_t tid; 
    int listenfd, *fdp; //ptr to avoid race condition REMEMBER: free fdp after dereferencing it in thread
    listenfd = open_listenfd((char*)port);
    while(1){
        printf("WHILE LOOP\n");

        clientLen=sizeof(struct sockaddr_storage);
	    fdp = malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
        if(fdp < 0){ //may break
            printf("%s\n", "Just can't accept that");
            return -1;
        }
	    *fdp = accept(listenfd, (struct sockaddr *) &clientAddr, &clientLen); //line:conc:echoservert:endmalloc
	    pthread_create(&tid, NULL, handle_connection, fdp);

	}

    //close proxy
    //free cache
    close(fdp);
    return 0;
}

int main(int argc, char** argv)
{
    printf("Begin main\n");
    int listenfd, connfd, clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    int port, sockfd;

    //Check args
    if (argc < 2){
        printf("Wrong args: %s \n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);
    if(port == 0){
        printf("%s\n", "Put in a real port idiot");
        exit(0);
    }

    //run proxy
    run_proxy_run(port);

    return 0;
}

int at_end( char *pointer ) {
    printf("END FUNCT\n");
    return( pointer[0] == '\r' &&
        pointer[3] == '\n' );
}

void createRequest(char** argv){
    
}
