#include "midgetv.inc"
        .globl _start
        .section .text
_start: 
        j       _midgetstart
#include "midgetv_dump_compliance.S"
#include "I-DELAY_SLOTS-01.S"
