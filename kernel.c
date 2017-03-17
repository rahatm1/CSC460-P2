#include "os.h"
#include <string.h>
#include <avr/interrupt.h>
#include "common.h"
#include "UART/BlockingUART.h"
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
extern void a_main();
/*===========
  * RTOS Internal
  *===========
  */

#define DEBUG 1
/* Prototype */
void Task_Terminate(void);
void setupTimer(void);

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXTHREAD];
task_queue_t system_tasks;
task_queue_t periodic_tasks;
task_queue_t rr_tasks;

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

/** number of tasks created so far */
volatile static unsigned int Tasks;
BOOL KernelActive = FALSE;

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
PD * Kernel_Create_Task( voidfuncptr f, PRIORITY_LEVELS type )
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
   *(unsigned char *)sp-- = 0x00;

   //Place return address of function at bottom of stack
   *(unsigned char *)sp-- = ((unsigned int)f) & 0xff;
   *(unsigned char *)sp-- = (((unsigned int)f) >> 8) & 0xff;
   *(unsigned char *)sp-- = 0x00;

	//Place stack pointer at top of stack
   sp = sp - 34;

   p->sp = sp;		/* stack pointer into the "workSpace" */
   p->code = f;		/* function to be executed as a task */
   p->request = NONE;
   p->type = type;
   p->pid = x;

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
	if (Cp->state != RUNNING) {
		if (system_tasks.head) {
			Cp = peek(&system_tasks);
			//TODO: Periodic
		} else if (rr_tasks.head) {
			Cp = peek(&rr_tasks);
		}
	}
#ifdef DEBUG
			UART_print("CP %p\n", Cp);
			UART_print("type %d\n", Cp->type);
			UART_print("Tasks %d\n", Tasks);
			UART_print("system_tasks len%d\n", system_tasks.len);
			UART_print("rr tasks len%d\n", rr_tasks.len);
#endif

    CurrentSp = Cp->sp;
	Cp->state = RUNNING;
}

void Next_Kernel_Request() {

	Dispatch();

   while(1) {
       Cp->request = NONE; /* clear its request */

       /* activate this newly selected task */
       CurrentSp = Cp->sp;

       Exit_Kernel();    /* or CSwitch() */

       /* if this task makes a system call, it will return to here! */

        /* save the Cp's stack pointer */
       Cp->sp = CurrentSp;

#ifdef DEBUG
			UART_print("Req: %d", Cp->request);
#endif
       switch(Cp->request){
       case CREATE:
       //TODO
           break;
       case NEXT:
       case NONE:
         /* NONE could be caused by a timer interrupt */
          Cp->state = READY;
          Dispatch();
          break;
       case TERMINATE:
          /* deallocate all resources used by this task */
		switch (Cp->type) {
			case SYSTEM:
				deque(&system_tasks);
				break;
			case PERIODIC:
				deque(&periodic_tasks);
				break;
			case RR:
				deque(&rr_tasks);
				break;
		}
#ifdef DEBUG
			UART_print("killed task\n");
#endif
		  Cp->state = DEAD;
          Dispatch();
          break;
       default:
          /* Houston! we have a problem here! */
          break;
       }
    }
}

/**
  * This function initializes the RTOS and must be called before any other
  * system calls.
  */
void OS_Init()
{
  Tasks = 0;
  KernelActive = FALSE;
  //Reminder: Clear the memory for the task on creation.
  int x;
   for (x = 0; x < MAXTHREAD; x++) {
      memset(&(Process[x]),0,sizeof(PD));
      Process[x].state = DEAD;
   }
  queue_init(&system_tasks);
  queue_init(&periodic_tasks);
  queue_init(&rr_tasks);
}

void OS_Start()
{
   if ( (! KernelActive) && (Tasks > 0)) {
      Disable_Interrupt();
      /* we may have to initialize the interrupt vector for Enter_Kernel() here. */

      /* here we go...  */
      KernelActive = TRUE;
      /* setupTimer(); */
      Next_Kernel_Request();
      /* NEVER RETURNS!!! */
   }
}

void OS_Abort(unsigned int error) {
	//TODO: blink lights to indicate errors
	Disable_Interrupt();
	for(;;) {}
}

void Task_Create( voidfuncptr f)
{
   if (KernelActive ) {
     Disable_Interrupt();
     Cp->request = CREATE;
     Cp->code = f;
     Enter_Kernel();
   } else {
      /* call the RTOS function directly */
      /* Kernel_Create_Task( f ); */
   }
}

PID Task_Create_System(voidfuncptr f, int arg) {
    PD * p = Kernel_Create_Task(f, SYSTEM);
    p->arg = arg;
    enqueue(&system_tasks, p);
    return p->pid;
}

PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset){
	//TODO
	return -1;
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
	 Cp->state = READY;
     Cp->request = NEXT;
     Enter_Kernel();
  }
}

/**
  * The calling task terminates itself.
  */
void Task_Terminate()
{
	/* Disable_Interrupt(); */
	Cp->request = TERMINATE;
#ifdef DEBUG
			UART_print("killed task\n");
#endif
	Enter_Kernel();
	/* never returns here! */
}


int Task_GetArg() {
    return Cp->arg;
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
	//TODO
  /* Cp->request = NEXT; */
  /* Enter_Kernel(); */
}


/**
  * This function creates two cooperative tasks, "Ping" and "Pong". Both
  * will run forever.
  */
void main()
{

#ifdef DEBUG
	UART_Init0(57600);
	UART_print("\nboot\n");
#endif

   OS_Init();
   //TODO: Manually create a_main as a system task
   Task_Create_System(a_main, 0);
   OS_Start();
}
