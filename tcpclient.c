#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<sys/types.h>
#include<arpa/inet.h> //inet_addr
#include <unistd.h> 
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "tokenizer.h"

pthread_t reader,writer;

void* readfrom(void* sock_desc){
  int sock = *(int*)sock_desc;
  char server_reply[2000];
  int read_size;
  CommandList* commando;

  while((read_size = recv(sock , server_reply , 1999 , 0))>0){
    
    commando = TokenizeString(server_reply);
    if(commando->count == 2){
      if(strcmp(commando->tail->next->command->token, "@rmvsoc")==0){
	send(sock,server_reply,strlen(server_reply),0);
	bzero(server_reply,2000);
	deleteCommandList(commando);
	free(commando);
	continue;
      }
    }
    
    puts(server_reply);
    fflush(stdout);
    deleteCommandList(commando);
    free(commando);
    
    
    if((strcmp(server_reply,"server exit")==0)||(strcmp(server_reply,"exit")==0)){
      puts("closing client");
      pthread_cancel(writer);
      exit(0);
    }
   
    bzero(server_reply,2000);

  }
  return 0;
}
void* writeto(void* sock_desc){
  int sock = *(int*)sock_desc;
  char buffer[256];
  
  
  while(1){
        
    bzero(buffer,256);
            
    // get a message from the client
    fgets(buffer,255,stdin);
    
   
 
    
    send(sock , buffer, strlen(buffer),0);
    
    bzero(buffer,256);
  
  }
}


int main(int argc , char *argv[])
{
  if(argc<4){
    return 1;
  }
    
  int portno;
  int sock;
  char* username;
  int on = 1;
  struct sockaddr_in server;
 
  struct hostent *serverIPAddress;
  username = malloc(strlen(argv[3])+1);
  strcpy(username,argv[3]);
  

  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
    {
      printf("Could not create socket");
    }
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  puts("Socket created");
  bzero((char *) &server, sizeof(server));

  serverIPAddress = gethostbyname(argv[1]);

  printf("connecting to hostname: %s \n",argv[1]);
  if (serverIPAddress == NULL)
    {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
    }

  bcopy((char *)serverIPAddress->h_addr, (char *)&server.sin_addr.s_addr, serverIPAddress->h_length);
  portno = atoi(argv[2]);
  server.sin_family = AF_INET;
  server.sin_port = htons(portno);
  printf("connecting to port %d\n",ntohs(server.sin_port));
  //Connect to remote server
  while (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
      perror("connect failed. Error,reconnect in two seconds");
      sleep(2);
    }
     
  puts("Connected\n");
  pthread_create(&reader,NULL,readfrom,(void*)&sock);
  send(sock , username, strlen(username),0);
  
  pthread_create(&writer,NULL,writeto,(void*)&sock);

  pthread_join(reader,NULL);
  pthread_join(writer,NULL);

  close(sock);
  return 0;
}
