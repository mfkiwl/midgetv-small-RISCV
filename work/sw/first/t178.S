        # CSRRWI
        # The smallest midgetv have no CSR, but the instruction is decoded 
        # nevertheless. This is to check the decoding
        # =============================================================================
#include "midgetv.inc"
        .globl _start
        # =============================================================================
        # Entry point for CSR instructions
        .org 0x108
        lw      x1,%lo(__jj)(x0)        // CSR address
        lw      x2,%lo(__yy)(x0)        // CSR type
        li      x3,0xfffffabc
        bne     x1,x3,1f
        li      x4,0xffffff80
        bne     x2,x4,2f        
        simend
        
        
1:      simerr  1
2:      simerr  2
3:      simerr  3
4:      simerr  4
5:      simerr  5
6:      simerr  6

#ifndef onlyEBR
        .section .text
#endif
_start:
        csrrwi x1,0xabc,3
        simdump
        simerr 33
