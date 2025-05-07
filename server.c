#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int port_number = 8080;
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 2

typedef struct storage{
        char* name;
        char* ip;
        int socket_fd;
        struct storage* next;
}storage;

storage* head = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void insert_clients(char* ip,char* name, int socket_fd){
        pthread_mutex_lock(&lock);
        storage* new_node = (storage*)malloc(sizeof(storage));
        new_node->ip = strdup(ip);
        new_node->name = strdup(name);
        new_node->socket_fd = socket_fd;
        new_node->next = head;
        head = new_node;
        pthread_mutex_unlock(&lock);
}

void print_list(){
        pthread_mutex_lock(&lock);
        storage* curr = head;
        while(curr!=NULL){
                printf("IP:%s\tName:%s\n",curr->ip,curr->name);
                curr = curr->next;
        }
        pthread_mutex_unlock(&lock);
}

storage* find_client(char* client_ip, char* client_name){
        pthread_mutex_lock(&lock);
        storage* curr = head;
        while(curr!=NULL){
                if(strcmp(curr->ip,client_ip)==0 && strcmp(curr->name,client_name)==0){
                        pthread_mutex_unlock(&lock);
                        return curr;
                }
                curr = curr->next;
        }
        pthread_mutex_unlock(&lock);
        return NULL;
}

void* handle_client(void* arg){
        int client_fd = *(int*)arg;
        free(arg);
        char buffer[BUFFER_SIZE];

        while(1){
                int bytes_received = recv(client_fd,buffer,BUFFER_SIZE-1, 0);
                if(bytes_received<=0){
                        printf("client disconnected");
                        close(client_fd);
                        break;
                }

                buffer[bytes_received] = '\0';
                printf("Received:%s\n",buffer);

                if(strncmp(buffer,"REGISTER;",9)==0){
                        char* data = buffer + 9;
                        char* ip = strtok(data,";");
                        char* name = strtok(NULL, ";");
                        if(ip && name){
                                insert_clients(ip,name,client_fd);
                                print_list();
                                char ack[] = "Registered successfully!";
                                send(client_fd,ack,strlen(ack),0);
                        }
                }else if(strncmp(buffer,"SEND;",5)==0){
                        char* data = buffer + 5;
                        char* sender_ip = strtok(data,";");
                        char* sender_name = strtok(NULL, ";");
                        char* receiver_ip = strtok(NULL, ";");
                        char* receiver_name = strtok(NULL, ";");
                        char* msg = strtok(NULL, "");

                        if(sender_ip && sender_name && receiver_ip && receiver_name && msg){
                                storage* receiver = find_client(receiver_ip,receiver_name);
                                if(receiver){
                                        send(receiver->socket_fd,msg,strlen(msg),0);
                                }else{
                                        char err[] = "Receiver not found or offline.\n";
                                        send(client_fd, err, strlen(err), 0);
                                }
                        }
                }else{
                        char err[] = "Unknown command.\n";
                        send(client_fd, err, strlen(err), 0);
                }
                memset(buffer, 0, BUFFER_SIZE);
        }
        return NULL;

}

int main()
{
        int server_fd, client_fd;
        struct sockaddr_in client_addr,server_addr;
        socklen_t client_addr_len;

        //socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(server_fd<0){
                perror("socket failed");
                exit(EXIT_FAILURE);
        }

        server_addr.sin_family=AF_INET;
        server_addr.sin_port = htons(port_number);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        //bind
        if(bind(server_fd,(struct sockaddr*)&server_addr, sizeof(server_addr))<0){
                perror("bind failed");
                exit(EXIT_FAILURE);
        }

        //listen
        if(listen(server_fd,5)<0){
                perror("listen failed");
                exit(EXIT_FAILURE);
        }

        printf("Peer1 listening on port %d\n",port_number);
        client_addr_len = sizeof(client_addr);

        while(1){
                //accept
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if(client_fd<0){
                        perror("accept failed");
                        //exit(EXIT_FAILURE);
                        continue;
                }
                printf("connection established with client\n");

                int* new_sock = malloc(sizeof(int));
                *new_sock = client_fd;

                pthread_t tid;
                if(pthread_create(&tid,NULL,handle_client,new_sock)!=0){
                        perror("pthread create failed");
                        close(client_fd);
                        free(new_sock);
                }
                pthread_detach(tid);
        }

        close(server_fd);
        return 0;
}