
#include "xc.h"
#include <cp0defs.h>

        ##################################################################
        # General Exception Vector Handler
        # Jumps to _general_exception_context
        ##################################################################
        .section .gen_handler,code
        .align 2
        .set noreorder
        .ent _gen_exception
        
_gen_exception:
0:      addu    k1, zero, sp
        la      k0,_general_exception_context
        jr      k0
        nop
       
        .end _gen_exception

        
        .extern _general_exception_handler

        .section .text.general_exception, code
        .set noreorder
        .set noat
        .set nomips16
        .globl _general_exception_context
        .ent _general_exception_context

_general_exception_context:

        # Save off the non-callee saved registers that may get mucked with
        addiu sp, sp, -140
        sw $1,  4(sp)
        sw v0,  8(sp)
        sw v1, 12(sp)
        sw a0, 16(sp)
        sw a1, 20(sp)
        sw a2, 24(sp)
        sw a3, 28(sp)
        sw t0, 32(sp)
        sw t1, 36(sp)
        sw t2, 40(sp)
        sw t3, 44(sp)
        sw t4, 48(sp)
        sw t5, 52(sp)
        sw t6, 56(sp)
        sw t7, 60(sp)
        sw t8, 64(sp)
        sw t9, 68(sp)
        sw ra, 72(sp)
        mflo t0
        sw t0, 76(sp)
        mfhi t0
        sw t0, 80(sp)
        sw fp, 84(sp)
        sw gp, 88(sp)
        sw k0, 92(sp)
        sw k1, 96(sp)
        sw s0, 100(sp)
        sw s1, 104(sp)
        sw s2, 108(sp)                
        sw s3, 112(sp)
        sw s4, 116(sp)
        sw s5, 120(sp)
        sw s6, 124(sp)                
        sw s7, 128(sp)
        sw sp, 132(sp)
                
#if defined(__PIC__)
        lw      t9,%call16(_general_exception_handler)(gp)
        nop
        # Pass Cause and Status to the handler function
        mfc0 a0, _CP0_CAUSE
        mfc0 a1, _CP0_STATUS
        jalr    t9
        nop
#else
        la      k0,_general_exception_handler
        nop
   
        # Pass Cause and Status to the handler function
        mfc0 a0, _CP0_CAUSE
        mfc0 a1, _CP0_STATUS
        jalr    k0
        nop
#endif
                
        lw $1,  4(sp)
        lw v0,  8(sp)
        lw v1, 12(sp)
        lw a0, 16(sp)
        lw a1, 20(sp)
        lw a2, 24(sp)
        lw a3, 28(sp)
        # lw t0, 32(sp)
        lw t1, 36(sp)
        lw t2, 40(sp)
        lw t3, 44(sp)
        lw t4, 48(sp)
        lw t5, 52(sp)
        lw t6, 56(sp)
        lw t7, 60(sp)
        lw t8, 64(sp)
        lw t9, 68(sp)
        lw ra, 72(sp)

        lw t0, 76(sp)
        mtlo t0
        lw t0, 80(sp)
        mthi t0                
                
        lw t0, 32(sp)        
                                
        lw fp, 84(sp)
        lw gp, 88(sp)
        lw k0, 92(sp)
        lw k1, 96(sp)
        lw s0, 100(sp)
        lw s1, 104(sp)
        lw s2, 108(sp)                
        lw s3, 112(sp)
        lw s4, 116(sp)
        lw s5, 120(sp)
        lw s6, 124(sp)                
        lw s7, 128(sp)
        lw sp, 132(sp)
                
        addiu sp, sp, 140

        ehb
        eret

        .end _general_exception_context  