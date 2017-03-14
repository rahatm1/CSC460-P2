#define F_CPU 16000000
#include "os.h"
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "common.h"
/**
 * \file active.c
 * \brief A Skeleton Implementation of an RTOS
 * 
 * \mainpage A Skeleton Implementation of a "Full-Served" RTOS Model
 * This is an example of how to implement context-switching based on a 
 * full-served model. That is, the RTOS is implemented by an independent
 * "kernel" task, which has its own stack and calls the appropriate kernel 
 * function on behalf of the user task.
 *
 * \author Dr. Mantis Cheng
 * \date 29 September 2006
 *
 * ChangeLog: Modified by Alexander M. Hoole, October 2006.
 *			  -Rectified errors and enabled context switching.
 *			  -LED Testing code added for development (remove later).
 *
 * \section Implementation Note
 * This example uses the ATMEL AT90USB1287 instruction set as an example
 * for implementing the context switching mechanism. 
 * This code is ready to be loaded onto an AT90USBKey.  Once loaded the 
 * RTOS scheduling code will alternate lighting of the GREEN LED light on
 * LED D2 and D5 whenever the correspoing PING and PONG tasks are running.
 * (See the file "cswitch.S" for details.)
 */

/*===========
  * RTOS Internal
  *===========
  */

#define DEBUG 0
/* Prototype */
void Task_Terminate(void);
void setupTimer(void);

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXTHREAD];

/**
  * The process descriptor of the currently RUNNING task.
  */
volatile static PD* Cp; 

/** 
  * Since this is a "full-served" model, the kernel is executing using its own
  * stack. We can allocate a new workspace for this kernel stack, or we can
  * use the stack of the "main()" function, i.e., the initial C runtime stack.
  * (Note: This and the following stack pointers are used primarily by the
  *   context switching code, i.e., CSwitch(), which is written in assembly
  *   language.)
  */         
volatile unsigned char *KernelSp;

/**
  * This is a "shadow" copy of the stack pointer of "Cp", the currently
  * running task. During context switching, we need to save and restore
  * it into the appropriate process descriptor.
  */
volatile unsigned char *CurrentSp;

/** index to next task to run */
volatile static unsigned int NextP;  

/** 1 if kernel has been started; 0 otherwise. */
volatile static unsigned int KernelActive;  

/** number of tasks created so far */
volatile static unsigned int Tasks;  

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
PD * Kernel_Create_Task( voidfuncptr f, int type )
{

   int x;

   if (Tasks == MAXTHREAD) return NULL;  /* Too many task! */

   /* find a DEAD PD that we can use  */
   for (x = 0; x < MAXTHREAD; x++) {
       if (Process[x].state == DEAD) break;
   }
   Tasks++;

   PD *p = &(Process[x]);

   unsigned char *sp;

#ifdef DEBUG
   int counter = 0;
#endif

   //Changed -2 to -1 to fix off by one error.
   sp = (unsigned char *) &(p->workSpace[WORKSPACE-1]);



   /*----BEGIN of NEW CODE----*/
   //Initialize the workspace (i.e., stack) and PD here!

   //Clear the contents of the workspace
   memset(&(p->workSpace),0,WORKSPACE);

   //Notice that we are placing the address (16-bit) of the functions
   //onto the stack in reverse byte order (least significant first, followed
   //by most significant).  This is because the "return" assembly instructions 
   //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig. 
   //second), even though the AT90 is LITTLE ENDIAN machine.

   //Store terminate at the bottom of stack to protect against stack underrun.
   *(unsigned char *)sp-- = ((unsigned int)Task_Terminate) & 0xff;
   *(unsigned char *)sp-- = (((unsigned int)Task_Terminate) >> 8) & 0xff;

   //Place return address of function at bottom of stack
   *(unsigned char *)sp-- = ((unsigned int)f) & 0xff;
   *(unsigned char *)sp-- = (((unsigned int)f) >> 8) & 0xff;
   *(unsigned char *)sp-- = 0x00;

#ifdef DEBUG
   //Fill stack with initial values for development debugging
   //Registers 0 -> 31 and the status register
   for (counter = 0; counter < 34; counter++)
   {
      *(unsigned char *)sp-- = counter;
   }
#else
   //Place stack pointer at top of stack
   sp = sp - 34;
#endif
      
   p->sp = sp;		/* stack pointer into the "workSpace" */
   p->code = f;		/* function to be executed as a task */
   p->request = NONE;
   p->type = type;

   /*----END of NEW CODE----*/

   p->state = READY;
   return p;
}

/**
  * This internal kernel function is a part of the "scheduler". It chooses the 
  * next task to run, i.e., Cp.
  */
static void Dispatch()
{
     /* find the next READY task
       * Note: if there is no READY task, then this will loop forever!.
       */
   while(Process[NextP].state != READY) {
      NextP = (NextP + 1) % MAXTHREAD;
   }

   Cp = &(Process[NextP]);
   CurrentSp = Cp->sp;
   Cp->state = RUNNING;

   NextP = (NextP + 1) % MAXTHREAD;
}

/**
  * This internal kernel function is the "main" driving loop of this full-served
  * model architecture. Basically, on OS_Start(), the kernel repeatedly
  * requests the next user task's next system call and then invokes the
  * corresponding kernel function on its behalf.
  *
  * This is the main loop of our kernel, called by OS_Start().
  */
static void Next_Kernel_Request() 
{
   Dispatch();  /* select a new task to run */

   while(1) {
       Cp->request = NONE; /* clear its request */

       /* activate this newly selected task */
       CurrentSp = Cp->sp;
       Exit_Kernel();    /* or CSwitch() */

       /* if this task makes a system call, it will return to here! */

        /* save the Cp's stack pointer */
       Cp->sp = CurrentSp;

       switch(Cp->request){
       case CREATE:
           Kernel_Create_Task( Cp->code, RR);
           break;
       case NEXT:
	   case NONE:
           /* NONE could be caused by a timer interrupt */
          Cp->state = READY;
          Dispatch();
          break;
       case TERMINATE:
          /* deallocate all resources used by this task */
          Cp->state = DEAD;
          Dispatch();
          break;
       default:
          /* Houston! we have a problem here! */
          break;
       }
    } 
}


/*================
  * RTOS  API  and Stubs
  *================
  */

/**
  * This function initializes the RTOS and must be called before any other
  * system calls.
  */
void OS_Init() 
{
   int x;

   Tasks = 0;
   KernelActive = 0;
   NextP = 0;
	//Reminder: Clear the memory for the task on creation.
   for (x = 0; x < MAXTHREAD; x++) {
      memset(&(Process[x]),0,sizeof(PD));
      Process[x].state = DEAD;
   }
  queue_init(&system_tasks);
}


/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start() 
{   
   if ( (! KernelActive) && (Tasks > 0)) {
      /* Disable_Interrupt(); */
      /* we may have to initialize the interrupt vector for Enter_Kernel() here. */

      /* here we go...  */
      KernelActive = 1;
	  setupTimer();
      Next_Kernel_Request();
      /* NEVER RETURNS!!! */
   }
}

void OS_Abort(unsigned int error) {
	//TODO: blink lights to indicate errors
	Disable_Interrupt();
	for(;;) {}
}

PID Task_Create_System(voidfuncptr f, int arg) {
    PD * p = Kernel_Create_Task(f, SYSTEM);
    p->arg = arg;
    enqueue(&system_tasks, p);
    return p->pid;
}

PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset){

}

PID Task_Create_RR(voidfuncptr f, int arg) {
    PD * p = Kernel_Create_Task(f, RR);
    p->arg = arg;
    enqueue(&rr_tasks, p);
	return p->pid;
}

/**
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next() 
{
   if (KernelActive) {
     Disable_Interrupt();
     Cp->request = NEXT;
     Enter_Kernel();
  }
}


/**
  * The calling task terminates itself.
  */
void Task_Terminate() 
{
   if (KernelActive) {
      Disable_Interrupt();
      Cp->request = TERMINATE;
      Enter_Kernel();
     /* never returns here! */
   }
}


void setupTimer() {
  //Clear timer config.
  TCCR3A = 0;
  TCCR3B = 0;
  //Set to CTC (mode 4)
  TCCR3B |= (1<<WGM32);

  //Set prescaller to 256
  TCCR3B |= (1<<CS32);

  //Set TOP value (0.01 seconds)
  OCR3A = 625;

  //Enable interupt A for timer 3.
  TIMSK3 |= (1<<OCIE3A);

  //Set timer to 0 (optional here).
  TCNT3 = 0;

  Enable_Interrupt();
}


ISR(TIMER3_COMPA_vect)
{
  Cp->request = NEXT;
  Enter_Kernel();
}


/**
  * This function creates two cooperative tasks, "Ping" and "Pong". Both
  * will run forever.
  */
void main() 
{
   OS_Init();
   OS_Start();
}