#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PORT 9003
#define QUEUE_MAX_COUNT 5
#define BUFF_SIZE 1000000


#define SERVER_STRING "Server: Simple Http Server\r\n"

int read_file_to_buffer(char* filename, char buffer[BUFF_SIZE]) {
     size_t len;
	
     FILE *fp = fopen(filename, "r");
     if (fp != NULL) {
         len = fread(buffer, sizeof(char), BUFF_SIZE-1, fp);
         if (ferror(fp) != 0) {
             return -1;
         } else {
             buffer[len] = '\0';
         }
     } 
     else {
         return -1;
     }
    
     // Clean exit.
     fclose(fp);
     return len;
}

char * file_extension(char *fn)
{
	int i;
	int len = strlen(fn);
	for(i = 0; i < len; i++) 
	{
		if(fn[i] == '.') 
		{
			break;
		}
	}
	return fn+i+1;
}
	

int main()
{

    /* socket descriptor */
    int server_fd = -1;
    int client_fd = -1;
    char *fn = (char*)malloc(sizeof(20));;
    size_t len;

    u_short port = PORT;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    char buf[BUFF_SIZE];
    char recv_buf[BUFF_SIZE];
    char readBuf[BUFF_SIZE];
    int received_len = 0;

    /* create a socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(-1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    /* tcp/ip */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* bind */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

    /* start socket */
    if (listen(server_fd, QUEUE_MAX_COUNT) < 0) {
        perror("listen");
        exit(-1);
    }


    printf("http server running on port %d\n", port);

    while(1) {

        int file_exist = 1;
	    
        /* accept */

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("accept");
            exit(-1);
        }
        printf("accept a client\n");

        printf("client socket fd: %d\n", client_fd);
        /* call receive function */
        received_len = recv(client_fd, recv_buf, BUFF_SIZE, 0);

        printf("receive %d\n", received_len);

        printf("receive: --- \%s\n", recv_buf);

	fn = strtok(recv_buf, " ");
	fn = strtok(NULL, " ");

	if(strcmp(fn,"/") == 0)
	{
	    sprintf(fn, "index.html");
	}
	else if(fn[0] == '/')
	{
	    fn +=1;
	}
	
	printf("Requested file = %s\n", fn);

	len = read_file_to_buffer(fn, readBuf);
	
	if(len == -1) {
	    file_exist = 0;
	}
	
	printf("File length = %d\n", len);
	
	char *file_ext = file_extension(fn);
	char content_type[100];

	if(strcmp(file_ext, "css") == 0) {
		sprintf(content_type, "Content-Type=text/css\r\n");
	} else if(strcmp(file_ext, "html") == 0) {
		sprintf(content_type, "Content-Type=text/html\r\n");
	} else if(strcmp(file_ext, "png") == 0) {
		sprintf(content_type, "Content-Type=image/png\r\n");
	} else if(strcmp(file_ext, "js") == 0) {
		sprintf(content_type, "Content-Type=text/javascript\r\n");
	} else if(strcmp(file_ext, "mp4") == 0) {
		sprintf(content_type, "Content-Type=video/mp4\r\n");
	} else {
		printf("Unknown file extension - %s\n", file_ext);
	}
		
        /* Send to client */
	if(!file_exist) {
		sprintf(buf, "HTTP/1.0 404 OK\r\n");
		printf("Sending %s\n", buf);
		send(client_fd, buf, strlen(buf), 0);
	} else {
		sprintf(buf, "HTTP/1.0 200 OK\r\n");
		printf("Sending %s\n", buf);
		send(client_fd, buf, strlen(buf), 0);
		strcpy(buf, SERVER_STRING);
		send(client_fd, buf, strlen(buf), 0);
		sprintf(buf, content_type);
		send(client_fd, buf, strlen(buf), 0);
		strcpy(buf, "\r\n");
		send(client_fd, buf, strlen(buf), 0);
		send(client_fd, readBuf, len, 0);
	}
        /* close connection */
        close(client_fd);
    }
    close(server_fd);

    return 0;
}

