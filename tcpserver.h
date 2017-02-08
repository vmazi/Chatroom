#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with -pthread
#include<signal.h>
#include "tokenizer.h"
#include "hashtable.h"

#define MAXHOSTNAME 80 

typedef struct color{
  int flag;
  char* sequence;
}color;
typedef struct colortable{
  color** table;
}colortable;
typedef struct account{
  char* acctname;
  int acctsock;
  int privatesock;
  pthread_mutex_t accountmute;
}account;
typedef struct chat{
  hashtable* map;
  FILE* chathist;
  pthread_mutex_t chatmute;
}chat;

typedef struct connectlist{
  struct connectnode* head;
  int connectnum;
}connectlist;

typedef struct connectnode{
  int sock_desc;
  char* acctname;
  struct  connectnode* next;
}connectnode;

chat* mainchat;
connectlist* globallist;
colortable* colorrz;

int acctslots;

color* createcolor(char* seq);
void destroycolor(color* del);
colortable* initcolor();
char* getcolor(colortable* pallet);//chat mutex must be set before using
int delcolor(colortable* pallet,char* seq);//same as above
chat* createchat();
int createaccount(char* accountname,chat* thisbank,int socketdesc);//uses mutex
account* accessacct(char* accountname, chat* thischat);//uses mutex

connectlist* createconnectlist();
void insertsock(connectlist* list,int sock_desc,char* acctname);
void sigcHandler(int sig);
int removesock(connectlist* list,int sock);
void *connection_handler(void *socket_desc);


#endif
