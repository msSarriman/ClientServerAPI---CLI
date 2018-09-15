/*************************************************
THE TABLE THAT IS CREATED IN THIS CODE TO STORE THE 
KEY AND VALUES IS A char*** TYPE. 
THE FIRST MEMORY ALLOCATION OF IT IS GETTING DONE WITH
THE allocateFunction CALL INSIDE put()

THE TABLE HAS THE FOLLOWING ORDER:

   R\C 	||---1---|---2---|   
|---1---|| KEY1  | VALUE1|
|---2---|| KEY2  | VALUE2|
|---3---|| KEY3  | VALUE3|
|-(N-1)-|| KEYN-1| VALN-1|
|---N---|| KEYN  | VALN  |

THE TABLE ABOVE IS LIKE A DB TABLE. 
COLUMN 1 : STORES KEY
COLUMN 2 : STORES VALUE
*************************************************/
#ifndef __KEYVALUE_H
#define __KEYVALUE_H

#ifdef _serv2SHM

typedef struct keyvalueStruct{
	char key[55];
	char val[55];
}keyvalue;
keyvalue *kvptr;

int shmidStruct;
int shmidTableRow;
int *tableRow;
char valuesRet[100];
char* returnNotFound="n\0";

char *get(char *key){
	//if(*tableRow==0) return "n";
	if(*tableRow==0) return returnNotFound;
	int j=0;
	printf("GET: key to retrieve is <%s> and replay <%d>\n", key, j++);
	printf("GET: ...with tableRow <%d> \n", *tableRow);
	for(int i=0; i<*tableRow; i++)
	{
		if(strcmp(kvptr[i].key,key) == 0)
		{
			bzero(valuesRet,100);
			valuesRet[0]= 'f';										// Making first byte of this communication to have the code 102 (ASCII for f). (a.k.a found)
			strcat(valuesRet,kvptr[i].val);
			valuesRet[strlen(kvptr[i].val)+1]= (char) 0;        						// // is NULL in order to have a terminated string.*/
			printf("GET: key to return is <%s>\n", valuesRet);
			return valuesRet;
		}
	}
	return returnNotFound;
}

int keyExists(char* key, char* value){
	for(int i=0; i<*tableRow; i++)
	{
		if(strcmp(kvptr[i].key,key)==0)
		{
			printf("PUT: Key exists!!!\n");
			bzero(kvptr[i].val,55);
			strcpy(kvptr[i].val,value);
			//#ifdef _debug
				for (int i=0; i<*tableRow; i++)
				{
					printf("KeyExists: row[%d].key <%s>\n",i,kvptr[i].key);
					printf("KeyExists: row[%d].val <%s>\n",i,kvptr[i].val);
				}
			//#endif
			return 1;
		}
	}
	printf("KeyExists: return 0\n");
	return 0;
}

void put(char *key, char *value){
	printf("PUT: put is about to search for key value <%s> <%s> with tableRow <%d> \n", key, value, *tableRow);
	if (keyExists(key,value)==1)				// checking if key-value pair already exists.
	{
		return;										// if it does, put() exits.
	}
	printf("PUT: tableRow is <%d>\n", *tableRow);
	bzero(kvptr[*tableRow].key,55);
	bzero(kvptr[*tableRow].val,55);
	strcpy(kvptr[*tableRow].key,key);
	strcpy(kvptr[*tableRow].val,value);
	*tableRow=*tableRow+1;
	//#ifdef _debug
		for (int i=0; i<*tableRow; i++)
		{
			printf("PUT: row[%d].key <%s>\n",i,kvptr[i].key);
			printf("PUT: row[%d].val <%s>\n",i,kvptr[i].val);
		}
	//#endif
	printf("PUT: tableRow is <%d>\n", *tableRow);
}

#endif




#ifdef _serv1
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

char* 	get(char*);									// Function Prototype
void 	put(char*,char*);							// Function Prototype
void 	firstAllocateFunction(size_t, size_t);		// Function Prototype
void 	rellocateFunction(size_t, size_t);			// Function Prototype
void 	freeFun();									// Function Prototype
int 	keyExists(char*, char*);					// Function Prototype
void	freeValuesRet();							// Function Prototype

char* returnNotFound="n\0";									// get() will return this variable if can't find the <key>
int tableRow=0, tableCol=0, rc, j=0, countersi=0, flag=0;	// rc = row counter , countersi=0 == counterrc
int lengthVar=0;
char* valuesRet = (char) 0;
char*** tas;									// Table of Arrays of Strings

/*=====================================================================================*/
/*============== get() ================================================================*/
/*=====================================================================================*/
char* get(char* key){							// get <key> . Function Implemantation
	#ifdef _debugheader
		printf("\nGET: Dragons with key <%s>\n",key);
		printf("GET: valuesRet is <%s>\n", valuesRet);
	#endif
	if(tableRow==0) return returnNotFound;
	if (valuesRet && valuesRet[0])							//	Initializing valuesRet Variable if it is not.
	{
		#ifdef _debugheader
			printf("GET: LengthVar is <%d>\n", lengthVar);
			printf("GET: ValuesRet[0]: <%s>\n", valuesRet);
		#endif		
		lengthVar = strlen(valuesRet);						//	Calculating the length of ValuesRet, to use it in bzero
		bzero(valuesRet,lengthVar);							// Initializing valuesRet
	}
	flag = 0;											// flag is zero. Flag is used for allocation functions
	for (rc=0; rc<tableRow; rc++)								// For all the elements of the the TAS do...
	{
		#ifdef _debugheader
			printf("GET: tas[%d][0] is <%s>\n",rc,tas[rc][0]);
			printf("GET: and key is <%s>\n", key);
		#endif
		if (strcmp(tas[rc][0],key) == 0)							// If i-th row has match with key given then do...
		{
			#ifdef _debugheader
				printf("GET: Den tha to katalavw pote <%d>\n", strcmp(tas[rc][0],key));
			#endif			
			if (flag==0) 											//Flag is needed to execute a diferent statement one for strcpy and then for strcat. Flag 0 is for empty string, so strcpy
			{
				valuesRet 	= (char *)malloc(sizeof(char) * strlen(tas[rc][1])+2);
				bzero(valuesRet,strlen(valuesRet));
				valuesRet[0]= 'f';										// Making first byte of this communication to have the code 102 (ASCII for f). (a.k.a found)
				strcat(valuesRet,tas[rc][1]);
				valuesRet[strlen(tas[rc][1])+1]= (char) 0;        						// // is NULL in order to have a terminated string.*/
				flag++;
				#ifdef _debugheader
					printf("GET: Dragons flag0 with rc<%d>\n", rc);
					printf("GET: values ret here is<%s>\n", valuesRet);
				#endif
			}
			else if(flag > 0) 									//strcat will take place here
			{
				valuesRet = (char *)realloc(valuesRet,sizeof(char) * (strlen(valuesRet)+strlen(tas[rc][1]))+2); // Memory allocation for the append. Plus 2 is for space " " and for null termination
				strcat(valuesRet," ");
				strcat(valuesRet,tas[rc][1]);
				#ifdef _debugheader
					printf("GET: Dragons flag1 with rc<%d>\n", rc);
					printf("GET: values ret apre isi is<%s>\n", valuesRet);
				#endif			
			}
		}
	}
	#ifdef _debugheader
		printf("GET: Dragons after valuesRet\n");
		printf("GET: ValuesRet to send <%s>\n", valuesRet);
		printf("GET: Flag is <%d>\n", flag);
	#endif
	if (flag == 0)						// In case key hasn't found, we are sending back te code 110 (ASCII for N, a.k.a. Not Found)
	{
		valuesRet = (char) 0;
		return returnNotFound;
	}
	#ifdef _debugheader
		printf("test2\n");
	#endif
	return valuesRet;
}


/*=====================================================================================*/
/*============== put() ================================================================*/
/*=====================================================================================*/
void put(char* key, char* value){				// put <key,value> . Function Implemantation
	#ifdef _debugheader
		printf("PUT: Dragons\n");
		printf("PUT: Table row is<%d>\n",tableRow);
	#endif
	if (keyExists(key,value)==1)				// checking if key-value pair already exists.
	{
		return;										// if it does, put() exits.
	}else 										// else put() stores the key-value into TAS.
	{
		size_t lengthKey=0, lengthValue=0;
		lengthKey   = strlen(key);
		lengthValue = strlen(value);
		if (tableRow == 0) firstAllocateFunction(lengthKey,lengthValue);	// We are going to need malloc for first allocation instead of realloc thats why we are testing if its the first memory allocation that we are going to
		else rellocateFunction(lengthKey,lengthValue);						// For all the other memory allocations we are going to need a reallocation function
		#ifdef _debugheader
			printf("PUT: LengthKey<%zd>\n",lengthKey);
			printf("PUT: LengthValue<%zd>\n",lengthValue);
			printf("PUT: Key to put in row <%d>: <%s>\n", tableRow, key);
			printf("PUT: Value to put in row <%d>: <%s>\n", tableRow, value);
		#endif
		#ifdef _debugheader	
			printf("PUT: tableRow number is <%d>\n", tableRow);
		#endif
		strcpy(tas[tableRow][0],key);			// tas[tableRow][0] = key
		strcpy(tas[tableRow][1],value);			// tas[tableRow][0] = value
		tableRow++;								// Used rows counter
		#ifdef _debugheader
			for (int c=0; c<tableRow; c++)
			{
				printf("PUT: tas[%d][0]:<%s>\n",c, tas[c][0]);
				printf("PUT: tas[%d][1]:<%s>\n",c, tas[c][1]);
			}
		#endif
	}
}


/*=====================================================================================*/
/*============== keyExists() -> checking if key value already exists ==================*/
/*=====================================================================================*/
int keyExists(char* key, char* value){
	for(int k=0; k<tableRow; k++)
	{
		if(strcmp(tas[k][0],key)==0/* && strcmp(tas[k][1],value)==0*/)
		{
			#ifdef _debugheader
				printf("KEYExists: The key already exists\n");
			#endif/*
			bzero(tas[k][1],strlen(tas[k][1]));*/
			free(tas[k][1]);
			tas[k][1]=(char *)calloc(strlen(value)+1,sizeof(char));
			strcpy(tas[k][1],value);
			//tas[k][1][strlen(tas[k][1])]=(char)0;// ADDING THIS AFTER WORKING SERVER ONE
			return 1;
		}
	}
	return 0;
}
/*=====================================================================================*/
/***** void freeValuesRet()										                   *****/	
/***** This fuction is called for deallocation of Memory used for valuesRet        *****/
void freeValuesRet(){
	free(valuesRet);
}

/*=====================================================================================*/
/**** void firstAllocateFunction()                                   *****/
/**** This function is beeing called for the first                   *****/
/**** memory allocation that we need for the first row of the table. *****/
void firstAllocateFunction(size_t lenKey, size_t lenValue){ 
	#ifdef _debugheader
		printf("FAF: Dragons firstalloc with rc<%d> and tableRow<%d>\n",rc,tableRow);        
	#endif	
	tas			= (char ***)calloc(1,sizeof(char **));					// Memory allocation for one triple pointer (for the table)
	tas[rc] 	= (char **)calloc(2,sizeof(char *));					// Memory allocation for two strings of pointers for i (for the row)
	tas[rc][0] 	= (char *)calloc(lenKey+1,sizeof(char));				// Memory allocation for lenKey+1 size of bytes for i,0 (for the string)
	tas[rc][1] 	= (char *)calloc(lenValue+1,sizeof(char));				// Memory allocation for lenValue+1 size of bytes fpr i,1 (for the string)
	rc++;
}



/*=====================================================================================*/
/***** void rellocateFunction()										  *****/	
/***** This fuction is called for the dynamic allocation of Memory    *****/
/***** that tas need in order to save the incoming <key,value> pairs. *****/
void rellocateFunction(size_t lenKey, size_t lenValue){
	#ifdef _debugheader
		printf("RF: Dragons reallocation\n");
		printf("RF: rc before Dragons is  <%d>\n",rc);    
	#endif
	tas 		= (char ***)realloc(tas,sizeof(char **)*(rc+1));		// Memory re-allocation for the triple pointer that already exists (for the table)
	tas[rc] 	= (char **)calloc(2,sizeof(char *));					// Memory allocation for two strings of pointers for i (for the row)
	tas[rc][0] 	= (char *)calloc(lenKey+1,sizeof(char));				// Memory allocation for lenKey+1 size of bytes for i,0 (for the string)
	tas[rc][1] 	= (char *)calloc(lenValue+1,sizeof(char));				// Memory allocation for lenValue+1 size of bytes fpr i,1 (for the string)
	rc++;
}

/*=====================================================================================*/
/*****************************************************************/
/***** This fuction is called for the memory deallocation    *****/
/*****************************************************************/
void freeFun(){
	#ifdef _debugheader
		printf("FF: Dragons Before\n");
	#endif
	int sc;						// Aka simple counter
	for (sc=0; sc<rc; sc++)
	{
		free(tas[sc][0]);		// Free the string0
		free(tas[sc][1]);		// Free the string1
		free(tas[sc]);			// Free the row
	}
	free(tas);					// Free the table
	#ifdef _debugheader
		printf("FF: Dragons After\n");
	#endif
}
#endif

#endif