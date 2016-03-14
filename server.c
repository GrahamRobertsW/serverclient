//Graham Roberts
//CS367 Project2
//A simple chat client
//
//
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>

//Structure definitions sorry
//github wreaked a whole new variety of gitty havoc
//and I have been forced to simplify the fixes due to time constraints
//
//Structure USERID
//contains a user, their socket and expandable read and write vectors

typedef struct vector_st{
	char* buffer;
	int messageLength;
	int size;
} vector_t;

vector_t* initVector(){
	char* newBuffer=calloc(1,64);   
	vector_t *newV = calloc(1,sizeof(vector_t));
	newV->buffer=newBuffer;
	newV->size=64;
	newV->messageLength=0;
	return newV;
}

void expandVector(vector_t* vec){
	char* tempBuffer = vec->buffer;
	vec->buffer = calloc(1,2*vec->size);
	strcat(vec->buffer, tempBuffer);
	return;
}

typedef struct user_st{
	char* name;
	vector_t* writeBuffer;
	vector_t* readBuffer;
	struct timeval* time;
	int sock;
} user_t;

user_t* initUser(char* N, int usock){
   user_t *newU = calloc(1,sizeof(user_t));
   newU->name=N;
	newU->writeBuffer=initVector();
	newU->readBuffer=initVector();
	newU->time=calloc(1,sizeof(struct timeval));
	newU->time->tv_sec=5;
	newU->time->tv_usec=0;
//	newU->sock=socket(PF_INET, SOCK_STREAM, 0);
	newU->sock=usock;
	return newU;
}
int hostSocket(struct sockaddr address){
//hostSocket creates a socket and binds it to listen on a port
//takes a sockadr_in i think to do things unless I fucked up and sont need that
	int sock =socket(PF_INET, SOCK_STREAM, 0);
   if (!(bind (sock, (struct sockaddr *) &address, sizeof(address)) >=0)){
		printf("Error something has gone awrye attempting a bind to socket. Have a nice daay.");
		exit(1);
	}
	return sock;
}

int generateHostAddress(int hostPort){
//generates a portt upon which we will listen
//similar to your listenon function except early returns have been removed
//it felt dishonest not to make "improvements"
//because we all know that early return are "bad"
	int val = 0;
   int newSock = socket(PF_INET, SOCK_STREAM, 0);
	if(newSock<0){
		val=-1;
		fprintf(stderr, "unable to generate operable port because (%d) \"%s\"\n", errno, strerror(errno));
	}
	if(newSock>0){
      struct sockaddr_in myAddress;
      memset(&myAddress, 0,sizeof(myAddress));
	   myAddress.sin_family = AF_INET;
	   myAddress.sin_addr.s_addr = INADDR_ANY;
	   myAddress.sin_port = htons(hostPort);
	   if (bind(newSock, (struct sockaddr*)&myAddress, sizeof(myAddress))){
		   fprintf(stderr, "unable to bind because (%d) \"%s\"\n", errno, strerror(errno));
	      close(newSock);
			newSock=-1;
		}
	}
	return(newSock);
}

int initiateConnection(char* address, unsigned short port){
// initialize sets up a connection, and plugs in a socket
    int cleanSock=socket(PF_INET, SOCK_STREAM, 0);
	 while(cleanSock<0){
		 fprintf(stderr, "failed to initialize socket (%d), %s",errno,strerror(errno));
		 cleanSock=socket(PF_INET, SOCK_STREAM, 0);
    }
    struct sockaddr_in socketToYa;
    socketToYa.sin_family = AF_INET;
    socketToYa.sin_port = htons(port);
    inet_aton(address, &socketToYa.sin_addr);
    while(connect(cleanSock,(struct sockaddr*) &socketToYa,sizeof(socketToYa))){
		 fprintf(stderr, "Failed to connect to %s:%d because (%d) \"%s\"\n",address, port, errno, strerror(errno));
		}
    return cleanSock;
}

int setListen(unsigned short inPort, int queueSize){
//setListen
//bundles listen and error statements into one call
	int val=0;
	if (listen(inPort, queueSize)){
      val=-1;
	   fprintf(stderr,"listening failed on account of (%d), \"%s\"",errno,strerror(errno));
	}
	return val;
}

int resolve(char* input, char* userid, char* secret){
int buffsize=strlen(input);
	userid = calloc(1,buffsize);
	secret = calloc(1,buffsize);
	int index=0;
	int nullcount=0;
	while(nullcount<1&index<64){
		userid[index]=input[index];
		if (input[index]=='\0'){
         nullcount=nullcount+1;
		}
		index=index+1;
	}
	if(nullcount<1){
		return nullcount;
	}
	int si = index;
	while(index<buffsize){
		secret[index-si]=input[index];
		if (input[index]=='\0'){
			nullcount=nullcount+1;
	   }
	}
   return nullcount;
}

//int authenticate(char* username, char* password){
//	return 1;
//}

user_t* newUser(char* username, char* password, int host){
	dprintf(1,"new user %s\n",username);
	user_t* newguy=initUser(username, host);
	return newguy;
}

void disconnect(user_t* loser){
	close(loser->sock);
	free(loser->writeBuffer);
	free(loser->readBuffer);
	free(loser);
	return;
}

int settowrite(char* message, user_t* recipient){
	recipient->writeBuffer->messageLength=recipient->writeBuffer->messageLength+strlen(message);
	if (recipient->writeBuffer->messageLength>recipient->writeBuffer->size){
		expandVector(recipient->writeBuffer);
	}
	int nullcount=0;
   for (int i=0; i<recipient->writeBuffer->messageLength; i++){
      if (recipient->writeBuffer->buffer[i]=='\0'){
			nullcount++;
		}
	}
   if(nullcount>5){
		return -1;
	}
return 0;
}
void find2messages(int inport, vector_t* id, vector_t* secret){
	vector_t *inbuff=initVector();
	inbuff->messageLength=read(inport,inbuff->buffer,inbuff->size);
	int nulls=0;
	int i=0;
	while(nulls<1){
		if (inbuff->buffer[i]=='\0'){
			nulls++;
		}
	   i++;
	}
	dprintf(1,"here comes the weird stuff \n");
	strcat(id->buffer, inbuff->buffer);
	char atnum=inbuff->buffer[i];
	strcat(secret->buffer,&atnum);
	dprintf(1,"found 2 messages");
	return;
}

//void find2messages(int inport, char* id, char* secret){
//	char* tempBuffer = calloc(1,64);
//	char* inBuffer = calloc(1,64);
 //  read(inport, inBuffer, 64);
//	int nulls=resolve(tempBuffer, id, secret);
//	while (nulls<1){
//		tempBuffer =calloc(0,sizeof(id)+64);
 //     strcat(tempBuffer, id);
//		id = tempBuffer;
//		tempBuffer = calloc(1,64);
//		inBuffer = calloc(1,64);
//		read(inport, tempBuffer, 64);
//		nulls=nulls+resolve(inBuffer, tempBuffer, secret);
//      strcat(id, tempBuffer);
 //  }
//	while (nulls<2){
//		tempBuffer = calloc(1,sizeof(secret)+64);
//		strcat(tempBuffer, secret);
//		secret = tempBuffer;
 //     tempBuffer = calloc(1,64);
//		read(inport, inBuffer, 64);
//	   nulls=nulls+resolve(inBuffer, tempBuffer, NULL);
//		strcat(secret, tempBuffer);
//	}
 //  return;
//}

void insertIntoUsers(user_t* entry, user_t **array){
	user_t **newArray= calloc(1,sizeof(array)+sizeof(entry));
	int i=0;
	int max = (int)(sizeof(array)/sizeof(user_t*));
	while(i<max){
		newArray[i]=array[i];
		i++;
	}
	newArray[i]=entry;
}

void newFriend(user_t** friends, int inport){
	vector_t* id = initVector();
	vector_t* secret = initVector();
	find2messages(inport, id, secret);
	dprintf(1,"Message1 %s\n",id->buffer);
	dprintf(1,"Message1 %s\n", secret->buffer);
	user_t* friend=newUser(id->buffer, secret->buffer, inport);
   insertIntoUsers(friend, friends);
	return;
}

void subtracTime(struct timeval *initial, struct timeval *delta){
   int init = 1000*initial->tv_sec+initial->tv_usec;
	int del = 1000*delta->tv_sec+delta->tv_usec;
	int diff = init-del;
	if (diff<=0){
		initial->tv_sec=0;
		initial->tv_usec=0;
	}
   else{
		initial->tv_sec=(time_t)(int)(diff/1000);
      initial->tv_usec=(suseconds_t)(int)(diff%1000);
	}
   return;
}

int timeComp(struct timeval *A, struct timeval *B){
	int a = 1000*A->tv_sec+A->tv_usec;
	int b = 1000*B->tv_sec+B->tv_usec;
	if(a>b){
		return(1);
	}
	else{
		return(0);
	}
}

void reseTimer(struct timeval* timer){
	timer->tv_sec=5;
	timer->tv_usec=0;
}
void readfrom(user_t* auser){
   int num=read(auser->sock, auser->readBuffer->buffer, auser->readBuffer->size-auser->readBuffer->messageLength);
   if (auser->readBuffer->messageLength+num==auser->readBuffer->size){
      expandVector(auser->readBuffer);
      fd_set thisguy;
      FD_ZERO(&thisguy);
      FD_SET(auser->sock,&thisguy);
      struct timeval* instant=calloc(1,sizeof(struct timeval));
      instant->tv_sec=0;
      instant->tv_usec=100;
      if(select(auser->sock+1, &thisguy, NULL, NULL, instant)){
         readfrom(auser);
      }
      return; 
   }
}

void writeto(user_t* auser){
   int num = write(auser->sock, auser->writeBuffer->buffer, auser->writeBuffer->messageLength);
	int i=0;
	while(i<auser->writeBuffer->messageLength-num){
		auser->writeBuffer->buffer[i]=auser->writeBuffer->buffer[i+num];
	}
	char atnum = auser->writeBuffer->buffer[num];
	memset(&atnum, 0, auser->writeBuffer->size-num);
	
}

void passMessage(user_t* recipient, vector_t* message, user_t* sender){
	vector_t* newM = initVector();
	while((int)strlen(sender->name)+message->messageLength>newM->size){
		expandVector(newM);
	}
//nt nulls=0;
	int i=0;
   strcat(newM->buffer,sender->name);
	newM->messageLength=strlen(sender->name)+1;
	char atnum = newM->buffer[newM->messageLength];
	strcat(&atnum, message->buffer);
	newM->messageLength=newM->messageLength+message->messageLength+1;
	while(newM->messageLength+recipient->writeBuffer->messageLength>recipient->writeBuffer->size){
		expandVector(recipient->writeBuffer);
	}
	atnum=recipient->writeBuffer->buffer[recipient->writeBuffer->messageLength];
	while(i<newM->messageLength){
		recipient->writeBuffer->buffer[recipient->writeBuffer->messageLength]=newM->buffer[i];
		recipient->writeBuffer->messageLength++;
		i++;
	}
}

void broadcast(vector_t* mess, user_t* sender, user_t** users){
   int i=0;
	while(i<sizeof(users)/sizeof(user_t)){
		if(strcmp(users[i]->name,sender->name)){
			passMessage(users[i], mess, sender);
		}
	}
}

void read2write(user_t** users){
	int i;
	int nulls;
	int j=0;
	int k=0;
	while (j<sizeof(users)/sizeof(user_t)){
		user_t* user=users[j];
		char* message = user->readBuffer->buffer;
		vector_t* id=initVector();
		vector_t* mess=initVector();
		nulls=0;
		i=0;
		while(nulls<1&&i<sizeof(message)){
		   id->buffer[i]=message[i];
		   id->messageLength++;
		   if(id->messageLength==id->size){
			   expandVector(id);
		   }
		   if(message[i]=='\0'){
			   nulls++;
		   }
		   i++;
	   }
	   while (nulls<2&&i<sizeof(message)){
         mess->buffer[mess->messageLength]=message[i];
			mess->messageLength++;
			if(mess->messageLength==mess->size){
				expandVector(mess);
			}
			if(mess->buffer[i]=='\0'){
				nulls++;
			}
			i++;
		}
      if(id=='\0'&&mess->buffer[0]!='\0'){
			broadcast(mess, user, users);
		}	
		else{
			k=0;
			while(k<sizeof(users)/sizeof(user_t)){
				if(!(strcmp(id->buffer, users[k]->name))){
					passMessage(users[k], mess, user);
				}
			}
		}
	}
}

void eventLoop(user_t** users, int inport){
   fd_set set4read;
	fd_set set4write;
	FD_ZERO(&set4read);
	FD_ZERO(&set4write);
	struct timeval* min = calloc(1,sizeof(struct timeval));
	reseTimer(min);
	int max=inport;
	printf("%d", max);
	for (int i=0; i<sizeof(users)/sizeof(user_t*); i++){
		if(timeComp(min, users[i]->time)){
			min=users[i]->time;
		}
		if(users[i]->writeBuffer->messageLength>0){
			FD_SET(users[i]->sock,&set4write);
	   	dprintf(1,"\nAdding %s: socket: %d\n", users[i]->name, users[i]->sock);
		}
		FD_SET(users[i]->sock,&set4read);	
		dprintf(1,"\nAdding %s: socket: %d\n", users[i]->name, users[i]->sock);
		if(max<users[i]->sock){
			max=users[i]->sock;
		}
	}
	FD_SET(inport, &set4read);
	struct timeval* interval = calloc(1,sizeof(struct timeval));
	interval->tv_sec=min->tv_sec;
	interval->tv_usec=min->tv_usec;
	dprintf(1,"entering select statement with timeval %d",(int)min->tv_sec);
   int sel = select(max+1,&set4read,&set4write,NULL,min);
	dprintf(1, "select: %d", sel);
	subtracTime(interval,min);
	for(int i=0; i<(int)(sizeof(users)/sizeof(user_t*)); i++){
		if(FD_ISSET(users[i]->sock, &set4read)){
			readfrom(users[i]);
			reseTimer(users[i]->time);
			dprintf(1,"reading on socket: %d name: %s\n",users[i]->sock, users[i]->name);
		}
		if(FD_ISSET(users[i]->sock, &set4write)&&users[i]->writeBuffer->messageLength>0){
			dprintf(1,"writing to %s\n",users[i]->name);
			writeto(users[i]);
			reseTimer(users[i]->time);
	   }
	}
	if(FD_ISSET(inport, &set4read)){
		struct sockaddr_in addr;
	   socklen_t len = sizeof(addr);
	   memset(&addr,0, sizeof(addr));
	   int client = accept(inport, (struct sockaddr*)&addr, &len);
	   if(client<0){
		   fprintf(stderr, "Failed to initialize a new user because (%d) \"%s\"", errno, strerror(errno));
		   return;
		}
		write(client,"OK\0\0",4);
		newFriend(users, client);
	}
	printf(".");
	return;
}
int main(){
   int serverHost=generateHostAddress(63956);
	dprintf(1,"%d", serverHost);
   setListen(serverHost,7);
   dprintf(1,"%d", serverHost);
   user_t** users= NULL;
   dprintf(1,"entering Loop\n");
   dprintf(1,"looping\n");
   struct timeval* timeout=calloc(1,sizeof(struct timeval));
	dprintf(1,"timeval made\n");
//	timeout->tv_sec=6;
	dprintf(1,"seconds set\n");
//	timeout->tv_usec=0;
	dprintf(1,"milliseconds set\n");
	fd_set input;
	dprintf(1,"fd_set initialized");
	FD_ZERO(&input);
	dprintf(1,"zeroed");
	FD_SET(serverHost, &input);
	dprintf(1,"what the goddamn LITERAL FUCK HILY SHIT FUCK FUCK FUCK FUCK");
	while(users==NULL){
	   while(!select(serverHost+1,&input, NULL, NULL, timeout)){
		   timeout->tv_sec=6;
		   timeout->tv_usec=0;
      	FD_ZERO(&input);
      	FD_SET(serverHost, &input);
			dprintf(1,"timeout %d", select(serverHost+1,&input, NULL, NULL, timeout));
			timeout->tv_sec=6;
		   timeout->tv_usec=0;
      	FD_ZERO(&input);
      	FD_SET(serverHost, &input);
	   }
	   struct sockaddr_in addr;
	   socklen_t len = sizeof(addr);
	   bzero(&addr, len);
	   int client = accept(serverHost, (struct sockaddr*)&addr, &len);
	   if(client<0){
		   dprintf(2, "Failed to initialize a new user because (%d) \"%s\"", errno, strerror(errno));
      }
		else{
			dprintf(1,"new connection on socket %d", client);
			write(client,"OK\0\0",4);
			dprintf(1,"\nbeep\n");
			vector_t* id = initVector();
			vector_t* secret = initVector();
	      find2messages(client, id, secret);
			dprintf(1,"\n%s\n",id->buffer);
	      user_t* friend=newUser(id->buffer, secret->buffer, client);
			dprintf(1,"His/her name is %s\n", friend->name);
			users=calloc(1,sizeof(user_t*));
			dprintf(1,"collocing %u\n",sizeof(user_t));
		        users[0]=friend;
			dprintf(1, "sizeof(friend)=%u\n", sizeof(friend));
			dprintf(1,"arry made\n");
	   }	
   }
	while (users!=NULL){
		dprintf(1,"beginning event loop");
		eventLoop(users, serverHost);
	}
	return 0;
}

