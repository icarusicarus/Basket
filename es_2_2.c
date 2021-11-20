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
    Task_LED1,
    Task_LED2,
    Task_LED3,
    USART,

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

static void AppTask_LED1(void *p_arg);
static void AppTask_LED2(void *p_arg);
static void AppTask_LED3(void *p_arg);
static void USART_Test(void *p_arg);

static void Setup_Gpio(void);

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static OS_TCB AppTaskStartTCB;
static CPU_STK AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static OS_TCB Task_LED1_TCB;
static OS_TCB Task_LED2_TCB;
static OS_TCB Task_LED3_TCB;
static OS_TCB USART_Test_TCB;

static CPU_STK Task_LED1_Stack[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK Task_LED2_Stack[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK Task_LED3_Stack[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK USART_Test_Stack[APP_CFG_TASK_START_STK_SIZE];
int count = 0;
task_t cyclic_tasks[TASK_N] = {
    {"Task_LED1", AppTask_LED1, 0, &Task_LED1_Stack[0], &Task_LED1_TCB},
    {"Task_LED2", AppTask_LED2, 0, &Task_LED2_Stack[0], &Task_LED2_TCB},
    {"Task_LED3", AppTask_LED3, 0, &Task_LED3_Stack[0], &Task_LED3_TCB},
    {"USART", USART_Test, 1, &USART_Test_Stack[0], &USART_Test_TCB},
};
char c;
char command[256] = {
    0,
};
u16_t led_on_off[3] = {0, 0, 0};
//u16_t led_off[3] = {0, 0, 0};
u16_t led_blink[3] = {0, 0, 0};

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
    Setup_Gpio();

    /* BSP Init */
    BSP_IntDisAll(); /* Disable all interrupts.                              */

    CPU_Init();  /* Initialize the uC/CPU Services                       */
    Mem_Init();  /* Initialize Memory Management Module                  */
    Math_Init(); /* Initialize Mathematical Module                       */

    /* OS Init */
    OSInit(&err); /* Init uC/OS-III.                                      */

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

    BSP_Init();      /* Initialize BSP functions                             */
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

/*
*********************************************************************************************************
*                                          AppTask_LED1
*
* Description : Example of 500mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void AppTask_LED1(void *p_arg)
{
    OS_ERR err;

    while (DEF_TRUE)
    {
        if (led_on_off[0] == 1)
            BSP_LED_On(1);
        else
            BSP_LED_Off(1);

        if (led_blink[0] != 0)
        {
            u16_t period = led_blink[0];
            BSP_LED_Toggle(1);
            OSTimeDlyHMSM(0u, 0u, 0u, period,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        }
        OSTimeDlyHMSM(0u, 0u, 0u, 1u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

/*
*********************************************************************************************************
*                                          AppTask_LED2
*
* Description : Example of 1000mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void AppTask_LED2(void *p_arg)
{
    OS_ERR err;

    while (DEF_TRUE)
    {
        if (led_on_off[1] == 1)
            BSP_LED_On(2);
        else
            BSP_LED_Off(2);

        if (led_blink[1] != 0)
        {
            u16_t period = led_blink[1];
            BSP_LED_Toggle(2);
            OSTimeDlyHMSM(0u, 0u, 0u, period,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        }
        OSTimeDlyHMSM(0u, 0u, 0u, 1u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

/*
*********************************************************************************************************
*                                          AppTask_LED3
*
* Description : Example of 2000mS Task
*
* Arguments   : p_arg (unused)
*
* Returns     : none
*
* Note: Long period used to measure timing in person
*********************************************************************************************************
*/
static void AppTask_LED3(void *p_arg)
{
    OS_ERR err;

    while (DEF_TRUE)
    {
        if (led_on_off[2] == 1)
            BSP_LED_On(3);
        else
            BSP_LED_Off(3);

        if (led_blink[2] != 0)
        {
            u16_t period = led_blink[2];
            BSP_LED_Toggle(3);
            OSTimeDlyHMSM(0u, 0u, 0u, period,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        }
        OSTimeDlyHMSM(0u, 0u, 0u, 1u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}

static void USART_Test(void *p_arg)
{
    OS_ERR err;
    volatile int index = 0;
    while (DEF_TRUE)
    {
        while (USART_GetFlagStatus(Nucleo_COM1, USART_FLAG_RXNE) == RESET)
        {
            OSTimeDlyHMSM(0u, 0u, 0u, 1u,
                          OS_OPT_TIME_HMSM_STRICT,
                          &err);
        }
        c = USART_ReceiveData(Nucleo_COM1);
        USART_SendData(Nucleo_COM1, c);
        if (c == '\r')
            break;
        command[index] = c;
        index++;
        OSTimeDlyHMSM(0u, 0u, 0u, 1u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
    send_string("\n\r");
    char buffer[65] = {0};
    send_string(itoa(index, buffer, 10));
    send_string("\n\r");
    send_string(command);
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
static void Setup_Gpio(void)
{
    GPIO_InitTypeDef led_init = {0};

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    led_init.GPIO_Mode = GPIO_Mode_OUT;
    led_init.GPIO_OType = GPIO_OType_PP;
    led_init.GPIO_Speed = GPIO_Speed_2MHz;
    led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    led_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;

    GPIO_Init(GPIOB, &led_init);
}
