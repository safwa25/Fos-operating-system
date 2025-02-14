/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/memlayout.h>
#include <inc/timerreg.h>
#include <kern/cons/console.h>
#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/trap/fault_handler.h>
#include <inc/dynamic_allocator.h>
#include "kern/cmd/command_prompt.h"
#include "kern/cmd/commands.h"
#include <kern/cpu/kclock.h>
#include <kern/cpu/cpu.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/picirq.h>
#include <kern/cpu/cpu.h>
#include <kern/mem/boot_memory_manager.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/mem/shared_memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/tests/test_kheap.h>
#include <kern/tests/test_dynamic_allocator.h>
#include <kern/tests/test_commands.h>
#include <kern/disk/pagefile_manager.h>

extern int sys_calculate_free_frames();

//Functions Declaration
//======================
void print_welcome_message();
//=======================================

//First ever function called in FOS kernel
bool autograde ;
void FOS_initialize()
{
	//get actual addresses after code linking
	extern char start_of_uninitialized_data_section[], end_of_kernel[];

	//cprintf("*	1) Global data (BSS) section...");
	{
		// Before doing anything else,
		// clear the uninitialized global data (BSS) section of our program, from start_of_uninitialized_data_section to end_of_kernel
		// This ensures that all static/global variables start with zero value.
		memset(start_of_uninitialized_data_section, 0, end_of_kernel - start_of_uninitialized_data_section);
	}
	//cprintf("[DONE]\n");

	{
		// Initialize the console.
		// Can't call cprintf until after we do this!
		cons_init();
		//print welcome message
		print_welcome_message();
	}

	cprintf("\n********************************************************************\n");
	cprintf("* INITIALIZATIONS:\n");
	cprintf("*=================\n");

	cprintf("* 1) CPU...");
	{
		//Initialize the Main CPU
		cpu_init(0);
	}
	cprintf("[DONE]\n");

	cprintf("* 2) MEMORY:\n");
	{
		// Lab 2 memory management initialization functions
		detect_memory();
		initialize_kernel_VM();
		initialize_paging();
		sharing_init();

#if USE_KHEAP
		initialize_kheap_dynamic_allocator(KERNEL_HEAP_START, PAGE_SIZE, KERNEL_HEAP_START + DYN_ALLOC_MAX_SIZE);
#endif
		//	page_check();
		setPageReplacmentAlgorithmNchanceCLOCK(1);
		//setPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX);
		//setPageReplacmentAlgorithmFIFO();
		//setPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX);

		setUHeapPlacementStrategyFIRSTFIT();
		setKHeapPlacementStrategyFIRSTFIT();

		enableBuffering(0);
		//enableModifiedBuffer(1) ;
		enableModifiedBuffer(0) ;
		setModifiedBufferLength(1000);

		ide_init();
	}
	//cprintf("* [DONE]\n");

	cprintf("* 3) USER ENVs...");
	{
		// Lab 3 user environment initialization functions
		env_init();
		ts_init();
		//2024: removed. called inside cpuinit()
		//idt_init();
	}
	cprintf("[DONE]\n");

	cprintf("* 4) PROGRAMMABLE INTERRUPT CONTROLLER:\n");
	{
		pic_init();
		cprintf("*	PIC is initialized\n");
		//Enable Clock Interrupt
		irq_clear_mask(0);
		cprintf("*	IRQ0 (Clock): is Enabled\n");
		//Enable KB Interrupt
		irq_clear_mask(1);
		cprintf("*	IRQ1 (Keyboard): is Enabled\n");
		//Enable COM1 Interrupt
		irq_clear_mask(4);
		cprintf("*	IRQ4 (COM1): is Enabled\n");
		//Enable Primary ATA Hard Disk Interrupt
		//		irq_clear_mask(14);
		//		cprintf("*	IRQ14 (Primary ATA Hard Disk): is Enabled\n");
	}
	cprintf("* 5) SCHEDULER & MULTI-TASKING:\n");
	{
		// Lab 4 multitasking initialization functions
		kclock_init();
		sched_init() ;
	}
	//cprintf("* [DONE]\n");

	cprintf("* 6) ESP to SCHED KERN STACK:\n");
	{
		//Relocate SP to its corresponding location in the specific stack area below KERN_BASE (SCHD_KERN_STACK_TOP)
		uint32 old_sp = read_esp();
		uint32 sp_offset = (uint32)ptr_stack_top - old_sp ;
		uint32 new_sp = KERN_STACK_TOP - sp_offset;
		write_esp(new_sp);
		cprintf("*	old SP = %x - updated SP = %x\n", old_sp, read_esp());
	}
	cprintf("********************************************************************\n");

	// start the kernel command prompt.
	autograde = 1;
	while (1==1)
	{
		cprintf("\nWelcome to the FOS kernel command prompt!\n");
		cprintf("Type 'help' for a list of commands.\n");
		if (autograde)
		{
			/*CHECK THE FOLLOWING:
			 * 1) time of each test
			 * 2) "unhandled trap in" message
			 */
			cprintf("\nMS3 Automatic testing is STARTED...\n") ;

			//TEST#1: FAULT HANDLER II [NTH CLOCK NORMAL] 	[10 sec]
			{
				char cmd0[BUFLEN] = "nclock 5 1";
				char cmd1[BUFLEN] = "run tpr1 11";
				char cmd2[BUFLEN] = "run tpr2 6";
				char cmd3[BUFLEN] = "run tnclock1 11";
				char cmdU[BUFLEN] = "run tnclock3 11";

//				execute_command(cmd0);
//				execute_command(cmdU);
			}
			//TEST#2: FAULT HANDLER II [NTH CLOCK MODIFIED] [10 sec]
			{
				char cmd0[BUFLEN] = "nclock 5 2";
				char cmd1[BUFLEN] = "run tpr1 11";
				char cmd2[BUFLEN] = "run tpr2 6";
				char cmd3[BUFLEN] = "run tnclock2 11";

//				execute_command(cmd0);
//				execute_command(cmd3);
			}

			//TEST#3: SEMAPHORES	[30 sec]
			{
				char cmd1[BUFLEN] = "run tsem1 500";	//5 sec
				char cmd2[BUFLEN] = "run tsem2 500";	//20 sec
				char cmdU[BUFLEN] = "run tair 500";		//10 sec
//				execute_command(cmd1);
			}

			//TEST#4: PRIORITY RR SCHEDULER
			{
				char cmd01[BUFLEN] = "schedPRIRR 10 40 1000";
				char cmd03[BUFLEN] = "schedPRIRR 10 40 20";
//				execute_command(cmd03);

				char cmdU1_1[BUFLEN] = "tst priorityRR 0";	//52 sec
				char cmdU2_1[BUFLEN] = "tst priorityRR 1";	//58 sec
				char cmdU3_1[BUFLEN] = "tst priorityRR 2";	//90 sec
//				execute_command(cmdU3_1);
			}

			//TEST#4: BONUSES
			{
				//[1] EXIT I
				{
//					char cmd0_1[BUFLEN] = "fifo";
//					char cmd1_1[BUFLEN] = "run tef1 5";		//4s
//					char cmd2_1[BUFLEN] = "run tef2 15";	//20s
//					//					execute_command(cmd0_1);
//					//					execute_command(cmd2_1);
//
//					char cmd0_2[BUFLEN] = "lru 2";
//					char cmd1_2[BUFLEN] = "run tef1 5 3";	//6s
//					char cmd2_2[BUFLEN] = "run tef2 15 5";	//25s
//					//					execute_command(cmd0_2);
//					//					execute_command(cmd2_2);
				}
				//[2] FREE SHARED OBJECT
				{
					char cmd1[BUFLEN] = "run tshr4 3000";
					char cmd2[BUFLEN] = "run tshr5 3000";
					//execute_command(cmd2);
				}
			}
			//TEST#5: OVERALL SCENARIOS 	[3min]
			{
				char cmd0_1[BUFLEN] = "nclock 10 1";
				char cmd0_2[BUFLEN] = "nclock 5 2";
				execute_command(cmd0_2);

				char cmd1[BUFLEN] = "run sc_MultipleApps 10000";		//45s
				char cmd2[BUFLEN] = "run sc_arrops 10000";				//10s
				char cmd3[BUFLEN] = "run sc_NthClk_Norm_vs_Mod 100";	//1m50s
				//execute_command(cmd3);

				char cmd4_1[BUFLEN] = "schedPRIRR 30 50 200";
				char cmd4_2[BUFLEN] = "run sc_priRR_1 100 29";	//Nth Clock Normal		//1m
				char cmd5_2[BUFLEN] = "run sc_priRR_2 100 29";	//Nth Clock Modified 	//2m25s
				execute_command(cmd4_1);
				execute_command(cmd5_2);
			}
			cprintf("MS3 Automatic testing is ENDED\n") ;
			autograde = 0;
		}
		get_into_prompt();
	}
}


void print_welcome_message()
{
	cprintf("\n\n\n");
	cprintf("\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	cprintf("\t\t!!                                                             !!\n");
	cprintf("\t\t!!                   !! FCIS says HELLO !!                     !!\n");
	cprintf("\t\t!!                                                             !!\n");
	cprintf("\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	cprintf("\n\n\n\n");
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
static const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", exit the curenv and schedule the next environment.
 */
void _panic(const char *file, int line, const char *fmt,...)
{
	struct Env* cur_env = get_cpu_proc();

	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("\nkernel [EVAL_FINAL]panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

	dead:
	/* break into the fos scheduler */
	//2013: Check if the panic occur when running an environment
	if (cur_env != NULL && cur_env->env_status == ENV_RUNNING)
	{
		//cprintf("\n>>>>>>>>>>> exiting the cur env<<<<<<<<<<<<\n");
		//Place the running env into the exit queue then switch to the scheduler
		env_exit(); //env_exit --> sched_exit_env --> sched --> context_switch into fos_scheduler
	}
	//else //2024: panic from Kernel and no current running env
	{
		char* esp = (char*)read_esp();
		cprintf("esp = %x\n", esp);
		get_into_prompt();
	}

}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", exit all env's and then enters the kernel command prompt.
 */
void _panic_all(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("\nkernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

	dead:
	/* break into the command prompt */
	pushcli();
	struct cpu *c = mycpu();
	int sched_stat = c->scheduler_status;
	popcli();
	/*2022*///Check if the scheduler is successfully initialized or not
	if (sched_stat != SCH_UNINITIALIZED)
	{
		//exit all ready env's
		sched_exit_all_ready_envs();
		struct Env* cur_env = get_cpu_proc();
		if (cur_env != NULL && cur_env->env_status == ENV_RUNNING)
		{
			//cprintf("exit curenv...........\n");
			//Place the running env into the exit queue then switch to the scheduler
			env_exit(); //env_exit --> sched_exit_env --> sched --> context_switch into fos_scheduler
		}
	}
	//else //2024: panic from Kernel and no current running env
	{
		get_into_prompt();
	}
}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", exit the curenv (if any) and break into the command prompt.
 */
void _panic_into_prompt(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf("\nkernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

	//	dead:
	/* break into the fos scheduler */
	//2013: Check if the panic occur when running an environment
	struct Env* cur_env = get_cpu_proc();
	if (cur_env != NULL && cur_env->env_status == ENV_RUNNING)
	{
		//Place the running env into the exit queue then switch to the scheduler
		env_exit(); //env_exit --> sched_exit_env --> sched --> context_switch into fos_scheduler
	}

	get_into_prompt();

}


/* like panic, but don't enters the kernel command prompt*/
void _warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("\nkernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}


