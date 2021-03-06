/* -----------------------------------------------------------------------------
 * Part of midgetv
 * 2019. Copyright B. Nossum.
 * For licence, see LICENCE
 * -----------------------------------------------------------------------------
 * ucode holds the microcode of midgetv in three EBRs, or two EBRs and some logic.
 * This module is a wrapper to assign control lines to a distinct data out of EBR.
 * 
 */
module m_ucode
  # ( parameter NO_UCODEOPT = 0 )
   (
    input        clk,
    input [7:0]  minx,
    input        progress_ucode, // Hold when shifting
    output       sa00, // sa02,sa03,sa04,sa05,sa06,
    output [1:0] s_alu_carryin,
    output [2:0] s_alu,
    output [1:0] s_shift,
    output [1:0] s_cyclecnt,
    output       sa11,sa12, sa14,sa15,
    output       sa20,sa21,sa22,sa23,sa24,sa25,sa26,sa27,sa28,sa29,
    output       sa30, sa32,sa33,sa34, sa37,sa38,sa39,
    output       sa40,sa41,sa42,sa43,
    output       clrM,
    output       ceM,
    output       potentialMODbranch,
    output [7:0] rinx,
    output       ucode_killwarnings
    );
`ifdef verilator
   function [47:0] get_sa;
      // verilator public
      get_sa = d;
   endfunction
`endif
   
   /*AUTOWIRE*/
   // Beginning of automatic wires (for undeclared instantiated-module outputs)
   wire [47:0]          d;                      // From inst_3ebr of m_3ebr.v, ...
   // End of automatics
   generate
      if ( NO_UCODEOPT == 1 ) begin
         m_3ebr inst_3ebr
           (/*AUTOINST*/
            // Outputs
            .d                          (d[47:0]),
            // Inputs
            .clk                        (clk),
            .minx                       (minx[7:0]),
            .progress_ucode             (progress_ucode));
      end else begin
         m_2ebr inst_2ebr
           (/*AUTOINST*/
            // Outputs
            .d                          (d[47:0]),
            // Inputs
            .clk                        (clk),
            .minx                       (minx[7:0]),
            .progress_ucode             (progress_ucode));
      end      
   endgenerate
   
   assign sa00 = d[ 0];
   assign s_alu_carryin = d[2:1]; //        Carry in select
   assign s_alu         = d[5:3]; //        Alu operation
   assign s_cyclecnt    = d[11:10]; //      rcount mix, +3/+4 mux
   assign s_shift       = {d[12],d[13]}; // Shift counter selection.
   
   
   assign sa11 = d[22]; // Immediate expand exactly when we use dinx.
   assign sa12 = d[ 7]; // u_io_i_latch
   assign sa14 = d[ 8]; // nReset Q
   assign sa15 = d[ 9]; // Part of enable to Q
   assign sa20 = d[14];
   assign sa21 = d[15];
   assign sa22 = d[16];
   assign sa23 = d[17];
   assign sa24 = d[18]; // | Governs write address mux
   assign sa25 = d[19]; // | 
   assign sa26 = d[20]; // |
   assign sa27 = d[21]; // |
   assign sa28 = d[22]; // use_dinx
   assign sa29 = d[23]; // Adr0Mustbe0
   assign sa30 = d[24]; // Adr1Mustbe0     
   assign sa32 = d[25]; // Part of enable to Q
   assign sa33 = d[26]; // Control freeze of control when shifting
   assign sa34 = d[27]; // Modify input mux.
   assign sa37 = d[6]; //  use_brcond
   assign sa38 = d[28]; // | isr_none/isr_use_uj/isr_intoCSR/isr_intoTrap
   assign sa39 = d[29]; // |
   assign sa40 = d[30]; // Select signal for rai
   assign sa41 = d[31]; // Latch SEL signals etc
   assign sa42 = d[32]; // Possibly activate STB_O or sram_stb
   assign sa43 = d[33]; // Possibly activate WE_O next cycle
   assign clrM = d[34]; // R for M register. Also used to flag unsigned subtraction in DIV/DIVU/REM/REMU
   assign ceM  = d[35]; // CE for M register. Also used by m_condcode
   assign potentialMODbranch = d[36]; // Distinguish DIV or MOD
   assign rinx = d[44:37];
   assign ucode_killwarnings = d[39]; // ???
endmodule

// Local Variables:
// verilog-library-directories:("." "../obj_dir" )
// verilog-library-extensions:(".v" )
// End:
