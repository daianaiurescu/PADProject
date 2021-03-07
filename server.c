#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#define SA struct sockaddr
#define MAX_CLIENTS 10
#define SIZE 2048
#define PORT 8888
int cli_count = 0;

/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Add clients to queue */
void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(char *name){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(!strcmp(clients[i]->name,name)){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients except sender */
void send_message(char *s, char *name){
	
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(strcmp(clients[i]->name,name)!=0){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void send_private_message(char *s, char *name, char *dest){
	
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			//printf("%s %d\n",clients[i]->name,strcmp(clients[i]->name,name));
			
			if(strcmp(clients[i]->name,name)!=0 && strcmp(clients[i]->name, dest)==0){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}
//online users

void online_users(char *name){
	
	int client_sockfd;
	char buffer1[2*SIZE];
	char buffer2[SIZE];
	strcpy(buffer2,"");
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(strcmp(clients[i]->name,name)!=0){
				sprintf(buffer1,"%s%s\n",buffer2,clients[i]->name);
				strcpy(buffer2,buffer1);
			}
			else
			 client_sockfd = clients[i]->sockfd;
		}
	}
	if (strlen(buffer2)==0)
	strcpy(buffer2,"Nimeni\n");
	if(write(client_sockfd, buffer2, strlen(buffer2)) < 0){
			perror("ERROR: write to descriptor failed");
		}
	pthread_mutex_unlock(&clients_mutex);
}

/* Handle all communication with the client */
void *handle_client(void *arg){
	char buff_out[SIZE];
	char name[32];
	int leave_flag = 0;
	char msg[2*SIZE];
	char mesaj[SIZE], user[32], var[SIZE];
	int i,j;
	cli_count++;
	client_t *cli = (client_t *)arg;
	int receive;
	// Name
	receive = recv(cli->sockfd, name, 32, 0);
		
	if(receive < 0){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else{
		name[receive-2]='\0';
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		send_message(buff_out, cli->name);
	}
	bzero(buff_out, SIZE/2);
	while(1){
		if (leave_flag) {
			  /* Delete client from queue and yield thread */
			close(cli->sockfd);
			queue_remove(cli->name);
			free(cli);
			cli_count--;
			pthread_detach(pthread_self());
			break;
		}

		receive = recv(cli->sockfd, buff_out, SIZE, 0);
		 if(receive<0){
			printf("ERROR: -1\n");
			leave_flag = 1;
		}
		buff_out[receive-2]='\0';
		 if (strcmp(buff_out, "Exit") == 0){
			sprintf(msg, "%s has left\n", cli->name);
			printf("%s", msg);
			send_message(msg, cli->name);
			leave_flag = 1;
		}
		else if (strcmp(buff_out, "Who is online") == 0){
			
			online_users(cli->name);
		} 		
		//else if(strlen(strstr(buff_out, "private "))>0){ 
			//printf("ceau\n");
			//printf("%s", buff_out);
			//strcpy(var, strstr(buff_out, "private "));
			//printf("%ld", strlen(var));
			//if(strlen(strcpy(var, strstr(buff_out, "private "))>0)){
				//var[strlen(var)]='\0';
				//while(var[i]!=' ' ){
				//	user[j]=var[i]; //destinatar
				//	i++;
				//	j++;
				//}
				//strcpy(mesaj,strstr(buff_out, " "));
				//sprintf(msg,"%s:%s\n",cli->name,mesaj);
				//send_private_message(msg,cli->name, user);
				
			//}
			else if(strstr(buff_out,"private ")!=NULL){
				sprintf(var, "%s",strstr(buff_out," "));
				i=1;
				j=0;
				while(var[i]!=' ' ){
					user[j]=var[i]; //destinatar
					i++;
					j++;
				}
				user[j]='\0';
				j=0;
				i++;
				//printf("%s\n",user);
						while(var[i]!='\0' ){
							mesaj[j]=var[i]; //mesaj
							i++;
							j++;
						}
				mesaj[j]='\0';
				sprintf(msg,"%s:%s\n",cli->name,mesaj);
				//printf("%s\n",mesaj);
				send_private_message(msg,cli->name, user);
				}
				else {
				//printf("%s\n",buff_out);
				sprintf(msg,"%s:%s\n",cli->name,buff_out);
				send_message(msg, cli->name);
				}
		bzero(mesaj,SIZE);
		bzero(user,32);	
		bzero(buff_out, SIZE);
		bzero(msg, 2*SIZE);
	}
	return NULL;
}

int main(int argc, char **argv){
	
	int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;
  /* Socket settings */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        printf("socket creation failed...\n");
        exit(-1);
    }
    printf("Socket successfully created..\n");
    // assign IP, PORT
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
  
    // Binding newly created socket to given IP and verification
    if ((bind(listenfd, (SA*)&serv_addr, sizeof(serv_addr))) != 0) {
        printf("socket bind failed...\n");
        exit(-1);
    }
    printf("Socket successfully binded..\n");
  
    // Now server is ready to listen and verification
    if ((listen(listenfd, 10)) < 0) {
        printf("Listen failed...\n");
        exit(-1);
    }
    printf("Server listening..\n");

	printf("=== WELCOME TO THE CHATROOM ===\n");

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		/* Check if max clients is reached */
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Max clients reached. Rejected: ");
			close(connfd);
			continue;
		}

		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;

		/* Add client to the queue and fork thread */
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

	}

	return 0;
}