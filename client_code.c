#include<stdio.h> 
#include <stdlib.h>
#include<string.h>  
#include<sys/socket.h>  
#include<arpa/inet.h> 
#include <unistd.h>
#include<json-c/json.h>
#include <pthread.h>
#define size 1024
#define port 8888
char *ip="127.0.0.1";
char Username[30];
char Password[30];
char message[size];
char complete_message[size];
char server_reply[size];
char encoded_pass[30]; //parola codificata(cea introdusa de client)
int chance_count=0;
int sock;
int leave_flag = 0;
unsigned reverse(unsigned x) {
   x = ((x & 0x55555555) <<  1) | ((x >>  1) & 0x55555555);
   x = ((x & 0x33333333) <<  2) | ((x >>  2) & 0x33333333);
   x = ((x & 0x0F0F0F0F) <<  4) | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) |
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

unsigned int crc32a(unsigned char *message) {
   int i, j;
   unsigned int byte, crc;

   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i] != 0) {
      byte = message[i];            // Get next byte.
      byte = reverse(byte);         // 32-bit reversal.
      for (j = 0; j <= 7; j++) {    // Do eight times.
         if ((int)(crc ^ byte) < 0)
              crc = (crc << 1) ^ 0x04C11DB7;
         else crc = crc << 1;
         byte = byte << 1;          // Ready next msg bit.
      }
      i = i + 1;
   }
   return reverse(~crc);
}

void encode_pass(char p[30]){
	int i;
	for(i=0;i<strlen(p);i++){
		encoded_pass[i]=p[i]+5;
	}
	encoded_pass[i+1]='\0'; 
}

int json_reader(char user[30], char pass[30]){
	FILE *fp;
	char buffer[1024];
    json_object *parsed_json;
	json_object *username; //username ul din json
    json_object *password; //parola din json
	json_object *value;
	char string_username[30];
	char string_password[30];
	int found=0;
	
	fp = fopen("users.json", "r");
	fread(buffer, 1024, 1, fp);
	fclose(fp);
	
	parsed_json = json_tokener_parse(buffer);
	
	for(int i=0; i < json_object_array_length(parsed_json); i++){
		value = json_object_array_get_idx(parsed_json, i);
		json_object_object_get_ex(value, "username", &username);
		json_object_object_get_ex(value, "password", &password);
		strcpy(string_username, json_object_get_string(username));
		strcpy(string_password, json_object_get_string(password));
		string_username[strlen(string_username)+1]='\0';
		string_password[strlen(string_password)+1]='\0';
		encode_pass(pass);  //pt a codifica parola introdusa de client => in encoded pass 
		//daca userul si parola(dupa codificare) introduse de client sunt egale cu cele din json=>succes
		encoded_pass[strlen(user)]='\0';
		if(!strcmp(string_username, user) && !strcmp(string_password, encoded_pass)){
			return 1;
		}
	}
	return 0;
}
void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}
void send_msg_handler() {
  while(1) {
	str_overwrite_stdout();
	 scanf(" %[^\n]", message);
	// int checksum=crc32a(message);
	//sprintf(complete_message, "%s//%d", message, checksum);
	if((send(sock, message, size , 0))<0) {
		break;
	}
    if (strcmp(message, "Exit") == 0) {
			break;
    } 
	bzero(message, size);
	bzero(complete_message, size);
  }
  leave_flag = 1;
}

void recv_msg_handler() {
  while (1) {
		int receive = recv(sock, message, size, 0);
    if (receive > 0) {
      printf("%s", message);
	  str_overwrite_stdout();
    } else if (receive < 0) {
			break;
    } 
		bzero(message, size);
  }
}
int main(int argc , char *argv[]) {


struct sockaddr_in server;

sock = socket(AF_INET , SOCK_STREAM , 0);
//int socket(int domain,int type,int protocol)
//AF_INET : reprezinta familia adreselor cu care socket-ul va comunica , in cazul nostru va lucra cu adrese IPv4
//SOCK_STREAM (TCP) protocol de comunicatii orientate pe conexiune (client-server)

if(sock<0) {
	
	printf("eroare la crearea unui socket");
	exit(-1);
	
}
server.sin_addr.s_addr = inet_addr(ip);
server.sin_family = AF_INET;
server.sin_port = htons(port); //converteste din formatul intern in cel extern , 2 octteti
printf("Bun venit");
printf("\nPentru a te conecta la server te rog sa introduci un username si o parola");

printf("\nUsername:");scanf("%s",Username);
printf("Password:");scanf("%s", Password);

while(chance_count<4 || json_reader(Username, Password)==1){

if(json_reader(Username, Password)==1){
		printf("Username-ul si parola sunt corecte!");

	  printf("\nMai jos se afla comenzi de care utilizatorul beneficeaza:");
	  printf("\nWho is online  (afiseaza pe ecran , toti utilizatorii aflati in sesiunea)");
	  printf("\n<private> <nume_username>  mesaj (mesajul se afla pe ecranul utlizatorului la care se doreste sa fie transmis)");
	  printf("\nExit (pentru deconectarea de la sesiune)\n");
	  
						//int connect(int sockfd , const struct sockaddr *addr , socklen_t addrlen)

						if(connect(sock , (struct sockaddr *)&server,sizeof(server))<0) {
							printf("\neroare de conectare la server");
							exit(-2);
						}
	  //sprintf(message,"%s %s",Username,Password);
	  //if( send(sock,message,strlen(message),0) < 0)
						if( send(sock,Username,30,0) < 0) {
							printf("eroare la trimitire");
							exit(-4);
						}
						pthread_t send_msg;
						if(pthread_create(&send_msg , NULL , (void*)send_msg_handler,NULL) <0 ) {
						
							printf("eroare la pthread send_msg\n");
							exit(-5);
						
						}
						pthread_t recv_msg;
						if(pthread_create(&recv_msg , NULL , (void*)recv_msg_handler,NULL) <0 ) {
						
							printf("eroare la pthread recv_msg\n");
							exit(-6);
						
						}
					while (1){
							if(leave_flag){
							break;
							}				
						}
						close(sock);
						return 0;
	  
				}
	else {
		printf("Incercati din nou!");
		printf("\nUsername:");scanf("%s",Username);
		printf("Password:");scanf("%s", Password);
	   }
	chance_count++;
	if(chance_count==4){
	printf("Numarul de incercari s-a terminat, va rugam incercati mai tarziu :) \n");
	}
  }
}