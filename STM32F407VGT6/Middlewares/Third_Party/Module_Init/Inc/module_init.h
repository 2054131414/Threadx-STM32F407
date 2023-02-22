/* USER CODE BEGIN Header */
/**
 * ****************************************************************************
 * @file    auto_init.h 
 * @brief   System initialization starts
 * ****************************************************************************
 * @attention
 * 
 * Copyright (c) 2022, Mingliang.sui.
 * All rights reserved.
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#ifndef _MODULE_INIT_H_
#define _MODULE_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported defines ----------------------------------------------------------*/

/*===================Automatically initialize the interface===================*/
#if defined(__CC_ARM)||(defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)        /* ARM Compiler */
    #define SECTION(x)                  __attribute__((section(x)))

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #define SECTION(x)                  @ x

#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #define SECTION(x)                  __attribute__((section(x)))

#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
    #define SECTION(x)                  __attribute__((section(x)))

#else
    #error not supported tool chain
#endif


#if defined(__CC_ARM)||(defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)        /* ARM Compiler */
#define TX_INITCALL_USED                 __attribute__((used))

#elif defined(__IAR_SYSTEMS_ICC__)       /* for IAR Compiler */
    #define TX_INITCALL_USED             __root

#elif defined(__GNUC__)                  /* GNU GCC Compiler */
    #define TX_INITCALL_USED             __attribute__((used))

#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
    #define SECTION(x)                   __attribute__((section(x)))

#else
#define INITCALL_USED
#endif


typedef int (*initcall_t)(void);

#if defined(__CC_ARM)||(defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)       /* ARM Compiler */
#define TX_INIT_EXPORT(fn, level)  \
                        TX_INITCALL_USED const initcall_t _init_call_##fn SECTION("init_call" level) = fn

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
#define TX_INIT_EXPORT(fn, level)  \
                        TX_INITCALL_USED const initcall_t _init_call_##fn SECTION("init_call" level) = fn

#elif defined (__GNUC__)                /* GNU GCC Compiler */
#define TX_INIT_EXPORT(fn, level)  \
                        TX_INITCALL_USED const initcall_t _init_call_##fn SECTION("init_call" level) = fn

#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
#define TX_INIT_EXPORT(fn, level)  \
                        TX_INITCALL_USED const initcall_t _init_call_##fn SECTION("init_call" level) = fn
#else
#define TX_INIT_EXPORT(fn, level)                       
#endif

#define TX_INIT_BOARD_EXPORT(fn)           TX_INIT_EXPORT(fn, "1")    //System clock initialization

#define TX_INIT_PREV_EXPORT(fn)            TX_INIT_EXPORT(fn, "2")    //On-board peripheral initialization

#define TX_INIT_DEVICE_EXPORT(fn)          TX_INIT_EXPORT(fn, "3")    //Device interface initialization

#define TX_INIT_COMPONENT_EXPORT(fn)       TX_INIT_EXPORT(fn, "4")    //Driver initialization

#define TX_INIT_ENV_EXPORT(fn)             TX_INIT_EXPORT(fn, "5")    //Environment initialization

#define TX_INIT_APP_EXPORT(fn)             TX_INIT_EXPORT(fn, "6")    //Application initialization

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void TX_COMPONENTS_BOARD_INIT(void);

void TX_COMPONENTS_INIT(void);


#ifdef __cplusplus
}
#endif

#endif /* _MODULE_INIT_H_ */
