#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

int port_number = 8080;
#define BUFFER_SIZE 1024

int sockfd;
char client_name[64];
char client_ip[64];

void* receive_handler(void *arg){
        char buffer[BUFFER_SIZE];
        while(1){
                int bytes_received=recv(sockfd,buffer,BUFFER_SIZE-1, 0);
                if(bytes_received>0){
                        buffer[bytes_received] = '\0';
                        printf("Received:%s\n",buffer);
                }
        }
        return NULL;
}

int main(int argc, char* argv[])
{
        if(argc!=3){
                fprintf(stderr,"Usage: %s <client_ip> <client_name>",argv[0]);
                exit(EXIT_FAILURE);
        }
        strncpy(client_ip,argv[1],sizeof(client_ip));
        strncpy(client_name,argv[2],sizeof(client_name));

        struct sockaddr_in server_addr;

        //socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd<0){
                perror("socket failed");
                exit(EXIT_FAILURE);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_number);

        //convert
        if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr)<0){
                perror("invalid address");
                exit(EXIT_FAILURE);
        }

        //connect
        if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
                perror("connect failed");
                exit(EXIT_FAILURE);
        }

        char register_msg[BUFFER_SIZE];
        snprintf(register_msg, sizeof(register_msg), "REGISTER;%s;%s",client_ip,client_name);
        send(sockfd,register_msg,strlen(register_msg),0);

        printf("Registered as %s (%s)\n", client_name, client_ip);

        pthread_t recv_thread;
        pthread_create(&recv_thread, NULL, receive_handler, NULL);

        printf("Enter message (format: <receiver_name>;<message>):\n");
        while(1){
                char input[BUFFER_SIZE];
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    input[strcspn(input, "\n")] = 0;

                    char receiver_name[64], message[BUFFER_SIZE];
                    if (sscanf(input, "%63[^;];%1023[^\n]", receiver_name, message) == 2) {
                        char safe_message[800];
                        strncpy(safe_message, message, sizeof(safe_message) - 1);
                        safe_message[sizeof(safe_message) - 1] = '\0';

                        char final_msg[BUFFER_SIZE];
                        snprintf(final_msg, sizeof(final_msg), "SEND;%s;%s;127.0.0.1;%s;%s",
                                 client_ip, client_name, receiver_name, safe_message);

                        send(sockfd, final_msg, strlen(final_msg), 0);
                    } else {
                        printf("Invalid input. Use format: <receiver_name>;<message>\n");
                    }
}
        }

        close(sockfd);
        return 0;

}