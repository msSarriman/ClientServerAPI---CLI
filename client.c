#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#define _buffer_length 1024
#define _buffer_read (_buffer_length + 1)

void signalHandlerFunction();
void readFunction();
ssize_t writen(const void*,size_t);

char buffer[_buffer_length];

int mySocket;
char* stringToSend=(char) 0;
int lengthToSend;
int transmitFlag=0;

int main(int argc, char* argv[]){
	int i;	
	//signal(SIGINT, signalHandlerFunction);
// ************************************************************************************************** //
// ************************ port integrity check code START****************************************** //
// ************************************************************************************************** //
	#ifdef _debug_port
		for (i=1; i<argc; i++)
		{
			printf("DEBUG LINE: The argv<%d> is: %s \n", i,argv[i]);
		}
	#endif	
	for (i=0; i<strlen(argv[2]); i++)
	{
		#ifdef _debug_port
			printf("DEBUG LINE: Number of for loop is <%d>\n", i);
			printf("DEBUG LINE: argv[2][<%d>] element is <%c>\n", i, argv[2][i]);
		#endif
		if ( !isdigit(argv[2][i]) )
		{
			printf("Port is not ok\n");
			return 1;
		}
	}	
// *************************************************************************************************** //
// ************************************port integrity check code END********************************** //
// *************************************************************************************************** //

// *************************************************************************************************** //	
// ************************** `put` and `get` com. integrity check code START ************************ //
// *************************************************************************************************** //
#ifdef _dontdebugthis
	#ifdef _debug_integrity
		printf("Checkpoint black argv3<%s>\n",argv[3]);
		printf("test\n");
	#endif	
	if (argv[3] == NULL)
	{
		return 0;
	}else
	{
		i=3;
		while(1)
		{
			if (argv[i] == NULL)
			{
				#ifdef _debug_integrity
					printf("Checkpoint 3 i<%d>\n",i);
				#endif
				break;
			}
			else if (strcmp(argv[i],"get")==0)
			{
				i = i + 2;
				#ifdef _debug_integrity	
					printf("Checkpoint 2 i<%d>\n",i);
				#endif
			}
			else if (strcmp(argv[i],"put")==0)
			{
				i = i + 3;
				#ifdef _debug_integrity
					printf("Checkpoint 1 i<%d>\n",i);
				#endif
			}
			else
			{
				#ifdef _debug_integrity
					printf("Checkpoint 4 i<%d>\n",i);
				#endif
				printf("There is an error in given order syntax\n");
				return 0;
				break;
			}
		#ifdef _debug_integrity
			printf("DEBUG LINE: argv[%d]:<%s>\n", i, argv[i]);
			printf("DEBUG LINE: argv[%d]:<%s>\n", i-1, argv[i-1]);
			printf("DEBUG LINE: argv[%d]:<%s>\n", i-2, argv[i-2]);
		#endif
		}
	} 
#endif
// *************************************************************************************************** //
// ************************** `put` and `get` com. integrity check code END ************************** //
// *************************************************************************************************** //

// *************************************************************************************************** // 
// ***************************************** Connection code START *********************************** //
// ********************** socket -> connect -> write -> read -> close ******************************** //
// *************************************************************************************************** // 
	mySocket = socket(AF_INET, SOCK_STREAM, 0);  // Socket creation
	if(mySocket == -1) 							 // Socket Creation Error Handling
	{
		perror("Error creating socket. Details follow.\n");
		return -1;
	}

	int gai;																// Integer for Get Address Info (what gai stand for)
	struct addrinfo hints, *serverinfo, *p; 								// Struct for getaddrinfo parameters //
	memset(&hints, 0, sizeof(hints));       								// {******************************** //
	hints.ai_family = AF_UNSPEC;		    								//       Used for getaddrinfo()      //
	hints.ai_socktype = SOCK_STREAM;        								// ********************************} //
	gai = getaddrinfo(argv[1],argv[2],&hints,&serverinfo);   				// 
	if (gai != 0) perror("Error getting adress info. Details follow.\n");   // Error Handling Line for Get Addr Info
	for (p=serverinfo; p!=NULL; p=p->ai_next)								// Various Error Handling conditions
	{
		if ((mySocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("Socket Error. Details follow.\n");
			continue;
		}
		if (connect(mySocket, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("Connect Error. Details follow.\n");
			continue;
		}
		break;
	}
	freeaddrinfo(serverinfo);

	i=3;
	do{
		if (strcmp(argv[i],"get")==0)
		{
			if (stringToSend && stringToSend[0])
			{
				bzero(stringToSend,strlen(stringToSend));
				free(stringToSend);
				stringToSend=(char) 0;
			}	
			stringToSend = (char *)malloc(sizeof(char)*(1+strlen(argv[i+1]))+1); //Frist one refers to first byte to send. Plus 1 refers to one the null terminator
			stringToSend[0]='g';
			for(int k=1; k<=(strlen(argv[i+1])+1); k++)
			{
				if(k<=strlen(argv[i+1]))
					stringToSend[k]=argv[i+1][k-1];
				else if(k==strlen(argv[i+1])+1)
					stringToSend[k]=(char)0;
			}

			#ifdef _debug
				printf("MAIN: get: string to send is:\n");
				for(int k=0; k<=(strlen(argv[i+1])+1); k++)
				{
					printf("char '%c' ascii '%d'\n",stringToSend[k],stringToSend[k]);
				}
			#endif		
			/*int eh = send(mySocket, stringToSend, strlen(stringToSend)+1, 0); // Aka ErrorHandling
			if (eh < 0) perror("ERROR writing to socket:");*/
			
			ssize_t writenVar = writen(stringToSend,strlen(stringToSend)+1);

			free(stringToSend);
			stringToSend=(char)0;
			readFunction();
			i = i + 2;
		}
		else if (strcmp(argv[i],"put")==0)
		{
			if (stringToSend && stringToSend[0])
			{
				free(stringToSend);
				stringToSend=(char) 0;
			}
			stringToSend = (char *)malloc(sizeof(char)*(3+strlen(argv[i+1])+strlen(argv[i+2]))); //plus 2 refers to one 'space' and the null terminator
			stringToSend[0]='p';
			int hv=0;
			for(int k=1; k<=(strlen(argv[i+1])+strlen(argv[i+2])+2); k++)
			{
				if(k<=strlen(argv[i+1]))
					stringToSend[k]=argv[i+1][k-1];
				else if(k==strlen(argv[i+1])+1)
					stringToSend[k]=(char)0;
				else if(k>strlen(argv[i+1])+1 && k<=(1+strlen(argv[i+1])+strlen(argv[i+2])))
					stringToSend[k]=argv[i+2][hv++];
				else if(k==strlen(argv[i+1])+strlen(argv[i+2])+2)
					stringToSend[k]=(char)0;
			}
			#ifdef _debug
				printf("MAIN: put: string to send is:\n");
				for(int k=0; k<(strlen(argv[i+1])+strlen(argv[i+2])+3); k++)
				{
					printf("char '%c' ascii '%d'\n",stringToSend[k],stringToSend[k]);
				}
			#endif

			ssize_t writenVar = writen(stringToSend,strlen(argv[i+1])+strlen(argv[i+2])+3);

			/*int eh = send(mySocket, stringToSend, strlen(argv[i+1])+strlen(argv[i+2])+3, 0); // Aka ErrorHandling
			if (eh < 0) perror("ERROR writing to socket:");*/
			free(stringToSend);
			stringToSend=(char)0;
			i = i + 3;
		}
		else break;
		free(stringToSend);
		stringToSend=(char)0;
	}while(i<argc);


	if (close(mySocket) != 0)   							//Socket Shutdown
	{
		perror("Error closing the socket. Details follow.\n");			//Socket Close if Shutdown Fails
		return -1;
	}

// ************************************************************************************************ // 
// **************************************** Connection code END *********************************** //
// ************************************************************************************************ //
	return 0;
}

ssize_t writen(const void *vptr, size_t n){
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;
	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = send(mySocket, ptr, nleft,0)) <= 0 ) {
			if (errno == EINTR)
				nwritten = 0; /* and call write() again */
			else
				return -1; /* error */
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}

void readFunction(){
	bzero(buffer,_buffer_read);
	int readVar = recv(mySocket,buffer,_buffer_read,0);
	if (readVar < 0) perror("ERROR reading from socket:");
	if (buffer[0]=='n')
	{
		printf("\n");
		return;
	}
	if (buffer[0]!='f' && buffer[0]!='n')
	{
		printf("RF: Protocol Error\n");
		close(mySocket);
		exit(0);
	}
	if (buffer[0]=='f' && buffer[1]==(char)0)
	{
		printf("RF: Protocol Error\n");
		close(mySocket);
		exit(0);
	}	
/*	if (buffer[0]=='n' && buffer[1]!=(char)0)
	{
		printf("RF: Protocol Error\n");
		close(mySocket);
		exit(0);
	}*/
	char* matchingValues = (char*)malloc(sizeof(char)*strlen(buffer)+1);
	memcpy(matchingValues,buffer,readVar);
	int i=1;
	while(matchingValues && matchingValues[i])
	{
		printf("%c", matchingValues[i]);
		if(matchingValues[++i]==(char)0) printf("\n");
	}
	#ifdef _debug
	for(int k=0; k<=readVar; k++)	
		printf("\n<%d>", matchingValues[k]);
	#endif
	free(matchingValues);
}

void signalHandlerFunction(){										// If ctrl+c is pressed.
	if (close(mySocket) != 0) perror("Error closing the socket:");
	exit(1);
}