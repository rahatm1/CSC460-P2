typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */ 

/**
  * This internal kernel function is the context switching mechanism.
  * It is done in a "funny" way in that it consists two halves: the top half
  * is called "Exit_Kernel()", and the bottom half is called "Enter_Kernel()".
  * When kernel calls this function, it starts the top half (i.e., exit). Right in
  * the middle, "Cp" is activated; as a result, Cp is running and the kernel is
  * suspended in the middle of this function. When Cp makes a system call,
  * it enters the kernel via the Enter_Kernel() software interrupt into
  * the middle of this function, where the kernel was suspended.
  * After executing the bottom half, the context of Cp is saved and the context
  * of the kernel is restore. Hence, when this function returns, kernel is active
  * again, but Cp is not running any more. 
  * (See file "switch.S" for details.)
  */
extern void CSwitch();
extern void Exit_Kernel();    /* this is the same as CSwitch() */

/**
  * This external function could be implemented in two ways:
  *  1) as an external function call, which is called by Kernel API call stubs;
  *  2) as an inline macro which maps the call into a "software interrupt";
  *       as for the AVR processor, we could use the external interrupt feature,
  *       i.e., INT0 pin.
  *  Note: Interrupts are assumed to be disabled upon calling Enter_Kernel().
  *     This is the case if it is implemented by software interrupt. However,
  *     as an external function call, it must be done explicitly. When Enter_Kernel()
  *     returns, then interrupts will be re-enabled by Enter_Kernel().
  */
extern void Enter_Kernel();

#define Disable_Interrupt()		asm volatile ("cli"::)
#define Enable_Interrupt()		asm volatile ("sei"::)

typedef enum priority_levels
{
	SYSTEM = 0,
	PERIODIC,
	RR
} PRIORITY_LEVELS;

/**
  *  This is the set of states that a task can be in at any given time.
  */
typedef enum process_states
{
   DEAD = 0,
   READY,
   RUNNING
} PROCESS_STATES;

/**
  * This is the set of kernel requests, i.e., a request code for each system call.
  */
typedef enum kernel_request_type
{
   NONE = 0,
   CREATE,
   NEXT,
   TERMINATE
} KERNEL_REQUEST_TYPE;

/**
  * Each task is represented by a process descriptor, which contains all
  * relevant information about this task. For convenience, we also store
  * the task's stack, i.e., its workspace, in here.
  */
typedef struct ProcessDescriptor
{
	/* stack pointer into the "workSpace" */
	unsigned char *sp;

	unsigned char workSpace[WORKSPACE];
	// The current state of the process
	PROCESS_STATES state;
	/* function to be executed as a task */
	voidfuncptr  code;
	//Process ID
	uint8_t pid;
	// Type (SYSTEM, PERIODIC, or RR)
	uint8_t type;
	// Argument passed by the init function
	int arg;
	// The remaining number of ticks for the process
	uint8_t ticks_remaining;
	// The next tick number when to run
	uint32_t next_start;
	// The period of the process (in ticks) (only used for PERIODIC task)
	uint32_t period;
	// The worst case execution time of the process (in ticks) (only used for PERIODIC task)
	uint32_t wcet;
	// A pointer to the next item in the linked list (or NULL if none)
	struct ProcessDescriptor* next;

	KERNEL_REQUEST_TYPE request;
} PD;

typedef struct task_queue_type {
    uint8_t len;
    PD * head;
    PD * tail;
} task_queue_t;

task_queue_t system_tasks;
task_queue_t periodic_tasks;
task_queue_t rr_tasks;

void queue_init(task_queue_t * list);
void enqueue(task_queue_t * list, PD * task);
PD * deque(task_queue_t * list);
