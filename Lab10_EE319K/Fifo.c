// FiFo.c
// Runs on LM4F120/TM4C123
// Provide functions that implement the Software FiFo Buffer
// Last Modified: 11/11/2021 
// Student names: change this to your names or look very silly
#include <stdint.h>

// Declare state variables for FiFo
//        size, buffer, put and get indexes
#define FIFO_SIZE 8
int32_t static PutI; // Index to put new
                
int32_t static GetI; //Index to get 

int32_t static Fifo[FIFO_SIZE];
int static isFull;
int static isEmpty;

// *********** FiFo_Init**********
// Initializes a software FIFO of a
// fixed size and sets up indexes for
// put and get operations
void Fifo_Init() {
	PutI = GetI = 0;
	isFull = 0;			//0 for false
	isEmpty = 1;		//1 for true
}

// *********** FiFo_Put**********
// Adds an element to the FIFO
// Input: Character to be inserted
// Output: 1 for success and 0 for failure
//         failure is when the buffer is full
uint32_t Fifo_Put(char data){
  //Complete this routine
	if (isFull == 1) {
    return(0);
	}
  Fifo[PutI] = data;
	if (PutI == (FIFO_SIZE-1)){
		PutI = 0;
	}else{
		PutI++;
	}
	isEmpty = 0;
	if (PutI == GetI){
		isFull = 1;
	}else{
		isFull = 0;
	}
   return(1);
}

// *********** Fifo_Get**********
// Gets an element from the FIFO
// Input: Pointer to a character that will get the character read from the buffer
// Output: 1 for success and 0 for failure
//         failure is when the buffer is empty
uint32_t Fifo_Get(char *datapt){
  if (isEmpty == 1) {
          return(0);
   }
   *datapt = Fifo[GetI];
	if (GetI == (FIFO_SIZE-1)){
		GetI = 0;
	}else{
		GetI++;
	}
	isFull = 0;
	if (PutI == GetI){
		isEmpty = 1;
	}else{
		isEmpty = 0;
	}
   return(1);
}




