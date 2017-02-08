#include "tcpserver.h"

color* createcolor(char* seq){
  color* new = (color*) malloc(sizeof(color));
  new->sequence = malloc(strlen(seq)+1);
  strcpy(new->sequence,seq);
  new->flag = 0;
  return new;
}
void  destroycolor(color* del){
  free(del->sequence);
  free (del);
  return;

}
colortable* initcolor(){
  colortable* new = (colortable*)malloc(sizeof(colortable));
  new->table =(color**) malloc(20*sizeof(color*));
  new->table[0] = createcolor("\033[1;32m");
  new->table[1] = createcolor("\033[1;31m");
  new->table[2] = createcolor("\033[1;33m");
  new->table[3] = createcolor("\033[1;34m");
  new->table[4] = createcolor("\033[1;35m");
  new->table[5] = createcolor("\033[1;36m");
  new->table[6] = createcolor("\033[1;32;41m");
  new->table[7] = createcolor("\033[1;33;45m");
  new->table[8] = createcolor("\033[1;34;45m");
  new->table[9] = createcolor("\033[1;31;44m");
  new->table[10] = createcolor("\033[1;36;41m");
  new->table[11] = createcolor("\033[1;37;40m");
  new->table[12] = createcolor("\033[1;32;44m");
  new->table[13] = createcolor("\033[1;30;47m");
  new->table[14] = createcolor("\033[1;31;42m");
  new->table[15] = createcolor("\033[1;31;43m");
  new->table[16] = createcolor("\033[1;32;41m");
  new->table[17] = createcolor("\033[1;33;41m");
  new->table[18] = createcolor("\033[1;36;41m");
  new->table[19] = createcolor("\033[1;34;42m");
  return new;
}

char* getcolor(colortable* pallet){
  int i;
  color* ptr;
  
  for(i = 0; i<20;i++){
    ptr = pallet->table[i];
    if (ptr->flag == 0){
      ptr->flag = 1;
      return ptr->sequence;
    }

  }

  return NULL;

}

int delcolor(colortable* pallet,char* seq){
  int i;
  color* ptr;

  for(i = 0; i<20;i++){
    ptr = pallet->table[i];
    if (strcmp(ptr->sequence,seq)== 0){
      ptr->flag = 0;
      return 0;
    }

  }

  return 1;

}




chat* createchat(){

  chat* new = (chat*)malloc(sizeof(chat));
  if (pthread_mutex_init(&new->chatmute, NULL) != 0)
    {
      printf("\n mutex init failed\n");
      return NULL;
    }
  new->map = constructor(50);
  return new;


}
int deleteaccount(char*accountname,chat* thischat, char* color,connectlist* private){
  connectnode* ptr; 
  char message[2000];
  account* thisacct;
  account* ptracct;
  pthread_mutex_lock(&thischat->chatmute);

  thisacct = delete(accountname,thischat->map);
  if (thisacct == NULL){
    pthread_mutex_unlock(&thischat->chatmute);
    return 1;
  }
  else{
    if(private->connectnum > 0){
      ptr = private->head;

      pthread_mutex_unlock(&thischat->chatmute);

      while(ptr!=NULL){

	
        ptracct = accessacct(ptr->acctname,thischat);
      
       
	if(ptracct == NULL){
          ptr = ptr->next;
          continue;
        }
        pthread_mutex_unlock(&ptracct->accountmute);
        bzero(message,2000);
        sprintf(message,"\nSERVER: %s has left, ending a private sesh with you",accountname);
        send(ptracct->acctsock,message,strlen(message),0);
        bzero(message,2000);
        ptracct->privatesock = 0;
        ptr = ptr->next;
      }
      pthread_mutex_lock(&thischat->chatmute);

    } 

    if(pthread_mutex_trylock(&thisacct->accountmute)==0){ //if no one has private comm
      free(thisacct->acctname);
      pthread_mutex_unlock(&thisacct->accountmute);
      pthread_mutex_destroy(&thisacct->accountmute);

      removesock(globallist,thisacct->acctsock);
      free(thisacct);
      acctslots--;
      
      delcolor(colorrz,color);
      
      pthread_mutex_unlock(&thischat->chatmute);     
    }
    else{
      bzero(message,2000);
      sprintf(message,"\nSERVER: %s has left us leaving private sesh with you\n",accountname);
      send(thisacct->privatesock,message,strlen(message),0);
      bzero(message,2000);
      sleep(1);
      sprintf(message,"@rmvsoc %d\n",thisacct->acctsock);
      send(thisacct->privatesock,message,strlen(message),0);

      pthread_mutex_unlock(&thisacct->accountmute);
      pthread_mutex_destroy(&thisacct->accountmute);
      free(thisacct->acctname);
      removesock(globallist,thisacct->acctsock);

      free(thisacct);
      acctslots--;

      delcolor(colorrz,color);

      pthread_mutex_unlock(&thischat->chatmute);

      
    }
    
    
    return 0; 
  }
}


int createaccount(char* accountname,chat* thischat,int socketdesc){
  pthread_mutex_lock(&thischat->chatmute);
  acctslots++;
  if(acctslots>20){
    acctslots--;
    pthread_mutex_unlock(&thischat->chatmute);
    return 1;
  }
  account* new = malloc(sizeof(account));
  new->acctname = malloc(strlen(accountname)+1);
  strcpy(new->acctname,accountname);
  pthread_mutex_init(&new->accountmute,NULL);
  new->acctsock = socketdesc;
  new->privatesock = 0;
  if(get(accountname,thischat->map)==NULL){

    set(accountname,new,thischat->map);
    printf("account: %s created \n",accountname);

  }
  else{
    pthread_mutex_unlock(&thischat->chatmute);
    free(new);
    return 1;
  }
  pthread_mutex_unlock(&thischat->chatmute);

  return 0;
}
account*  accessacct(char* accountname,chat* thischat){

  account* found;
  pthread_mutex_lock(&thischat->chatmute);
  found = get(accountname,thischat->map);
  pthread_mutex_unlock(&thischat->chatmute);
  return found;
}


connectlist* createconnectlist(){
  connectlist* new;
  new = (connectlist*) malloc(sizeof(connectlist));
  new->head = NULL;
  new->connectnum = 0;
  return new;
}
void insertsock(connectlist* list,int sock_desc,char* acctname){
  connectnode* new;
  
  new = (connectnode*) malloc(sizeof(connectnode));
  new->sock_desc = sock_desc;
  new->next = NULL;
  new->acctname = acctname;
  if(list->head == NULL){
    list->head = new;
    list->head->next= NULL;
    list->connectnum++;
  }
  else{
    new->next = list->head;
    list->head = new;
    list->connectnum++;  
  }
  
}
int removesock(connectlist* list, int sock){
  connectnode* ptr;
  connectnode* tmp;
  if (list->connectnum == 0){
    return 1;
  }
  ptr = list->head;
  if(ptr->sock_desc == sock){
    tmp = ptr;
    if(ptr->next == NULL){
      free(ptr);
      list->head = NULL;
      list->connectnum--;
      return 0;
    }
    else{
      list->head = ptr->next;
      free(ptr);
      list->connectnum--;
      return 0;
    }

  }
  while(ptr->next!=NULL){
    if(ptr->next->sock_desc == sock){
      if (ptr->next->next!=NULL){
	tmp = ptr->next;
	ptr->next = ptr->next->next;
	free(tmp);
	list->connectnum--;
	return 0;
      }
      else{
	tmp = ptr->next;
	ptr->next = NULL;
	free(tmp);
	list->connectnum--;
	return 0;
      }
     
    }
    ptr = ptr->next;
  }
  return 1;
}

void sigcHandler(int sig){
  if(sig == SIGINT){
    int sock;
    char *message;
    //Send some messages to the client
    message = "server exit";

    connectnode* ptr;
    ptr = globallist->head;
    while(ptr!=NULL){
      //Get the socket descriptor
      sock = ptr->sock_desc;
      send(sock , message , strlen(message),0);
      close(sock);
      ptr = ptr->next;
    }
    exit(0);
  }
  

}
void broadcastchat(chat* thischat,char* message,char* username){
  
  account* thisacct;
  hashnode* thisnode;
  int i;
  
  pthread_mutex_lock(&thischat->chatmute);
  
  for(i=0;i<thischat->map->size;i++){      //sends chat out to everyone in chat 
    if(thischat->map->table[i]!=NULL){
      thisnode = thischat->map->table[i];
      while(thisnode!=NULL){
	thisacct = thisnode->value;
	if(strcmp(username,thisacct->acctname)!=0){
	  send(thisacct->acctsock,message,strlen(message),0);
	
	}
	thisnode= thisnode->next;
      }
    }
  }
  thischat->chathist = fopen("chathist.txt","a+"); //save in chat history  
  fputs(message,thischat->chathist);
  fputs("\n",thischat->chathist);
  fclose(thischat->chathist);
  
  pthread_mutex_unlock(&thischat->chatmute);
  
}

void broadcastprivate(connectlist* private,char* message){
  int sock;  
  //Send some messages to the client
  
  connectnode* ptr;
  ptr = private->head;
  while(ptr!=NULL){
    //Get the socket descriptor
    sock = ptr->sock_desc;
    send(sock , message , strlen(message),0);
    
    ptr = ptr->next;
  }


}

void *connection_handler(void *socket_desc)
{
  
  //Get the socket descriptor
  int sock = *(int*)socket_desc;
  free(socket_desc);
  int read_size;
  char* user;
  char *message , client_message[2000],buffer[2000];
  char* colorseq;
  char* resetseq;
  CommandList* commando;
  hashnode* thisnode;
  account* thisacct;
  int i;
  connectlist* privatelist;
  int rmvpriv;
  privatelist = createconnectlist();
  //Send some messages to the client
  message = "HEY you've connected to Vamsi's chat!";
  send(sock , message , strlen(message),0);
  message = "\n";
  send(sock , message , strlen(message),0);
     
  read_size = recv(sock , client_message , 2000 , 0);
  client_message[read_size] = '\0';
  
 
  while(createaccount(client_message,mainchat,sock)==1){
    message = "username taken or chat is full to try again retype username";
    send(sock , message , strlen(message)+1,0);
    bzero(client_message,2000);
    read_size = recv(sock , client_message , 2000 , 0);
    client_message[read_size-1] = '\0';
  }
  user = malloc(strlen(client_message)+1);
  strcpy(user,client_message);

  bzero(buffer,2000);

  pthread_mutex_lock(&mainchat->chatmute);  

  colorseq = getcolor(colorrz); //choose output color
  resetseq = "\033[0m";
 
  mainchat->chathist = fopen("chathist.txt","r");
  if(mainchat->chathist!=NULL){                      //send entire chathist
    while(fgets(buffer,2000,mainchat->chathist)!=NULL){
      
      send(sock,buffer,strlen(buffer),0);
      bzero(buffer,2000);
    }
    fclose(mainchat->chathist);
  } 
 
  pthread_mutex_unlock(&mainchat->chatmute);
 
 //Receive a message from client
  while( ((read_size = recv(sock , client_message , 2000 , 0)) > 0) )
    {
      
      
      //end of string marker (take newline char out)
      client_message[read_size-1] = '\0';
      commando = TokenizeString(client_message);
      
      if(commando->count ==1){
	if(strcmp(commando->tail->command->token,"@who")==0){
	    message = "SERVER: you want a list of all the active users";
	    send(sock,message,strlen(message),0);
	    pthread_mutex_lock(&mainchat->chatmute);
	    
	    for(i=0;i<mainchat->map->size;i++){      //send name of actives
	      if(mainchat->map->table[i]!=NULL){
		thisnode = mainchat->map->table[i];
		while(thisnode!=NULL){
		  thisacct = thisnode->value;
		  if(strcmp(user,thisacct->acctname)!=0){
		    send(sock,thisacct->acctname,strlen(thisacct->acctname),0);
		    message = "\n";
		    send(sock,message,strlen(message),0);
		  }
		  thisnode= thisnode->next;
		}
	      }
	    }
	    
	    pthread_mutex_unlock(&mainchat->chatmute);
	    deleteCommandList(commando);
	    free(commando);	    
	    continue;
	  }
	  else if(strcmp(commando->tail->command->token,"@exit")==0){
	    message = "exit";
	    send(sock,message,strlen(message),0);
	    deleteCommandList(commando);
	    free(commando);
	    continue;
	  }
      }
      else if(commando->count ==2){
	if(strcmp(commando->tail->next->command->token,"@private")==0){
	  message = "\nSERVER: you want to start a private session with: ";
	  bzero(buffer,2000);
	  sprintf(buffer,"%s%s",message,commando->tail->command->token);
	  send(sock,buffer,strlen(buffer),0);
	  bzero(buffer,2000);
	  thisacct = accessacct(commando->tail->command->token,mainchat);
	  
	  if(thisacct!=NULL&&strcmp(thisacct->acctname,user)==0){
	    message = "\nSERVER: cannot start private session with self ";
	    bzero(buffer,2000);
	    send(sock,message,strlen(message),0);
	    deleteCommandList(commando);
	    free(commando);
	    continue;
	  }
	  if(thisacct == NULL){
	    message = "account not found try again";
	    send(sock,message,strlen(message),0);
	    deleteCommandList(commando);
	    free(commando);
	    continue;
	  }
	  else{
	    if(pthread_mutex_trylock(&thisacct->accountmute)!=0){
	      message = "\nSERVER: that person is busy, try later";
	      send(sock,message,strlen(message),0);
	      deleteCommandList(commando);
	      free(commando);
	      continue;
	    }
	    else{
	      message = "\nSERVER: private session started with: ";
	      bzero(buffer,2000);
	      sprintf(buffer,"%s%s",message,thisacct->acctname);
	      send(sock,buffer,strlen(buffer),0);
	      bzero(buffer,2000);
	    
	      insertsock(privatelist,thisacct->acctsock,thisacct->acctname);
	      thisacct->privatesock = sock;
	      message = "  has started a private session with you";
	      bzero(buffer,2000);
              sprintf(buffer,"\n%s%s",user,message);
              send(thisacct->acctsock,buffer,strlen(buffer),0);
              bzero(buffer,2000);

	      deleteCommandList(commando);
              free(commando);
              continue;
	    }
	  }

	}
	else if(strcmp(commando->tail->next->command->token,"@end")==0){
	  message = "\nSERVER: you want to end a private session with: ";
          bzero(buffer,2000);
          sprintf(buffer,"%s%s",message,commando->tail->command->token);
          send(sock,buffer,strlen(buffer),0);
          bzero(buffer,2000);
          thisacct = accessacct(commando->tail->command->token,mainchat);
          if(thisacct == NULL){
            message = "account not found try again";
            send(sock,message,strlen(message),0);
            deleteCommandList(commando);
            free(commando);
            continue;
          }
	  else{
	    if(removesock(privatelist, thisacct->acctsock)!=0){
	      message = "\nSERVER: you did not have a private session with: ";
	      bzero(buffer,2000);
	      sprintf(buffer,"%s%s",message,commando->tail->command->token);
	      send(sock,buffer,strlen(buffer),0);
	      bzero(buffer,2000);
	      deleteCommandList(commando);
	      free(commando);
	      continue;

	    }
	    else{
	      pthread_mutex_unlock(&thisacct->accountmute);
	      message = "\nSERVER: you ended  a private session with: ";
              bzero(buffer,2000);
              sprintf(buffer,"%s%s",message,commando->tail->command->token);
              send(sock,buffer,strlen(buffer),0);
              bzero(buffer,2000);

	      sprintf(buffer,"%s has ended a private session with you",user);
	      send(thisacct->acctsock,buffer,strlen(buffer),0);
              bzero(buffer,2000);
	      thisacct->privatesock = 0;
              deleteCommandList(commando);
              free(commando);
              continue;

	    }
	  }


	}
	else if(strcmp(commando->tail->next->command->token,"@rmvsoc")==0){
	  rmvpriv = atoi(commando->tail->command->token);
	  
	  if( removesock(privatelist, rmvpriv)==1){

	    puts("failed to find in privatelist");
	  }
	  continue;
	}	
      }    
    	 
      
      deleteCommandList(commando);
      free(commando);

      message = (char*) malloc(strlen(client_message)+strlen(user)+strlen(colorseq)+strlen(resetseq)+16);
     
      if(privatelist->connectnum > 0){
	sprintf(message,"%s(private) %s: %s %s",colorseq,user,client_message,resetseq);
	broadcastprivate(privatelist,message);
	free(message);
      }
      else{
	sprintf(message,"%s %s: %s %s",colorseq,user,client_message,resetseq);
	broadcastchat(mainchat,message,user);     
	free(message);
      }
    
    }   	
  if(read_size == 0)
    {
      puts("Client disconnected");
      deleteaccount(user,mainchat,colorseq,privatelist);
      
      fflush(stdout);
      
    }
  else if(read_size < 0)
    {
      deleteaccount(user,mainchat,colorseq,privatelist);
      perror("recv failed");
      
    }
     
  return 0;
      
    
} 

       


int main(int argc , char *argv[])
{
  acctslots = 0;
  mainchat = createchat();
  globallist=createconnectlist();
  colorrz = initcolor();
  int socket_desc , client_sock , clilen;
  int* new_sock;
  int on = 1;
  struct sockaddr_in server , client;    
  char ThisHost[80];
    
  
  signal(SIGINT,sigcHandler); 
  
  gethostname(ThisHost, MAXHOSTNAME);
  printf("----TCP/Server running at host NAME: %s\n", ThisHost);
  //Create socket
  
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));


  if (socket_desc == -1)
    {
      puts("Could not create socket");
    }
  puts("Socket created");

  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = 0;
 
  bzero((char *) &server, sizeof(server));   

  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) != 0)
    {
      //print the error message
      perror("bind failed. Error");
      return 1;
    }
  
  puts("bind done");
  
  struct sockaddr_in sin;
  
  socklen_t len = sizeof(sin);
  
  if (getsockname(socket_desc, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname");
  else
    printf("port number %d\n", ntohs(sin.sin_port));   
  
  //Listen  
  listen(socket_desc , 500);
     
  //Accept and incoming connection
  puts("Waiting for incoming connections...");
  clilen = sizeof(struct sockaddr_in);
     
  
  
  while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&clilen)) )
    {
      puts("Connection accepted");
      pthread_t conn_thread;
     
      new_sock = malloc(sizeof(int));
      *new_sock = client_sock;
      insertsock(globallist,client_sock,"Global");

      if(pthread_create( &conn_thread , NULL ,  connection_handler , (void*)new_sock) < 0)
        {
	  perror("could not create thread");
	  return 1;
        }
      
    }
  
  if (client_sock < 0)
    {
      perror("accept failed");
      return 1;
    }
  
  return 0;
}
 
