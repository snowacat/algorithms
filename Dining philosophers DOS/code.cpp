#include <conio.h>
#include <dos.h>
#include <fstream.h>
#include <stdlib.h>
#include <iostream.h>
 
#define INTR 0x1C
#define stackSize 38
 
// Constants
int const TIME = 100;
int const COUNT_ITERATION = 10;
int const COUNT_PHILOSOPHER = 4;
char const *OUTPUT_FILE_NAME = "log.txt";
 
int eatTimeArray[COUNT_PHILOSOPHER];
int step = 0;
int currentProcess = 0;
 
typedef enum
{
  Think,
  Eat,
  Wait
} EPhilState;
 
typedef struct
{
  EPhilState State;
} SPhilosopher;
 
struct Table {
   char state[stackSize];
   int* left_fork;
   int* right_fork;
} Processes[COUNT_PHILOSOPHER];
 
int Forks[COUNT_PHILOSOPHER];
SPhilosopher Philosoph[COUNT_PHILOSOPHER];
int CurPhil = 0;
int first = 1;
 
ofstream log(OUTPUT_FILE_NAME);
void AddToLog();
void interrupt (far* old_vector) (...);
void interrupt handler(...);
 
void PhilosopherLife()
{
  // Step 1: thinking...
  delay(random(TIME));
  Philosoph[currentProcess].State = Wait;
  AddToLog();
 
  // Step 2: Eating...
  while(1)
  {
    delay(random(TIME));
    setvect(INTR, old_vector);
 
    // Get forks if all of them are not busy, else repeat cycle
    if ((*(Processes[currentProcess].left_fork) == 1) && (*(Processes[currentProcess].right_fork) == 1))
    {
      *(Processes[currentProcess].left_fork) = 0;
      *(Processes[currentProcess].right_fork) = 0;
      break;
    }
    else
    {
      setvect(INTR, handler);
    }
  }
 
  setvect(INTR, handler);
  Philosoph[currentProcess].State = Eat;
  AddToLog();
 
  int eatingTime = random(TIME);
  eatTimeArray[currentProcess] = eatingTime;
  delay(eatingTime);
 
  // Step 3: set free forks
  setvect(INTR, old_vector);
  *(Processes[currentProcess].left_fork) = 1;
  *(Processes[currentProcess].right_fork) = 1;
  Philosoph[currentProcess].State = Think;
  setvect(INTR, handler);
 
  AddToLog();
}
 
void interrupt handler(...)
{
  char *stack_pointer;    
  asm mov stack_pointer, sp; // get current stack value (adress)
 
  if (first)
  {
    first = 0;
    // copy current programm state for all "process"
    for (int i = 0; i < COUNT_PHILOSOPHER; i++)
    {
      memcpy(Processes[i].state, stack_pointer, stackSize);
    }
  }
  else
  {
    // Copy current programm state to state of currentProcess
    memcpy(Processes[currentProcess].state, stack_pointer, stackSize);
    currentProcess++;
    if(currentProcess >= COUNT_PHILOSOPHER)
    {
      currentProcess = 0;
    }
    memcpy(stack_pointer, Processes[currentProcess].state, stackSize);
  }
}
 
void AddToLog()
{
  setvect(INTR, old_vector);
 
  cout << "------------------------------------------" << endl;
  cout << "Step: " << step << endl;
 
  for (int i = 0; i < COUNT_PHILOSOPHER; i++)
  {
    if (Forks[i])
    {
      cout << "|";
      log << "|";
    }
    else
    {
      cout << " ";
      log << " ";
    }
 
    cout << " ";
    log << " ";
 
    if (Philosoph[i].State==Think)
    {
      cout << "T";
      log << "T";
    }
    else if (Philosoph[i].State==Eat)
    {
      cout << "E";
      cout << "(Time: " << eatTimeArray[CurPhil] << ")";
      log << "E";
      log << "(Time: " << eatTimeArray[CurPhil] << ")";
    }
    else if (Philosoph[i].State==Wait)
    {
      cout << "W";
      log << "W";
    }
    cout << " ";
    log << " ";
  }
 
  cout << endl;
  log << endl;
 
  step++;
  setvect(INTR, handler);
}
 
int main()
{
  clrscr();
  randomize();
  for (unsigned int i = 0; i < COUNT_PHILOSOPHER; i++)
  {
    Forks[i] = 1;
 
    Processes[i].left_fork = &(Forks[i]);
    if (i < COUNT_PHILOSOPHER - 1) 
    {
      Processes[i].right_fork = &(Forks[i+1]);
    }
    else 
    {
      Processes[i].right_fork = &(Forks[0]);
    }
  }
 
  // Save old vector
  old_vector = getvect(INTR); 
  setvect(INTR, handler);
 
  while (step < COUNT_ITERATION)
  {
    PhilosopherLife();
  }
  
  log.close();
  setvect(INTR, old_vector);
 
  return 0;
}