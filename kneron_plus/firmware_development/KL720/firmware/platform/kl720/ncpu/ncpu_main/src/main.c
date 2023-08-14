
/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#ifndef TARGET_CUSTOM
#include "npu_log.h"
#endif
#include "xtensa_api.h"
#include "ncpu_main.h"

boolean gInIdle;

void vAssertCalled( unsigned long ulLine, const char * const pcFileName );

static void  prvInitialiseHeap( void );

void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/*
 * Writes trace data to a disk file when the trace recording is stopped.
 * This function will simply overwrite any trace files that already exist.
 */
static void prvSaveTraceFile( void );

/*-----------------------------------------------------------*/

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
use a callback function to optionally provide the memory required by the idle
and timer tasks.  This is the stack that will be used by the timer task.  It is
declared here, as a global, so it can be checked by a test that is implemented
in a different file. */
StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

/* Notes if the trace is running or not. */
static BaseType_t xTraceRunning = pdTRUE;

/*-----------------------------------------------------------*/

int main( void )
{
    /* This demo uses heap_5.c, so start by defining some heap regions.  heap_5
    is only used for test and example reasons.  Heap_4 is more appropriate.  See
    http://www.freertos.org/a00111.html for an explanation. */
    prvInitialiseHeap();

    ncpu_init();
    ncpu_main();
	gInIdle = FALSE;

    /* Finally start the scheduler. */
    vTaskStartScheduler();

    /* Will only reach here if there is insufficient heap available to start
       the scheduler. */
    for( ;; );

    return 0;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
    size of the heap available to pvPortMalloc() is defined by
    configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
    API function can be used to query the size of free heap space that remains
    (although it does not provide information on how the remaining heap might be
    fragmented).  See http://www.freertos.org/a00111.html for more
    information. */
    vAssertCalled( __LINE__, __FILE__ );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If application tasks make use of the
    vTaskDelete() API function to delete themselves then it is also important
    that vApplicationIdleHook() is permitted to return to its calling function,
    because it is the responsibility of the idle task to clean up memory
    allocated by the kernel to any task that has since deleted itself. */

    /* Uncomment the following code to allow the trace to be stopped with any
    key press.  The code is commented out by default as the kbhit() function
    interferes with the run time behaviour. */
    /*
        if( _kbhit() != pdFALSE )
        {
            if( xTraceRunning == pdTRUE )
            {
                vTraceStop();
                prvSaveTraceFile();
                xTraceRunning = pdFALSE;
            }
        }
    */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  This function is
    provided as an example only as stack overflow checking does not function
    when running the FreeRTOS Windows port. */
    vAssertCalled( __LINE__, __func__ );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()). */

}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    /* This function will be called once only, when the daemon task starts to
    execute (sometimes called the timer task).  This is useful if the
    application includes initialisation code that would benefit from executing
    after the scheduler has been started. */
}
/*-----------------------------------------------------------*/

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{

    static BaseType_t xPrinted = pdFALSE;
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
    http://www.freertos.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    ( void ) ulLine;
    ( void ) pcFileName;

    taskENTER_CRITICAL();
    {
        /* Stop the trace recording. */
        if( xPrinted == pdFALSE ) {
            xPrinted = pdTRUE;
            if( xTraceRunning == pdTRUE ) {
                prvSaveTraceFile();
            }
        }

        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 ) {
            __asm__ volatile ("nop.n");
            __asm__ volatile ("nop.n");

        }
    }
    taskEXIT_CRITICAL();
//#endif

}
/*-----------------------------------------------------------*/

static void prvSaveTraceFile( void )
{

}
/*-----------------------------------------------------------*/

static void  prvInitialiseHeap( void )
{

}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#if configUSE_TICKLESS_IDLE == 1
static unsigned long ulGetExternalTime(void)
{
	return 0;
}

static void prvStartTickInterruptTimer(void)
{
    xt_ints_on(1 << XCHAL_TIMER0_INTERRUPT);
}

static void prvStopTickInterruptTimer(void)
{
    xt_ints_off(1 << XCHAL_TIMER0_INTERRUPT);
    gInIdle = TRUE;
}

static void disable_interrupts(void)
{
	portDISABLE_INTERRUPTS();
}

static void enable_interrupts(void)
{
	portENABLE_INTERRUPTS();
}

static void prvSleep(void)
{
    __asm__ volatile ("waiti 0");
}

static void vSetWakeTimeInterrupt(TickType_t xExpectedIdleTime)
{
}

void vApplicationSleep( TickType_t xExpectedIdleTime )
{
	unsigned long ulLowPowerTimeBeforeSleep, ulLowPowerTimeAfterSleep;
	eSleepModeStatus eSleepStatus;

	/* Read the current time from a time source that will remain operational
	while the microcontroller is in a low power state. */
	ulLowPowerTimeBeforeSleep = ulGetExternalTime();

	/* Stop the timer that is generating the tick interrupt. */
	prvStopTickInterruptTimer();

	//critical_msg("Idle: entered\n");

	/* Enter a critical section that will not effect interrupts bringing the MCU
	out of sleep mode. */
	disable_interrupts();

	/* Ensure it is still ok to enter the sleep mode. */
	eSleepStatus = eTaskConfirmSleepModeStatus();

	if( eSleepStatus == eAbortSleep )
	{
		/* A task has been moved out of the Blocked state since this macro was
		executed, or a context siwth is being held pending.  Do not enter a
		sleep state.  Restart the tick and exit the critical section. */
		prvStartTickInterruptTimer();
		enable_interrupts();
		//critical_msg("Idle: aborted\n");
	}
	else
	{
		if( eSleepStatus == eNoTasksWaitingTimeout )
		{
			/* It is not necessary to configure an interrupt to bring the
			microcontroller out of its low power state at a fixed time in the
			future. */
			prvSleep();
			//critical_msg("Idle: woken up\n");
		}
		else
		{
			/* Configure an interrupt to bring the microcontroller out of its low
			power state at the time the kernel next needs to execute.  The
			interrupt must be generated from a source that remains operational
			when the microcontroller is in a low power state. */
			vSetWakeTimeInterrupt( xExpectedIdleTime );

#ifndef TARGET_CUSTOM
			profile_msg("Idle: for %d\n", xExpectedIdleTime);
#endif
			/* Enter the low power state. */
			prvSleep();

			/* Determine how long the microcontroller was actually in a low power
			state for, which will be less than xExpectedIdleTime if the
			microcontroller was brought out of low power mode by an interrupt
			other than that configured by the vSetWakeTimeInterrupt() call.
			Note that the scheduler is suspended before
			portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
			portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
			execute until this function completes. */
			ulLowPowerTimeAfterSleep = ulGetExternalTime();

			/* Correct the kernels tick count to account for the time the
			microcontroller spent in its low power state. */
			vTaskStepTick( ulLowPowerTimeAfterSleep - ulLowPowerTimeBeforeSleep );
		}

		/* Exit the critical section - it might be possible to do this immediately
		after the prvSleep() calls. */
		enable_interrupts();

		/* Restart the timer that is generating the tick interrupt. */
		prvStartTickInterruptTimer();
	}
}
#endif

/* dummy function for performance trace */
uint32_t osKernelGetTickCount(void)
{
    TickType_t xTime;
    xTime = xTaskGetTickCount();
    return (uint32_t)xTime;
}

