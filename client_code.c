#include<stdio.h> 
#include <stdlib.h>
#include<string.h>  
#include<sys/socket.h>  
#include<arpa/inet.h> 
#include <unistd.h>

#define size 1024
#define port 8888
char *ip="127.0.0.1";
char Username[30];
char Password[30];
char message[size];
char server_reply[size];

int main(int argc , char *argv[]) {

int sock;
struct sockaddr_in server;

sock = socket(AF_INET , SOCK_STREAM , 0);
//int socket(int domain,int type,int protocol)
//AF_INET : reprezinta familia adreselor cu care socket-ul va comunica , in cazul nostru va lucra cu adrese IPv4
//SOCK_STREAM (TCP) protocol de comunicatii orientate pe conexiune (client-server)

if(sock<0) {
	
	printf("eroare la crearea unui socket");
	exit(-1);
	
}

printf("Bun venit");
printf("\nPentru a te conecta la server te rog sa introduci un username si o parola");

printf("\nUsername:");scanf("%s",Username);
printf("Password:");scanf("%s",Password);

 server.sin_addr.s_addr = inet_addr(ip);
 server.sin_family = AF_INET;
 server.sin_port = htons(port); //converteste din formatul intern in cel extern , 2 octteti
	
//int connect(int sockfd , const struct sockaddr *addr , socklen_t addrlen)

printf("\nmai jos se afla comenzi de care utilizatorul beneficeaza");
printf("\nWho is online  (afiseaza pe ecran , toti utilizatorii aflati in sesiunea)");
printf("\nprivate nume_username (mesajul se afla pe ecranul utlizatorului la care se doreste sa fie transmis)");
printf("\npublic (mesajul se va transmite la toti utilizatorii inscrisi in sesiune)");
printf("\nExit (pentru deconectarea de la sesiune)");
if(connect(sock , (struct sockaddr *)&server,sizeof(server))<0) {
	
	printf("\neroare de conectare la server");
	exit(-2);
	
}
sprintf(message,"%s %s",Username,Password);
if( send(sock,message,strlen(message),0) < 0)
        {
            printf("eroare la trimitire");
            exit(-4);
        }
message[0]='\0';
if( recv(sock , server_reply , size , 0) < 0)
        {
			printf("eroare la primire");
            exit(-5);        
        }
printf("\n%s",server_reply);
server_reply[0]='\0';


 while(1)
    {

        printf("Enter message: ");
		scanf("%s" , message);
		char buffer[2*size];
		sprintf(buffer,"%s %s",Username,message);
        if(send(sock , buffer , strlen(buffer),0) < 0)
        {
            printf("eroare la trimitire");
            exit(-3);
        }
		buffer[0]='\0';
		if((strcmp(message,"Exit"))==0) {
			
				printf("\nDeconectare de la server");
				close(sock);
				exit(1);
				
		}
		
        if( recv(sock , server_reply , size , 0) < 0)
        {
			printf("eroare la primire");
            exit(-4);
        
        }
		printf("Server: %s\n", server_reply);
		server_reply[0]='\0'; 
    
	}

return 0;
	
}