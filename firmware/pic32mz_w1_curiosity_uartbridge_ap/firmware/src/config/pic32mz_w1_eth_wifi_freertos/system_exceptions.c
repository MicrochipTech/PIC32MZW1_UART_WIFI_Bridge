/*******************************************************************************
  MPLAB Harmony Exceptions Source File

  File Name:
    system_exceptions.c

  Summary:
    This file contains a function which overrides the deafult _weak_ exception
    handler provided by the XC32 compiler.

  Description:
    This file redefines the default _weak_  exception handler with a more debug
    friendly one. If an unexpected exception occurs the code will stop in a
    while(1) loop.  The debugger can be halted and two variables _excep_code and
    _except_addr can be examined to determine the cause and address where the
    exception occured.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2017 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


#include <xc.h>                 /* Defines special function registers, CP0 regs  */
#include "system_config.h"
#include "system_definitions.h"
#include "system/debug/sys_debug.h"
#include "tcpip/src/tcpip_private.h"
#include "../../vt100.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Exception Reason Data

  <editor-fold defaultstate="expanded" desc="Exception Reason Data">

  Remarks:
    These global static items are used instead of local variables in the
    _general_exception_handler function because the stack may not be available
    if an exception has occured.
 */

/* Code identifying the cause of the exception (CP0 Cause register). */
static unsigned int _excep_code;

/* Address of instruction that caused the exception. */
static unsigned int _excep_addr;

/* Pointer to the string describing the cause of the exception. */
static char *_cause_str;

/* Array identifying the cause (indexed by _exception_code). */
static char *cause[] = {
    "Interrupt",
    "Undefined",
    "Undefined",
    "Undefined",
    "Load/fetch address error",
    "Store address error",
    "Instruction bus error",
    "Data bus error",
    "Syscall",
    "Breakpoint",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

#define AT_REG  1
#define V0_REG  2
#define V1_REG  3
#define A0_REG  4  
#define A1_REG  5
#define A2_REG  6 
#define A3_REG  7
#define T0_REG  8 
#define T1_REG  9
#define T2_REG  10
#define T3_REG  11
#define T4_REG  12
#define T5_REG  13
#define T6_REG  14
#define T7_REG  15
#define T8_REG  16
#define T9_REG  17
#define RA_REG  18
#define T0_LOW_REG  19
#define T0_HIG_REG  20            
#define FP_REG 21
#define GP_REG 22
#define K0_REG 23
#define K1_REG 24
#define S0_REG 25
#define S1_REG 26
#define S2_REG 27                
#define S3_REG 28
#define S4_REG 29
#define S5_REG 30
#define S6_REG 31
#define S7_REG 32
#define SP_REG 33

EXCEPT_MSG last_expt_msg __attribute__((persistent));



uint32_t get_last_expt_msg(void) {

    //    sprintf(last_expt_msg.msg,"\n\rHalloechen... \n\r");
    //    last_expt_msg.magic = MAGIC_CODE;

    if (last_expt_msg.magic == MAGIC_CODE) {
        last_expt_msg.magic = 0x0;
        return (uint32_t)&last_expt_msg.msg[0];
    } else {
        return 0;
    }
}


// </editor-fold>


// *****************************************************************************
// *****************************************************************************
// Section: Exception Handling
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void _general_exception_handler ( void )

  Summary:
    Overrides the XC32 _weak_ _generic_exception_handler.

  Description:
    This function overrides the XC32 default _weak_ _generic_exception_handler.

  Remarks:
    Refer to the XC32 User's Guide for additional information.
 */


void _general_exception_handler(void) {
    //    char str[4096];
    //    int len;
    //    int ix;
    uint32_t *pul;
    volatile uint32_t s5;
    asm volatile("addu %0,$0,$21" : "=r" (s5));

    volatile uint32_t sp;
    asm volatile("addu %0,$0,$27" : "=r" (sp));
    pul = (uint32_t*) (sp - 140);

    /* Mask off Mask of the ExcCode Field from the Cause Register
     Refer to the MIPs Software User's manual */
    _excep_code = (_CP0_GET_CAUSE() & 0x0000007C) >> 2;
    _excep_addr = _CP0_GET_EPC();
    _cause_str = cause[_excep_code];

    sprintf(last_expt_msg.msg,            
            "===> General Exception <===\n\r"            
            "%s (cause=%d, addr=%08x)\r\n"
            "v0=%08x v1=%08x a0=%08x a1=%08x\n\r"
            "a2=%08x a3=%08x ra=%08x s5=%08x\r\n"
            "t0=%08x t1=%08x t2=%08x t3=%08x\n\r"
            "t4=%08x t5=%08x t6=%08x t7=%08x\n\r"
            "t8=%08x t9=%08x k0=%08x k1=%08x\n\r"
            "fp=%08x gp=%08x s0=%08x s1=%08x\n\r"
            "s2=%08x s3=%08x s4=%08x s5=%08x\n\r"
            "s6=%08x s7=%08x sp=%08x            "                        
            ,
            _cause_str, _excep_code, _excep_addr,
            pul[V0_REG], pul[V1_REG], pul[A0_REG], pul[A1_REG],
            pul[A2_REG], pul[A3_REG], pul[RA_REG], s5,
            pul[T0_REG], pul[T1_REG], pul[T2_REG], pul[T3_REG],
            pul[T4_REG], pul[T5_REG], pul[T6_REG], pul[T7_REG],
            pul[T8_REG], pul[T9_REG], pul[K0_REG], pul[K1_REG],
            pul[FP_REG], pul[GP_REG], pul[S0_REG], pul[S1_REG],
            pul[S2_REG], pul[S3_REG], pul[S4_REG], pul[S5_REG],
            pul[S6_REG], pul[S7_REG], pul[SP_REG]
            );

//    sprintf(last_expt_msg.msg,
//            VT100_FOREGROUND_RED
//            "\r\n===> General Exception <===\n\r"
//            VT100_FOREGROUND_CYAN
//            "%s (cause=%d, addr=%08x)\r\n"
//            "v0=%08x v1=%08x a0=%08x a1=%08x\n\r"
//            "a2=%08x a3=%08x ra=%08x s5=%08x\r\n"
//            "stack pointer=%08x\r\n"
//            VT100_FOREGROUND_GREEN
//            VT100_FOREGROUND_GREEN
//            VT100_FOREGROUND_GREEN,
//            _cause_str, _excep_code, _excep_addr,
//            pul[V0_REG], pul[V1_REG], pul[A0_REG], pul[A1_REG],
//            pul[A2_REG], pul[A3_REG], pul[RA_REG], pul[S5_REG],
//            pul[SP_REG]
//            );

    last_expt_msg.magic = MAGIC_CODE;
//
//    int len = strlen(last_expt_msg.msg);
//
//    U1STAbits.UTXEN = 0;
//    volatile uint32_t dummy = 0xFFFFFF;
//    while (dummy--);
//
//    /* Set up UxMODE bits */
//    U1MODE = 0x0;
//    /* Enable UART1 Transmitter */
//    U1STAbits.UTXEN = 1;
//    /* BAUD Rate register Setup */
//    /* 115200 at 200MHz CPU Clock */
//    U1BRG = 53;
//    /* Disable Interrupts */
//    IEC1bits.U1TXIE = 0;
//    /* Turn ON UART1 */
//    U1MODESET = _U1MODE_ON_MASK;
//
//    int ix = 0;
//    while (len) {
//        while (U1STAbits.UTXBF == 1);
//        U1TXREG = last_expt_msg.msg[ix++];
//        len--;
//    }

    while (1) {
        SYSKEY = 0x00000000;
        SYSKEY = 0xAA996655;
        SYSKEY = 0x556699AA;
        RSWRSTSET = _RSWRST_SWRST_MASK;
        RSWRST;
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

void _simple_tlb_refill_exception_handler(void) {
    static unsigned int badInstAddr;
    char str[4096];
    int len;
    int ix;

    badInstAddr = _CP0_GET_NESTEDEPC();

    sprintf(str, "\r\n\r\nTLB Refill Exception at %x \r\n", badInstAddr);

    len = strlen(str);

    /* Set up UxMODE bits */
    U1MODE = 0x0;
    /* Enable UART1 Transmitter */
    U1STAbits.UTXEN = 1;
    /* BAUD Rate register Setup */
    /* 115200 at 200MHz CPU Clock */
    U1BRG = 53;
    /* Disable Interrupts */
    IEC1bits.U1TXIE = 0;
    /* Turn ON UART1 */
    U1MODESET = _U1MODE_ON_MASK;

    ix = 0;
    while (len) {
        while (U1STAbits.UTXBF == 1);
        U1TXREG = str[ix++];
        len--;
    }

    while (1) {
        SYSKEY = 0x00000000;
        SYSKEY = 0xAA996655;
        SYSKEY = 0x556699AA;
        RSWRSTSET = _RSWRST_SWRST_MASK;
        RSWRST;
        Nop();
        Nop();
        Nop();
        Nop();
    }
}

/*******************************************************************************
 End of File
 */
