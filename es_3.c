/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : YS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx.h"
#include "bsp.h"
#include "cpu.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define APP_TASK_EQ_0_ITERATION_NBR 16u
/*
*********************************************************************************************************
*                                            TYPES DEFINITIONS
*********************************************************************************************************
*/
typedef enum
{
    Task_LED,
    Task_TTY,
    Task_BTN,

    TASK_N
} task_e;

typedef struct
{
    CPU_CHAR *name;
    OS_TASK_PTR func;
    OS_PRIO prio;
    CPU_STK *pStack;
    OS_TCB *pTcb;
} task_t;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg);
static void AppTaskCreate(void);
static void AppObjCreate(void);

static void AppTask_LED(void *p_arg);
static void AppTask_TTY(void *p_arg);
static void AppTask_BTN(void *p_arg);

static void Button_Init(void);

CPU_SR_ALLOC();
const char messages[5][64] = {"Btn Push\n\r", "LED1 ON\n\r", "LED2 ON\n\r", "LED3 ON\n\r", "ALL LED OFF\n\r"};

OS_Q TTY_Q, LED_Q;

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static OS_TCB AppTaskStartTCB;
static CPU_STK AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static OS_TCB Task_LED_TCB;
static OS_TCB Task_TTY_TCB;
static OS_TCB Task_BTN_TCB;
static CPU_STK Task_LED_Stack[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK Task_TTY_Stack[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK Task_BTN_Stack[APP_CFG_TASK_START_STK_SIZE];

task_t cyclic_tasks[TASK_N] = {
    {"Task_LED", AppTask_LED, 1, &Task_LED_Stack[0], &Task_LED_TCB},
    {"Task_TTY", AppTask_TTY, 0, &Task_TTY_Stack[0], &Task_TTY_TCB},
    {"Task_BTN", AppTask_BTN, 2, &Task_BTN_Stack[0], &Task_BTN_TCB},
};

int btn_cnt = 0;
int led_pattern[4][3] = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}};
/* ------------ FLOATING POINT TEST TASK -------------- */
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int main(void)
{
    OS_ERR err;

    /* Basic Init */
    RCC_DeInit();
    //    SystemCoreClockUpdate();

    /* BSP Init */
    BSP_IntDisAll(); /* Disable all interrupts.                              */

    CPU_Init();  /* Initialize the uC/CPU Services                       */
    Mem_Init();  /* Initialize Memory Management Module                  */
    Math_Init(); /* Initialize Mathematical Module                       */

    BSP_Init();
    Button_Init();

    /* OS Init */
    OSInit(&err); /* Init uC/OS-III.                                      */

    OSQCreate(&TTY_Q, "TTY QUEUE", 10, &err);
    OSQCreate(&LED_Q, "LED QUEUE", 10, &err);

    OSTaskCreate((OS_TCB *)&AppTaskStartTCB, /* Create the start task                                */
                 (CPU_CHAR *)"App Task Start",
                 (OS_TASK_PTR)AppTaskStart,
                 (void *)0u,
                 (OS_PRIO)APP_CFG_TASK_START_PRIO,
                 (CPU_STK *)&AppTaskStartStk[0u],
                 (CPU_STK_SIZE)AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY)0u,
                 (OS_TICK)0u,
                 (void *)0u,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);

    OSStart(&err); /* Start multitasking (i.e. give control to uC/OS-III). */

    (void)&err;

    return (0u);
}
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg)
{
    OS_ERR err;

    (void)p_arg;

    BSP_Init(); /* Initialize BSP functions                             */
    CPU_Init();

    BSP_Tick_Init(); /* Initialize Tick Services.                            */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err); /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    // BSP_LED_Off(0u);                                            /* Turn Off LEDs after initialization                   */

    APP_TRACE_DBG(("Creating Application Kernel Objects\n\r"));
    AppObjCreate(); /* Create Applicaiton kernel objects                    */

    APP_TRACE_DBG(("Creating Application Tasks\n\r"));
    AppTaskCreate(); /* Create Application tasks                             */
}

static void AppTask_LED(void *p_arg)
{
    OS_ERR err;
    int button = 0;

    while (DEF_TRUE)
    { /* Task body, always written as an infinite loop.       */

        OS_MSG_SIZE size;
        OSQPend(&LED_Q, 0, 0, &size, NULL, &err);
        // Queue pend
        CPU_CRITICAL_ENTER();
        button = btn_cnt % 4;
        CPU_CRITICAL_EXIT();

        switch (button)
        {
        case 0:
            OSQPost(&TTY_Q, (void *)messages[4], strlen(messages[4]), 0, &err);
            BSP_LED_Off(1);
            BSP_LED_Off(2);
            BSP_LED_Off(3);
            break;
        case 1:
            OSQPost(&TTY_Q, (void *)messages[1], strlen(messages[1]), 0, &err);
            BSP_LED_On(1);
            break;
        case 2:
            OSQPost(&TTY_Q, (void *)messages[2], strlen(messages[2]), 0, &err);
            BSP_LED_On(2);
            break;
        case 3:
            OSQPost(&TTY_Q, (void *)messages[3], strlen(messages[3]), 0, &err);
            BSP_LED_On(3);
            break;
        }

        OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

static void AppTask_TTY(void *p_arg)
{
    OS_ERR err;
    OS_MSG_SIZE size;

    while (DEF_TRUE)
    {
        char *event = OSQPend(&TTY_Q, 0, 0, &size, (void *)0, &err);
        send_string(event);
        if (strncmp(event, "Btn", 3) == 0)
        {
            OSQPost(&LED_Q, (void *)messages[0], strlen(messages[0]), 0, &err);
        }
    }
}

static void AppTask_BTN(void *p_arg)
{
    OS_ERR err;
    int button = 0;

    while (DEF_TRUE)
    { /* Task body, always written as an infinite loop.       */

        button = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
        CPU_CRITICAL_ENTER();
        btn_cnt += button;
        CPU_CRITICAL_EXIT();

        if (button)
        {
            OSQPost(&TTY_Q, (void *)messages[0], strlen(messages[0]), 0, &err);
        }
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void AppTaskCreate(void)
{
    OS_ERR err;

    u8_t idx = 0;
    task_t *pTask_Cfg;
    for (idx = 0; idx < TASK_N; idx++)
    {
        pTask_Cfg = &cyclic_tasks[idx];

        OSTaskCreate(
            pTask_Cfg->pTcb,
            pTask_Cfg->name,
            pTask_Cfg->func,
            (void *)0u,
            pTask_Cfg->prio,
            pTask_Cfg->pStack,
            pTask_Cfg->pStack[APP_CFG_TASK_START_STK_SIZE / 10u],
            APP_CFG_TASK_START_STK_SIZE,
            (OS_MSG_QTY)0u,
            (OS_TICK)0u,
            (void *)0u,
            (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            (OS_ERR *)&err);
    }
}

/*
*********************************************************************************************************
*                                          AppObjCreate()
*
* Description : Create application kernel objects tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void AppObjCreate(void)
{
}
/*
*********************************************************************************************************
*                                          Setup_Gpio()
*
* Description : Configure LED GPIOs directly
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     :
*              LED1 PB0
*              LED2 PB7
*              LED3 PB14
*
*********************************************************************************************************
*/

static void Button_Init(void)
{
    GPIO_InitTypeDef btn_init = {0};

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    btn_init.GPIO_Mode = GPIO_Mode_IN;
    btn_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    btn_init.GPIO_Speed = GPIO_Speed_2MHz;
    btn_init.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &btn_init);
}
