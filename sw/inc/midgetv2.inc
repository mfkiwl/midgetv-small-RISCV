        // =============================================================================
        // Part of midgetv
        // 2019. Copyright B. Nossum.
        // For licence, see LICENCE
        // =============================================================================
        
        // =============================================================================
        // First a custom-0 instruction "ij"
        // This is an indirect jump. Ideally the syntax should have been
        // "ij rd,ofs(rs1)" but I do not know how to make such a macro. Hence we have
        // "ij rdnr,ofs,rs1nr" instead. The motivation for the instruction is to
        // be able to return to a 32-bit address stored in memory, "pc = mem[ofs+rs1]",
        // this make CSR emulation code and MRET emulation code much more agile. 
        // See "ucode.h" for details.
        // 
        .macro ij rdnr=0,ofs=0,rs1nr=0
        .word (((\ofs) & 0xfff)<<20) + (((\rs1nr) & 31)<<15) + (((\rdnr) & 31)<<7) + 0b0001011
        .endm
        
        // =============================================================================

        // =============================================================================        
        //
        // midgetv relies on magical constants in the start of EBR.
        // Overwriting these constants will crash midget. Writing to address 0 is
        // the same as writing to register x0, and so will break the programs, as
        // the assemblers rightfully relies on x0 == 0. Writing directly to __pc will 
        // fail for a subtle reason, in the microcode one will then try to write and 
        // read a location in EBR in the same clock cycle. This is undefined.
        // In all, there are 8 words that should never be written.
        // 
        // There are a number of locations that can be written, but usually are not,
        // for example one can directly change a register xn by writing a word to
        // mem[4*n], software implemented multiple psh/pop anyone?
        //
        // =============================================================================
        .section .magicconstants, "a"
        .option norelax //           Never
        .org 0x0 //                  write
STARTOFEBR: //                       these   // Wordaaddress   Wai   
        .word 0   //                     *   // 0x00 00000000  0x00 Register x0      
        .word 0   //                     .   // 0x04 00000100  0x01 Register x1
        .word magic_x2    //             .   // 0x04 00000100  0x01 Register x2 == sp
                  //                     .   // :::
                  //                     .   // 0x7c 01111100  0x1e Register x31
        .org 0x80 //                     .   //
__jj:           .word 0  //              .   // 0x80 10000000  0x20 ucode register
__rinst:        .word 0  //              .   // 0x84 10000100  0x21 32-lsb of instret counter
__pc:           .word magic_start-4   // *   // 0x88 10001000  0x22 Program counter
__mcycle:       .word 0               // .   // 0x8c 10001100  0x23 32-lsb of cycle counter
__rNMI_IIV:     .word magic_NMI       // .   // 0x90 10010000  0x24 Must point to handlers for NMI/Internal interrupts
cteffffff7f:    .word 0xffffff7f      // *   // 0x94 10010100  0x25 For sign extension in "lb"
cte000000ff:    .word 0x000000ff      // *   // 0x98 10011000  0x26 Mask in "lb", "lbu", also for entrypoint CSR
cte0000ffff:    .word 0x0000ffff      // *   // 0x9c 10011100  0x27 Mask in "lh" and "lhu"
cteffff7fff:    .word 0xffff7fff      // *   // 0xa0 10100000  0x28 For sign extension in "lh"
__mtvec:        .word magic_trap      // .   // 0xa4 10100100  0x29 Should point to excption handler.
cte00000000:    .word 0x00000000      // *   // 0xa8 10101000  0x2a ucode constant. 
cteffffffff:    .word 0xffffffff      // *   // 0xac 10101100  0x2b ucode constant.
__yy:           .word 0                      // 0xb0 10110000  0x2c ucode register.
__mepc:         .word 0                      // 0xb4 10110100  0x2d For exception handling
__mcause:       .word 0                      // 0xb8 10111000  0x2e For exception handling
__mtval:        .word 0                      // 0xbc 10111100  0x2f For exception handling
__mscratch:     .word 0                      // 0xc0 Used by CSR code
__misa:         .word 0x40800100             // 0xc4 Used by CSR code. RV32I base ISA, nonstd. ext. present, XLEN = 32.
_ex_dup_x0:     .word 0 	             // 0xc8 | Used by CSR emulation. These
_ex_rCSRadr:    .word 1 	             // 0xcc | locations must be consequtive
_ex_rCSRty:     .word 2 	             // 0xd0 | 
_ex_rrs1:       .word 3 	             // 0xd4 | 
_ex_rrd:        .word 4 	             // 0xd8 | 
_ex_rtmp:       .word 5 	             // 0xdc | 
_mtime:         .word 0 	             // 0xe0 Read/write from user code. Updated by internal interrupt
_mtimeh:        .word 0 	             // 0xe4 Read/write from user code. Updated by internal interrupt
__freefornow_e8:.word 0                      // 0xe8 Free location
_minstreth:     .word 0 	             // 0xec Read/write from user code. Updated by internal interrupt
_mcycleh:       .word 0 	             // 0xf0 Read/write from user code. Updated by internal interrupt
__freefornow_f4:.word 0                      // 0xf4
_mtimecmp:      .word 0 	             // 0xf8 Read/write from user code. 
_mtimecmph:     .word 0 	             // 0xfc Read/write from user code. 
        .org 0x100  		             //
CSRretadr:      .word 0		             //0x100 Microcode writes PC to CSRretadr when a CSR instruction is seen.
Entry_xRET:     ij      0,0xb6,0             //0x104 Microcode branches to here when MRET/SRET/URET is seen
EntryCSR:                                    //0x108 Microcode branches to here when a CSR instruction is seen


#define changeme_mtime 0xe0
#define changeme_mtimeh 0xe4        
        
//        .section .EBRram, "a"
        // =============================================================================
        // Midgetv is usually implemented with some SRAM, but can be used without.
        // For these very constrained implementations it should be considered to
        // remove system support for CSR instructions, and simplify or eliminate
        // interrupts.
        //
        // Memory map:
        // ===========
        // Coarse regions:
        // 0x3fffffff-0x00000000 EBR               00
        // 0x7fffffff-0x40000000 IO                01
        // 0xffffffff-0x80000000 SRAM              1x
        //
        // Memory map of the start of EBR
        // ==============================
        //
        // 0x07f-0x000 Registers/code/data. Writing anything other than 0 (with
        //             SW/SH/SB) to location 0x0 is a grave error. 
        // 0x0b3-0x080 Dedicated registers and magical constants. It is a grave
        //             fault to change magical constants. Usable locations are 
        //             probably restricted to: 
        //             __rinst      Can be written directly. Incremented each 
        //                          instruction by code executing from SRAM.
        //             __mcycle     Can be written directly. Updated each instruction.
        //             __rNMI_IIV   Vector to the internal interrupt service
        //                          routine that is called when an interrupt
        //                          occurs. Please ensure the low 2 bits are
        //                          clear.
        //             __mtvec      Vector to interrupt/trap handling. Please ensure
        //                          the low 2 bits are clear. (DIRECT mode traps).
        // 0x0bf-0x0b4 Written by microcode when exceptions/interrupts occurs.
        //             __mepc       Used by exceptions/interrupts
        //             __mcause     Used by exceptions/interrupts
        //             __mtval      Used by exceptions/interrupts
        // 0x0df-0x0c0 This range is used by the minimal CSR implementation and also
        //             the internal interrupt routine.
        // 0x0ff-0x0e0 This range is used by the internal interrupt routine.
        // 0x103-0x100 Microcode writes the PC of the CSR instruction here
        // 0x107-0x104 URET/SRET/MRET emulation code entry point. Branched to by
        //             microcode when URET/SRET/MRET is seen
        // 0x10b-0x108 CSR emulation code entry point. Branched to by microcode
        // 
        // Hence, for a midgetv program with no CSR instructions, and with no
        // interrupts, it is conceivable to start the real program at address 0xb4.
        // I suspect most programs will include a minimal CSR, and a minimal 
        // interrupt routine. Preliminary testing indicate the real program will
        // start at around 0x300. And this leads to the obvious choice to let two
        // EBR rams hold system code (csr emulation, mret emulation, and internal
        // interrupt), and leave the SRAM for user code.
        // 
        // The following sizes of RAM exists for midgetv in ice40. Tested versions
        // are those with 8k and 65k RAM.
        // 
        //     \   No     64k    128k   
        //      \  Sram   SRAM   SRAM   
        // NrEBR +-------------------- 
        // 2     | 1k     65k    129k
        // 4     | 2k     66k    130k
        // 8     | 4k     68k    132k
        // 16    | 8k     72k    136k
        // 
        // 
        // A few utility macros used during simulation:
        // --------------------------------------------
        // I use the hint "sltu x0,rs1,rs2" as follows:
        // sltu x0, x((simarg)>>5) & 31, x((simarg) & 31).
        //
        // 0x2ff-0x000  simerr     End simulation with an error code
        // 0x31f-0x300  simputc()  Report macro
        // 0x3fe-0x320  free
        // 0x3ff        simdump    Dump EBR and SRAM
        //
        // #define simputc(reg) putchar(mem[reg*4] & 0xff)
        // 
        // =============================================================================
        .macro simerr simarg=0
        .word (((\simarg) & 0x3ff)<<15)+0x3033
        .endm

        .macro simend
        simerr 
        .endm

        .macro simdump
        simerr 0x3ff
        .endm

        .macro simputc simarg=1
        simerr \simarg+0x300
        .endm

        // =============================================================================
        // Interrupt/counter support. Most of the locations defined here require a
        // minimal internal ISR to work well.

        // The following locations are used to trigger side-effects when used as write address.
#define __write_mtime       0x200020e0 // Clears mtimeincip
#define __write_minstreth   0x200040fc // Clears minstrethincip
#define __write_mtimecmp    0x200800f8 // Clears mtip
#define __write_mtimecmph   0x200800fc // Also clears mtip

#define __write_bit_mtip    0x20008080 // Sets mtip. Should only be used in internalISR

//        // The following is mapped to EBR instead of IO for historical reasons.
//        // I see no need to change it
#define __write_mstatus     0x20010080  
#define __write_mie         0x20020080 
#define __write_mip         0x20040080
        

        
        // Reading these registers just as normal IO read.
        // The only difference is that the muxing is implemented in m_inputmux
        // instead of being externally visible
                                         // 31        18 17           16          15 12 11   10 8 7    6 4 3    2 0          
#define __read_mip          0x50000000   // 000000000000 mrinstretip  mtimeincip  0000  meip 000  mtip 000 msip 000  MIP     
#define __read_mie          0x60000000   // 000000000000 mrinstrettie mtimeincie  0000  meie 000  mtie 000 msie 000  MIE     
#define __read_mstatus      0x70000000   // 000000000000 0            0           0001  1    000  mpie 000 mie  000  MSTATUS 

        // minstreth lives in software, in EBR ram.
#define __read_minstreth    0x000000fc

        // =============================================================================
        // During simulation a whishbone register is at a defined address.
#define SLOWREGADR     0x40000004
#define NOSUCHREG_ADR  0x40000008