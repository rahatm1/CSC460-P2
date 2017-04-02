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
static PD Process[MAXTHREAD+1];
static channel channels[MAXCHAN];

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

volatile unsigned int num_ticks = 0;
uint8_t chan_num = 0;

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

   if (Tasks == MAXTHREAD +1) return NULL;  /* Too many task! */

   /* find a DEAD PD that we can use  */
   for (x = 0; x < MAXTHREAD +1; x++) {
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
   p->pid = x+1;
   p->ticks_remaining = 0;

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
#ifdef DEBUG
        UART_print("Dispatch\n");
        UART_print("num_ticks %d\n", num_ticks);
#endif

//TODO: Handle BLOCKED better
	if (Cp->state != RUNNING ) {
		if (system_tasks.head && peek(&system_tasks)->state != BLOCKED) {
			Cp = peek(&system_tasks);
        } else if (periodic_tasks.len > 0 && num_ticks >= peek(&periodic_tasks)->next_start) {
    		PD* p = peek(&periodic_tasks);
    		// Check for overlap with another periodic task
    		if (p->next != NULL && num_ticks >= p->next->next_start) {
    			OS_Abort(-2);
    		}
#ifdef DEBUG
        UART_print("periodic task dispatch\n");
#endif
    		Cp = p;
		} else if (rr_tasks.head) {
			while (peek(&rr_tasks)->state == BLOCKED) {
               enqueue(&rr_tasks, deque(&rr_tasks));
			}
			Cp = peek(&rr_tasks);
		}
	}

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

/* #ifdef DEBUG */
/* 			UART_print("Req: %d\n", Cp->request); */
/* #endif */
       switch(Cp->request){
       case NEXT:
       switch (Cp->type) {
           case SYSTEM:
               enqueue(&system_tasks, deque(&system_tasks));
               break;
           case PERIODIC:
				deque(&periodic_tasks);
				Cp->next_start = Cp->next_start + Cp->period;
				Cp->ticks_remaining = Cp->wcet;
				task_list_insert_into_offset_order(&periodic_tasks, (PD *) Cp);
                #ifdef DEBUG
                        UART_print("periodic kernel len %d\n", periodic_tasks.len);
                #endif
               break;
           case RR:
			   Cp->ticks_remaining = 1;
               enqueue(&rr_tasks, deque(&rr_tasks));
               break;
       }
	    if (Cp->state != BLOCKED) Cp->state = READY;
		Dispatch();
        break;

       case TIMER:
		switch (Cp->type) {
			case SYSTEM: // drop down
				break;
			case PERIODIC: // drop down
				Cp->ticks_remaining--;
				if (Cp->ticks_remaining <= 0) {
					OS_Abort(-1);
				}
				break;
			case RR:
				Cp->ticks_remaining--;
				if (Cp->ticks_remaining <= 0) {
					// Reset ticks and move to back
					Cp->ticks_remaining = 1;
					enqueue(&rr_tasks, deque(&rr_tasks));
				}
				break;
		}
	    if (Cp->state != BLOCKED) Cp->state = READY;
        Dispatch();
        break;

       case NONE:
         /* NONE could be caused by a timer interrupt */
	      if (Cp->state != BLOCKED) Cp->state = READY;
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
			UART_print("type: %d\n", Cp->type);
			UART_print("PID: %d\n", Cp->pid-1);
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
   for (x = 0; x < MAXTHREAD +1; x++) {
      memset(&(Process[x]),0,sizeof(PD));
      Process[x].state = DEAD;
   }
   for (x = 0; x < MAXCHAN; x++) {
      memset(&(channels[x]),0,sizeof(channel));
   }
  queue_init(&system_tasks);
  queue_init(&periodic_tasks);
  queue_init(&rr_tasks);
}

void OS_Start()
{
   if ( (! KernelActive) && (Tasks > 0)) {
      /* we may have to initialize the interrupt vector for Enter_Kernel() here. */

      /* here we go...  */
      KernelActive = TRUE;
      setupTimer();
      for (;;) {
          Next_Kernel_Request();
      }
      /* NEVER RETURNS!!! */
   }
}

void OS_Abort(unsigned int error) {
	//TODO: blink lights to indicate errors
	Disable_Interrupt();

#ifdef DEBUG
    UART_print("\nOS Aborted: %d\n", error);
#endif
	for(;;) {}
}

CHAN Chan_Init() {
    if (chan_num == MAXCHAN) return (CHAN) NULL;
    return ++chan_num;
}

void Send( CHAN ch, int v ) {
    if (channels[ch].sender != NULL) {
        OS_Abort(-5);
    }

	Cp->message = v;
    channels[ch].sender = (PD *) Cp;

	uint8_t recv_index = channels[ch].recv_index;
    if (recv_index == 0) {
        Disable_Interrupt();
        Cp->state = BLOCKED;
        Cp->request = NEXT;
        Enter_Kernel();
    }
#ifdef DEBUG
    UART_print("Channel %d\n", ch);
    UART_print("from send %d\n", channels[ch].sender->message);
    UART_print("recv index %d\n", recv_index);
#endif
	int i;
	for (i=0; i<recv_index; i++) {
    	channels[ch].receiver[i]->state = READY;
	}
}

void Write(CHAN ch, int v) {
	Cp->message = v;
    channels[ch].sender = (PD *) Cp;

	uint8_t recv_index = channels[ch].recv_index;
#ifdef DEBUG
    UART_print("Channel %d\n", ch);
    UART_print("from write, recv index %d\n", recv_index);
#endif
	int i;
	for (i=0; i<recv_index; i++) {
    	channels[ch].receiver[i]->state = READY;
	}
}

int Recv( CHAN ch ) {
    if (channels[ch].sender == NULL) {
        Disable_Interrupt();
        Cp->state = BLOCKED;
        Cp->request = NEXT;
		uint8_t recv_index = channels[ch].recv_index;
        channels[ch].receiver[recv_index++] = (PD*) Cp;
		channels[ch].recv_index = recv_index;
#ifdef DEBUG
    UART_print("Channel %d\n", ch);
    UART_print("before kernel, recv index%d\n", recv_index);
#endif
        Enter_Kernel();
    }
#ifdef DEBUG
	UART_print("%d", Cp->arg);
    UART_print("from recv, msg %d\n", channels[ch].sender->message);
#endif
    channels[ch].sender->state = READY;
	Cp->state = READY;
	int msg = channels[ch].sender->message;
	channels[ch].recv_index--;
	if (channels[ch].recv_index == 0) {
      memset(&(channels[ch]),0,sizeof(channel));
	  chan_num--;
	}
	return msg;
}

PID Task_Create_System(voidfuncptr f, int arg) {
    PD * p = Kernel_Create_Task(f, SYSTEM);
	if (p == NULL) return 0;
    p->arg = arg;
    enqueue(&system_tasks, p);
    return p->pid;
}

PID Task_Create_Period(voidfuncptr f, int arg, TICK period, TICK wcet, TICK offset){
    PD * p = Kernel_Create_Task(f, PERIODIC);
	if (p == NULL) return 0;
    p->arg = arg;

    p->period = period;
    p->wcet = wcet;
    p->next_start = num_ticks + offset;
    p->ticks_remaining = wcet;

    task_list_insert_into_offset_order(&periodic_tasks, p);

    return p->pid;
}

PID Task_Create_RR(voidfuncptr f, int arg) {
    PD * p = Kernel_Create_Task(f, RR);
	if (p == NULL) return 0;
    p->arg = arg;
    enqueue(&rr_tasks, p);
	return p->pid;
}

/**
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next()
{
     Disable_Interrupt();
	 Cp->state = READY;
     Cp->request = NEXT;
     Enter_Kernel();
}

/**
  * The calling task terminates itself.
  */
void Task_Terminate()
{
	Disable_Interrupt();
	Cp->request = TERMINATE;
	Process[Cp->pid-1].state = DEAD;
	Tasks--;
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
  num_ticks++;
  Cp->request = TIMER;
  Enter_Kernel();
}

unsigned int Now() {
	unsigned int tmp = num_ticks * MSECPERTICK;
	return tmp + TCNT3/62;
}

void idle_task() {
	for(;;) {};
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
   Task_Create_System(a_main, 0);
   Task_Create_RR(idle_task, 0);
   OS_Start();
}
