        # Check sltiu
        #===========================================================
#include "midgetv.inc"
        .globl _start
#ifndef onlyEBR
        .section .text
#endif
_start:
        addi    x3,x0,1
        addi    x1,x0,22
        sltiu   x2,x1,25        # x2 == 1 because 22 < 25
        bne     x2,x3,1f        # 
        sltiu   x2,x1,2         # x2 == 0 because (22 < 2) false
        bne     x2,x0,1f        #
        sltiu   x2,x1,22        # x2 == 0 because ( 22 < 22) false
        bne     x2,x0,1f

        li      x1,0x10
        sltiu   x2,x1,0x20      #  x2 == 1 because 0x10 < 0x20
        bne     x2,x3,2f        #         
        
        simend
1:      simerr  1
2:      simerr  2
