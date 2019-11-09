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

http_request* parseReq(char* buffer){
    printf("%s\n", "PARSING REQ");
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

void add_header(http_request* request, char* name, char* value){
	//TODO does it matter the order of headers?
	http_header* new_header = malloc(sizeof(http_header));
	new_header->name = name;
	new_header->value = value;

	http_header* next = request->next;
	new_header->next = next;
	request->next = new_header;
}

void remove_header(http_request* request, char* name){
	http_header* past = NULL;
	http_header* current = request->next;
	while (current){
		if (strcmp(current->name, name) == 0){
			http_header* tmp = current;
			current = current->next;
			if (past){
				past->next = current;
			}
			else{
				request->next = current;
			}
			free(tmp);
		}
		else{
			past = current;
			current = current->next;
		}
	}
}

char* get_header(http_request* request, char* name){
	http_header* current = request->next;
	while (current){
		if (strcmp(current->name, name) == 0){
			return current->value;
		}
		current = current->next;
	}
	return NULL;
}

void replace_user_agent(http_request* request){
	remove_header(request, "User-Agent");
	add_header(request, "User-Agent", user_agent_hdr);
}

char* get_host_from_url(char* url){
	char* url_rest = url;
	char* host = strtok_r(url, "/", &url_rest);
	printf("%s\n", "HOSTS");
	printf("%s\n", url);
	printf("%s\n", host);
	printf("%s\n", url_rest);
	return host;
}

char* get_content_from_url(char* url){
	char* url_rest = url;
	char* host = strtok_r(url, "//", &url_rest);
	host = strtok_r(NULL, "/", &url_rest);
	char* content = malloc(sizeof(url));
	strcpy(content, "/");
	strcat(content, url_rest);
	return content;
}

void switchRequest(http_request* request){
    request->version = http_version_hdr;
    replace_user_agent(request);
    remove_header(request, "Connection");
    add_header(request, "Connection", "close");
	remove_header(request, "Proxy-Connection");
	add_header(request, "Proxy-Connection", "close");
	if (!get_header(request, "Host")){
		char* host = get_host_from_url(request->url);
		add_header(request, "Host", host);
	}
	if (request->url[0] != '/'){
		char* content = get_content_from_url(request->url);
		request->url = content;
	}
}

char* req_to_str(http_request* request){
	//TODO need to malloc?
	char* buffer = malloc(MAX_OBJECT_SIZE);
	strcpy(buffer, request->type);
	strcat(buffer, " ");
	strcat(buffer, request->url);
	strcat(buffer, " ");
	strcat(buffer, request->version);
	strcat(buffer, "\r\n");
	http_header* current = request->next;
	while(current){
		strcat(buffer, current->name);
		strcat(buffer, ":");//TODO add space?
		strcat(buffer, current->value);
		strcat(buffer, "\r\n");
		current = current->next;
	}
	strcat(buffer, "\r\n");
	return buffer;//TODO free
}

char *trimwhitespace(char *str){
	char *end;

	// Trim leading space
	while(isspace((unsigned char)*str)) str++;

	if(*str == 0)  // if it's all spaces
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;

	// Add new ending char
	end[1] = '\0';
	return str;
}


char* makeRequest(char* host, char* request){
	char* host_rest = host;
	char* server = strtok_r(host, ":", &host_rest);
	char* port = NULL;
	if (host_rest != NULL){
		port = host_rest;
	}
	server = trimwhitespace(server);
	port = trimwhitespace(port);
	
	rio_t rio;
	int request_sock = Open_clientfd(server, port);
	
	char* response = malloc(MAX_OBJECT_SIZE);
	memset(response, 0, sizeof(response));
	Rio_readinitb(&rio, request_sock);
	Rio_writen(request_sock, request, strlen(request));
	//send(request_sock, request, strlen(request), 0);
	char* response_line[MAXLINE];
	int header_size = 0;
	//Read headers
	while (Rio_readlineb(&rio, response_line, MAXLINE) > 0){
		if (strcmp(response_line, "\r\n") == 0){
			break;
		}
		header_size += strlen(response_line);
		//printf("{%s}", response_line);
		strcat(response, response_line);
	}
	strcat(response, "\r\n");//TODO need this?
	header_size += 2;
	//printf("\nBEGIN<%s>END\n", response);

	//Read body
	char body[MAX_OBJECT_SIZE];
	Rio_readnb(&rio, body, (MAX_OBJECT_SIZE - header_size));
	memcpy(response+header_size, body, (MAX_OBJECT_SIZE - header_size));
	//recv(request_sock, response, MAX_OBJECT_SIZE, 0);
	
	close(request_sock);

	return response;
}

void *handle_connection( void *fdp){ //STUB. TODO finish
    printf("%s\n", "MADE THREAD");
    int fd = *((int *)fdp);
    Pthread_detach(Pthread_self());
    Free(fdp);
    
    ///////////copied code to read and write
    char* buffer = Malloc(MAX_OBJECT_SIZE);
	recv(fd, buffer, MAX_OBJECT_SIZE, 0);
	printf("Buffer %s\n", buffer);

    http_request* request = parseReq(buffer);
	if (request == NULL){
		printf("%s\n", "Invalid HTTP request");
		close(fd);
		exit(-1);
	}

    switchRequest(request);

    char* reqBuffer = req_to_str(request);
    char* host = get_header(request, "Host");
    char* response = makeRequest(host, reqBuffer); //TODO
    free(reqBuffer);

    //send response
    Rio_writen(fd, response, MAX_OBJECT_SIZE);

    //free buffer
    free(buffer);
    close(fd);
    free(request);
    return 0;
}


int main(int argc, char** argv){
    printf("%s\n", "Begin main");
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
    printf("%s\n", "RUN PROXYHh");
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
    printf("%s\n", "END FUNCT");
    return( pointer[0] == '\r' &&
        pointer[3] == '\n' );
}
