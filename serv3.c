/*************************************************
THIS IS A A SIMPLE TCP Server IMPLEMENTATION
CREATED BY Sarris Emmanuel, for University of Patras
and Computer Informatics and Engineer Department
at the laboratory of 'Network Systems'.
 
This server is running forever (while 1 loop).
It has signalHandler implemented to free the memory 
and the Port that will use, if stopped.
 
This server is handling put and get requests (see client).
 
Examples of a client using this server will provide the following
terminal output:
./client localhost 5432 put city Athens get city put country Greece get country
Athens
Greece
./client localhost 5432 put city NewYork get city put country Italy get country put city London get city
NewYork
Italy
London
========================================
This server must be executed as follows:
./server <port>
----------------
To enable debug stdouts of server at runtime, compile the code with -D_debug flag, p.e:
gcc -D_debug server.c -o server
----------------
To enable debug stdouts of keyvalue.h header at runtime, compile the code with -D_debugheader flag, p.e.:
gcc -D_debugheader server.c -o server
Of course both of the above can be combined:
gcc -D_debugheader -D_debug server.c -o server
**************************************************/
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#define _serv2SHM
#include "keyvalue.h"
#define _BUFFER_SIZE 2048

int readAndWriteFunc();
int socketFunction();
int bindFunction(char*);
int putFunctionCall(char*);
void tokenCleanerFunction(char*);
void signalHandlerFunction();
void initFunc();

char buffer[_BUFFER_SIZE];
int mySocket, clientSocket, portNumber, bindVar;
int breakVar;
struct sockaddr_in serverAddr;
int dc; //debug counter
int k=0; //debug counter
int putComLength;
int temp;   
int listenVar;
struct sockaddr_in cli_addr;                                                            // Init/ing a socaddr struct to use in accept().
int addrlen, breakCond=0;                                                               // addrlen var is going to be used in accpet(). Brea

struct sembuf up = {0, 1, 0};
struct sembuf down = {0, -1, 0};
int mySemaphore;                      //Initialization of the semaphore
/*=====================================================================================*/
/*====== main() =======================================================================*/
/*=====================================================================================*/
int main(int argc, char* argv[]){
    if (argc < 3)                                                // If arguments are less than two, then server's execution has not be done well.
    {
        printf("Please provide valid parameters.\n");
        return 0;                                               // ...and the programm will exit.
    }
    #ifdef _debug
        clock_t tic = clock();                                  // tic var of time.h library. Used to calculate execution times. (only the time for the first aproach of accept() is legit)
    #endif
    signal(SIGINT, signalHandlerFunction);                      // Signal Handler Implementation to free the memory and the socket that this server will use when stopped.
    signal(SIGCHLD, SIG_IGN);                                   // Catching SIGCHLD and ignoring it. Avoiding zombiesssss

    if(socketFunction()==-1) return -1;                         // Create a socket.
    if(bindFunction(argv[1])==-1) return -1;                    // Bind the socket created.
 

    mySemaphore = semget(IPC_PRIVATE, 1, 0666);                 //Initialization of the semaphore
    semctl(mySemaphore, 0, SETVAL, 1);                          //Unlocking the semaphore
    if(mySemaphore<0){perror("MAIN: semaphore error"); exit(1);}



    shmidStruct=shmget(IPC_PRIVATE, 1500*sizeof(keyvalue), IPC_CREAT | 0666);
    if(shmidStruct<0)
    {
        perror("shmget");
        exit(1); 
    }

    kvptr=shmat(shmidStruct, NULL, 0);
    if(kvptr==(void*)-1){
        perror("MAIN: kvptr error"); 
        exit(1);
    }

    shmidTableRow=shmget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    if(shmidTableRow<0)
    {
        perror("shmget");
        exit(1); 
    }
    tableRow=shmat(shmidTableRow, NULL, 0);
    if(tableRow==(void*)-1){
        perror("MAIN: kvptr error"); 
        exit(1);
    }

    listenVar       = listen(mySocket,10);                                                  // Making client socket a server socket.
    if (listenVar != 0) perror("Error Listening. Details follow\n");                        // Error init/ing listen.
    addrlen         = sizeof(cli_addr); 
    *tableRow=0;
    for(int k=0; k<atoi(argv[2]); k++)
    {
        pid_t pid=fork();
        if (pid==-1)
            perror("MAIN: Error Forking");
        if (pid==0) // children code
        {
            printf("MAIN: I am a child with pid <%d>\n",getpid());
            //if((close(listenVar))!=0)
            //    perror("MAIN:KID Error closing listenVar");
            while(1)
            {
                clientSocket = accept(mySocket, (struct sockaddr *) &cli_addr, &addrlen);
                if (clientSocket==-1)
                    perror("MAIN: Error accepting");
                initFunc();
                printf("I am waiting for the next dude <%d>\n", getpid());
                printf("I am waiting for the next dude <%d>\n", getpid());
                printf("I am waiting for the next dude <%d>\n", getpid());
                printf("I am waiting for the next dude <%d>\n", getpid());
                close(clientSocket);
            }
        }
    }
    while (waitpid(-1, NULL, 0) > 0);
    close(mySocket);
    return 0;
}
 
/*=====================================================================================*/
/*=========================== socketFunction() -> used to create a socket =============*/
/*=====================================================================================*/
int socketFunction(){
    mySocket = socket(AF_INET, SOCK_STREAM, 0);  // Socket creation.
    if(mySocket == -1)                           // Socket Creation Error Handling.
    {
        perror("Error creating socket. Details follow.\n");
        return -1;
    }
    if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    return 0;
}
 
/*=====================================================================================*/
/*============ bindFunction() -> used to bind a socket to a port ======================*/
/*=====================================================================================*/
int bindFunction(char* portStdin){
    bzero((char *) &serverAddr, sizeof(serverAddr));            // Initialiazing socaddr_in.serverAddr struct.
    portNumber                  =   atoi(portStdin);            // String to Integer convertion of port given from stdin.
    serverAddr.sin_family       =   AF_INET;                    // Family protocol is INTERNET.
    serverAddr.sin_port         =   htons(portNumber);          // <expression>=BigEndian - LittleEndian transmission (Host to Network SHORT).
    serverAddr.sin_addr.s_addr  =   INADDR_ANY;                 // Address the socket is going to bind onto.
    bindVar                     =   bind(mySocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)); // Here the socket is trying to bind()
    if (bindVar < 0)
    {
        perror("Error binding socket. Details follow.\n");      // Error Handling for bind()
        return -1;
    }
    return 0;
}

/*=====================================================================================*/
/*=========================== socketFunction() -> used to create a socket =============*/
/*=====================================================================================*/
void initFunc(){
    //sleep(3); //enable it for more help in debug.
    if (clientSocket == 0)
    {
        perror("Error when client tried connection.Details follow. \n");                // Error Handling on Dedicated Socket
    }
    else
    {
        printf("Connection from <%s> is successful!\n", inet_ntoa(cli_addr.sin_addr));    // Message for successful connection
    }
    #ifdef _debug
        printf("MAIN: breakcondvar is:<%d>\n", breakCond);
        printf("MAIN: K is k<%d>\n", k++);
    #endif
    breakCond = readAndWriteFunc();                     // readAndWriteFunc() call. Every data trasmission and protocol integrity checking, is taking place inside that function. Return non zero integer, when something is wrong.
    #ifdef _debug
        printf("MAIN: breakCond: <%d>\n", breakCond);
    #endif
    if (breakCond == 0)                                 // if readAndWriteFunc() executed successfully.
    {
        #ifdef _debug
            printf("MAIN: a dragon is in here\n"); 
        #endif
        if (close(clientSocket) != 0) perror("Error closing the socket:");
    }
    #ifdef _debug
        printf("MAIN: a dragon and a sleep(10) is out here\n"); 
        //sleep(10);
        printf("MAIN: ok i slept\n"); 
    #endif
}
 
 
/*=====================================================================================*/
/*===== sig_child() -> used to avoid zombie child processes. ==========================*/
/*=====================================================================================*/
/*void sig_child(int x){
    while(waitpid(0, NULL, WNOHANG) > 0){
        printf("what the hell is going on?????????????????????????????\n");
    }
    signal(SIGCHLD,sig_chld);
}   */
 
 
/*=====================================================================================*/
/*=====readAndWriteFunc() -> used to manipulate data transactions =====================*/
/*=====================================================================================*/
int readAndWriteFunc(){
    int readVar;                // readVar is used to store temporary the recieved buffer, for manipulation.
    char* tempKeyVar=(char) 0;  // tempKeyVar is used to store the <key> that will be used to call header's functions
    char* tempValVar=(char) 0;  // tempValVar is used to store the <Value> that will be used to call header's functions
    while(1)
    {   
        breakVar=0;
        #ifdef _debug
            printf("\n\n\nServ_RF: breakVar is <%d>\n", breakVar);
        #endif
        #ifdef _debug
            printf("Serv_RF: c h e c k p o i n t\n");
        #endif
        if (buffer && buffer[0])                                // if there is content inside buffer clear them                                                 
        {
            bzero(buffer,_BUFFER_SIZE);                                                                     
        }                                                                                           
        if (tempValVar && tempValVar[0])                        // if there is content inside tempValVar clear them                             
        {
            bzero(tempValVar,strlen(tempValVar));
        }
        if (tempKeyVar && tempKeyVar[0])                        // if there is content inside tempKeyVar clear them     
        {
            bzero(tempKeyVar,strlen(tempKeyVar));
        }
        readVar = recv(clientSocket, buffer, _BUFFER_SIZE, 0);  // Reading from the socket of clientSocket var.
        #ifdef _debug
            printf("Serv_RF: c h e c k p o i n t\n");
        #endif      
        if(readVar < 0) perror("Error reading from Socket\n");   // Error Reading from the socket.
        #ifdef _debug
            printf("Serv_RF: buffer is <");
            for(int k=0; k<readVar; k++)
                printf("'%c'",buffer[k]);       
            printf(">\n");
        #endif
        if(!(buffer && buffer[0])) return 0;                    // if buffer is empty then terminate this function.
        #ifdef _debug
            for(int j=0; j<readVar; j++)
                printf("Serv_RF: b u f f e r position<%d> is char <%c> and ASCII <%d>\n",j, buffer[j],buffer[j]);
            printf("Serv_RF: readVar is <%d>\n", readVar);
        #endif
        int flagBreak, sc=0;                                    // variable used for <key> <value> creation into specific strings.
        for(int j=0; j<readVar;)
        {
            #ifdef _debug
                printf("Serv_RF: buffer[j]is<%c> buffer[1]is<%c> buffer[2]is<%c> buffer[3]is<%c> buffer[4]is<%c>\n",buffer[j],buffer[1],buffer[2],buffer[3],buffer[4]);
            #endif
            flagBreak=0;                                        // initialize flag. Flag var is used for checkpoints.
            if(buffer[j]==103 && buffer[j+1]!=(char)0)          // if the letter of buffer[j] is 'g' (ASCII -> 103), and it is followed by a non Null char (because of protocol's reqs)
            {
                #ifdef _debug
                    printf("Serv_RF: GET outside while -> buffer[%d] is <%c>",j,buffer[j]);
                #endif
                j++;                                            // increment the counter once, to point at the first char of <key>
                sc=0;                                           // initialize sc (simple counter). Simple counter is going to be used for <key> creation.
                while(1)
                {
                    if (sc==0) tempKeyVar = (char*)malloc(sizeof(char)+1);                      // Allocate space for tempKeyVar. If its the first character use malloc.
                    if (sc!=0) tempKeyVar = (char*)realloc(tempKeyVar,sizeof(char)*(sc+2));     // Allocate space for tempValVar. If its not the first character use realloc.
                    tempKeyVar[sc++] = buffer[j];                                               // Store j-th char of buffer at sc-th position of tempKeyVar
                    j++;                                                                        // point at next charcter of buffer
                    #ifdef _debug
                            printf("Serv_RF: GET inSide while -> buffer[%d] was <%c> or ASCII <%d> and tempKeyVar[%d] became <%c>\n",(j-1),buffer[j-1],buffer[j-1],sc-1,tempKeyVar[sc-1] );
                            printf("Serv_RF: GET inSide while -> now j is <%d> and sc is <%d>\n",j,sc);
                            printf("Serv_RF: GET inSide while -> and IF statement\n");
                    #endif
                    if(buffer[j]==(char) 0)                                                     // if the next character is null, then the hole key was found.
                    {
                        #ifdef _debug
                            printf("Serv_RF: GET if condition is T R U E");
                        #endif
                        tempKeyVar[sc]=(char)0;                                                 // null terminate the tempKeyVar string. Its ANSI C here...
                        break;
                    }
                }
                j++;    // make J point at the next character of the buffer (if that doesn't exists, the above for() will stop running)
            }
            else if(buffer[j]==112 && buffer[j+1]!=(char)0) // if the letter of buffer[j] is 'p' (ASCII -> 112), and it is followed by a non Null char (because of protocol's reqs)
            {
                #ifdef _debug
                        printf("Serv_RF: PUT outSide while -> buffer[j] is <%c> or ASCII <%d>\n",buffer[j],buffer[j] );
                #endif
                j++;                        // increment j to point at first char of <key>
                sc=0;                       // initialize sc (simple counter). SC is going to be used to for tempKeyVar and tempValVar creation.
                flagBreak=0;                // initialize checkpoint checker var, flagBreak.
                while(flagBreak!=2)
                {
                    #ifdef _debug
                        printf("Serv_RF: PUT before -> buffer[j] is <%c> or ASCII <%d>\n",buffer[j],buffer[j] );
                    #endif
                    if(flagBreak==0)        //  key is beeing manipulated
                    {
                        #ifdef _debug
                            printf("Serv_RF: PUT flagBreak 0 -> buffer[j] is <%c> or ASCII <%d>\n",buffer[j],buffer[j] );
                        #endif
                        if (sc==0) tempKeyVar = (char*)malloc(sizeof(char)+1);                  // Allocate space for tempValVar. If its the first character use malloc.
                        if (sc!=0) tempKeyVar = (char*)realloc(tempKeyVar,sizeof(char)*(sc+2)); // Allocate space for tempValVar. If its not the first character use realloc.
                        tempKeyVar[sc++] = buffer[j];                                           // Store j-th char of buffer at sc-th position of tempKeyVar
                        j++;                                                                    // Point to next character of the buffer.
                        if(buffer[j]==(char) 0)                                                 // If that character is NULL then do...
                        {
                            tempKeyVar[sc] = (char) 0;                                          // Finalize the tempKeyVar with a null character.
                            j++;                                                                // Point to the next char of the buffer.
                            flagBreak++;                                                        // increment the flagBreak.
                            sc=0;                                                               // Re-initialize sc.
                            if(buffer[j]==(char) 0)                                             // if buffer's new location is NULL, then there is an error in the protocol because <value>'s first character is excepted there.
                            {
                                breakVar=1;
                                break;
                            }
                        }
                    }
                    else if(flagBreak==1)   // value is beeing manipulated
                    {
                        #ifdef _debug
                            printf("Serv_RF: PUT flagBreak 1 -> buffer[j] is <%c> or ASCII <%d>\n",buffer[j],buffer[j] );
                        #endif
                        if (sc==0) tempValVar = (char*)malloc(sizeof(char)+1);                  // Allocate space for tempValVar. If its the first character use malloc.
                        if (sc!=0) tempValVar = (char*)realloc(tempValVar,sizeof(char)*(sc+2)); // Allocate space for tempValVar. If its not the first character use realloc.
                        #ifdef _debug
                            printf("Serv_RF: digit <%d> and char <%c>\n", buffer[j],buffer[j]);
                            printf("Serv_RF: sc is: <%d>\n", sc);
                        #endif
                        tempValVar[sc++] = buffer[j];                                           // Store j-th char of buffer at sc-th position of tempValVar
                        j++;                                                                    // Point to next character of the buffer.
                        if(buffer[j]==(char) 0)                                                 // If that character is NULL then the hole <value> is found.
                        {
                            tempValVar[sc]=(char)0;                                             // Finalize the tempValVar with a null character.
                            flagBreak++;                                                        // increment flagBreak to break the while loop.
                            j++;                                                                // Point to the next element of the buffer (if that doesn't exists, for() will stop running.)
                        }
                    }
                    #ifdef _debug
                        printf("Serv_RF: PUT after -> buffer[j] is <%c> or ASCII <%d>\n",buffer[j],buffer[j] );
                    #endif
                }
            }
            else    // if there is an error in protocols, this statement will trigger
            {
                #ifdef _debug
                    printf("Serv_RF: E r r o r in protocol\n");
                    printf("Serv_RF: buffer[j]is<%c> buffer[1]is<%c> buffer[2]is<%c> buffer[3]is<%c> buffer[4]is<%c>\n",buffer[j],buffer[1],buffer[2],buffer[3],buffer[4]);
                #endif
                breakVar=1;
            }
            if(breakVar==1) // if there is an error in protocol then close the socket immediately
            {
                printf("Serv_RF: E r r o r following protocol.\n");
                if (close(clientSocket) != 0) perror("Error closing the socket:");
                return 1;
            }
            #ifdef _debug
                printf("Serv_RF: t e m p KeyVar is:<%s>\n",tempKeyVar);
                printf("Serv_RF: t e m p ValVar is:<%s>\n",tempValVar);
                printf("Serv_RF: flagBreak is:<%d>\n",flagBreak);
            #endif
            if(flagBreak==0 && breakVar!=1) // If after the above procedure the flagBreak is 0, then the server has to respond to a get command.
            {
                #ifdef _debug
                    printf("Serv_RF: get command\n");
                    printf("Serv_RF: t e m p KeyVar is:<%s>\n",tempKeyVar);
                    //char* debugVar = get(tempKeyVar);
                    //for(int k=0;k<=strlen(get(tempKeyVar));k++) printf("Serv_RF: d e b u g V a r[%d] is <%c>\n", k, debugVar[k]);
                #endif
                printf("Serv_RF: before send strToSend is <%s>\n", get(tempKeyVar));
                semop(mySemaphore, &down, 1); //Down operation
                int eh = send(clientSocket, get(tempKeyVar), strlen(get(tempKeyVar))+1, 0); // Aka ErrorHandling
                semop(mySemaphore, &up, 1);   //Up operation
                printf("Serv_RF: after send\n");
                if (eh < 0) perror("ERROR writing to socket:");
            }
            if(flagBreak==2 && breakVar!=1) // If after the above procedure the flagBreak is 2, then the server has to execute a put command.
            {
                #ifdef _debug
                    printf("Serv_RF: put command\n");
                    printf("Serv_RF: t e m p KeyVar is:<%s>\n",tempKeyVar);
                    printf("Serv_RF: t e m p ValVar is:<%s>\n",tempValVar);
                #endif
                semop(mySemaphore, &down, 1); //Down operation
                put(tempKeyVar,tempValVar);
                semop(mySemaphore, &up, 1);   //Up operation
                free(tempValVar);
                tempValVar=(char) 0;
            }
            free(tempKeyVar);
            tempKeyVar=(char) 0;
        }
        #ifdef _debug
            printf("Serv_RF: int cast Buffer[0]:<%d>\n", buffer[0]);
            printf("Serv_RF: char cast Buffer[0]:<%c>\n", buffer[0]);
        #endif
        bzero(buffer,_BUFFER_SIZE);
        #ifdef _debug
            printf("Serv_RF: b u f f e r  i s  <%s>\n", buffer);
        #endif
    }
} 
 
void signalHandlerFunction(){                                       // If ctrl+c is pressed.
    #ifdef _debug
        printf("signalHandlerFunction: a dragon is out here\n"); 
    #endif
    //freeFun();                                                      // frees the memory that allocated by <key> <value> stores at TAS(see keyvalue.h for TAS)
    semctl(mySemaphore, 0, IPC_RMID);
    shmctl(shmidStruct, IPC_RMID, NULL);
    shmctl(shmidTableRow, IPC_RMID, NULL);
    if (close(mySocket) != 0) perror("Error closing the socket:");
    exit(0);
}
 
/*=====================================================================================*/
/*======tokenCleanerFunction -> used to clear out the input stream that has the =======*/
/*======following form : pcity\\0patra\\0pcity\\0athens\\0gcity\\0=====================*/
/*=== THIS FUNCTION WAS USED IN EARLIER VERSIONS ======================================*/
/*=== THIS FUNCTION WAS USED IN EARLIER VERSIONS ======================================*/
/*=== THIS FUNCTION WAS USED IN EARLIER VERSIONS ======================================*/
/*void tokenCleanerFunction(char* string){                                          // string is like: "pcity\\0patra\\0pcity\\0athens\\0gcity\\0", and commands from client are like "put city patra get city".
    #ifdef _debug
        printf("TCF: string is<%s>\n", string);
    #endif
    int i=0, flag=0, keylen=0, vallen=0, j=0;                                       // Some integers that are going to be needed.
    while(string && string[i])                                                      // While string(the buffer) exists and i-th element is not null do:
    {
        char* cleanKey=(char)0;                                                     // CleanKey is the variable that is going to have the key that came from client, cleared out by the tokens and null terminators. We are initializing it with NULL.
        char* cleanValue=(char)0;                                                   // CleanValue is the variable that is going to have the value that came from client, cleared out by the tokens and null terminators. We are initializing it with NULL.
        #ifdef _debug
            printf("TCF: i start with i:<%d>\n",i);
            printf("TCF: %c %c %c %c %c %c %c\n", string[i], string[i+1], string[i+2], string[i+3], string[i+4], string[i+5], string[i+6]);
            printf("TCF: string[%d] ascii: <%d>\n",i,string[i]);
        #endif
        keylen=0; vallen=0;                                                         // Initializing keylen and vallen. KeyLen and ValLen are variables that are going to have the length of key and value respectively.
        if(string[i]==103) //flag1 get                                              // If the first byte(first char) is 103 ('g' in ASCII), the do: (get command (p.e. get city)).
        {
            if (cleanKey && cleanKey[0]) bzero(cleanKey,strlen(cleanKey));          // Initialize clenKey var, if it is not.
            string++;                                                               // Left shift in string to get rid of 'p' or 'g' (112 , 103) and then point to first char of key.
            int sc=0;                                                               // Simplecounter//flag1 get.
            #ifdef _debug
                printf("TCF: get string is<%s>\n",string);
            #endif
            flag=0;                                                                 // Flag initialization.
            while(flag!=1)                                                          //
            {
                #ifdef _debug
                    printf("TFG: while loop with i <%d>\n", i);
                #endif  
                if(string[i] == '\\' && string[i+1] == '0')                         // If we find \0 then we are incrementing the flag. At get command that means the end of it.
                {
                    flag++;                                                         // Incrementing flag to end the loop. All work is done.
                    i = i+2;                                                        // Incrementing i by 2, to point to the next command of the string that came.
                    j=0;                                                            // Initializing j, that is going to be used as a counter to store the data into the cleanKey and cleanValue in the future loops.
                    continue;                                                       // Break the loop.
                }
                if (sc==0) cleanKey = (char*)malloc(sizeof(char));                  // If we are writing for the first time then we need malloc.
                if (sc!=0) cleanKey = (char*)realloc(cleanKey,sizeof(char)*(j+1));  // If cleanKey already has elements inside it, then we are using realloc.
                cleanKey[j++]=string[i];                                            // Building the clenKey variable, char by char.
                keylen++;                                                           // Incrementing the length of key (not used somewhere though).
                sc++;                                                               // Incrementing the simple counter that is used in malloc and realloc conditions.
                i++;                                                                // Incrementing i that points to the elements of the string.
            }
            #ifdef _debug
                printf("TFG: before malloc\n");
            #endif
            char* stringToSend=(char) 0;                                            // Initializing char* stringToSend. string to send will have the matching values, or the no match operation code.
            stringToSend  = (char*)malloc(sizeof(char)*strlen(get(cleanKey))+1);    // Allocating space for stringToSend to client.
            #ifdef _debug
                printf("TFG: after malloc\n");
            #endif
            stringToSend = get(cleanKey);                                           // Storing result of get function (refer to keyvalue.h header).                                 
            #ifdef _debug                                                           // stringToSend now has all the matching values.
                printf("TCF: stringToSend is: <%s>\n", stringToSend);
            #endif
            int eh = write(clientSocket,stringToSend,strlen(stringToSend));         // 'eh' Aka ErrorHandling == W rite F unction VAR iable.
            if (eh < 0) perror("ERROR writing to socket:");
        }
        else if(string[i]==112) //flag2 put
        {
            #ifdef _debug
                printf("TCF: clean key from before <%s>\n", cleanKey);
                printf("TCF: clean value from before <%s>\n", cleanValue);
            #endif
            if (cleanKey && cleanKey[0]) bzero(cleanKey,strlen(cleanKey));          // Initialize clenKey var, if it is not.
            if (cleanValue && cleanValue[0]) bzero(cleanValue,strlen(cleanValue));  // Initialize clenValue var, if it is not.
            #ifdef _debug
                printf("TCF: clean key from before after zero <%s>\n", cleanKey);
                printf("TCF: clean value from before after zero <%s>\n", cleanValue);
            #endif
            string++;                                                                       // Left shift in string to get rid of 'p' or 'g' (112 , 103) and then point to first char of key.
            int sc=0;                                                                       // Simplecounter.
            flag=0;                                                                         // Initializing flag.
            while(flag!=2)                                                                  // If the first byte(first char) is 112 ('p' in ASCII), the do: (put command (p.e. put city patras).
            {
                if(string[i] == '\\' && string[i+1] == '0')                                 // If we find \0 then we are incrementing the flag. At put command that means start of value for flag=1 and the end of it in the next case.
                {
                    flag++;                                                                 // Flag incrementing.
                    i = i+2;                                                                // i points to first char of Value for flag=1 and the the first char of the next command for flag=2.
                    j=0;                                                                    // Initializing j, that is going to be used as a counter to store the data into the cleanKey and cleanValue in the future loops.
                    sc=0;   // CHECK IF THAT IS NEEDED DUDE
                    continue;                                                               // Breaking the loop.
                }
                if(flag==0)                                                                 // If flag=0 then we are manipulating the key.
                {
                    if (sc==0) cleanKey = (char*)malloc(sizeof(char));                      // If we are writing for the first time then we need malloc.
                    if (sc!=0) cleanKey = (char*)realloc(cleanKey,sizeof(char)*(j+1));      // If cleanKey already has elements inside it, then we are using realloc.
                    cleanKey[j++]=string[i];                                                // Building the clenKey variable, char by char.
                    keylen++;                                                               // Counting length of key.
                    sc++;                                                                   // Incrementing simple counter that is used in malloc and realloc conditions.
                }
                if(flag==1)                                                                 // If flag=0 then we are manipulating the value.
                {
                    #ifdef _debug
                        printf("TCF: Simple Counter for value is <%d>\n", sc);
                    #endif
                    if (sc==0) cleanValue = (char*)malloc(sizeof(char));                    // If we are writing for the first time then we need malloc
                    if (sc!=0) cleanValue = (char*)realloc(cleanValue,sizeof(char)*(j+1));  // If cleanKey already has elements inside it, then we are using realloc.
                    cleanValue[j++]=string[i];                                              // Building the clenValue variable, char by char.
                    vallen++;                                                               // Counting length of key.
                    sc++;                                                                   // Incrementing the simple counter that is used in malloc and realloc conditions.
                }
                #ifdef _debug
                    printf("TCF: 112string[i]:<%c>\n",string[i]);
                #endif
                i++;                                                                        // Incrementing i to point to the next char of the string.
            }
            put(cleanKey,cleanValue);                                                       // Calling put function (refer to keyvalue.h header).
            #ifdef _debug
                printf("TCF:   cleanKey is<%s>\n", cleanKey);
                printf("TCF:   cleanVal is<%s>\n", cleanValue);
            #endif
        }
        else //if(string[i]!=103 && string[i]!=112)
        {
            #ifdef _debug
                printf("TCF: E r r o r following protocol\n");
            #endif
            breakVar = 1;
            return;
        }
        #ifdef _debug
            printf("TCF: string[i]:<%c>\n",string[i]);
            printf("TCF: kelen is <%d>\n", keylen);
            printf("TCF: vallen is <%d>\n", vallen);
        #endif
        //sleep(1);                                                                         // Sleep here was used to debug
    }
}*/