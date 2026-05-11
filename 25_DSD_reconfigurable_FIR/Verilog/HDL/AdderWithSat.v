/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : AdderWithSat.v
  - Description      : Saturated Adder Module
*******************************************************************/

/**
  * SRAM arbitration module
  * generate SRAM write, read, off signals
  * depend on p_state & top moudle sram sginal
  */

`timescale 1ns/10ps

module AdderWithSat (
  input                         iEn,
  input   signed        [15:0]  iA,
  input   signed        [15:0]  iB,
  output  wire signed   [15:0]  oSum
);
  wire [16:0] wSum;
  localparam MAX = $signed({1'b0, {(15){1'b1}}});
  localparam MIN = $signed({1'b1, {(15){1'b0}}});

  assign wSum = iEn ? iA + iB : {17{1'b0}};

 // saturation
  assign oSum = !iEn ? {16{1'b0}} :
                (iA[15] != iB[15]) ? wSum[15:0] :
                (iA[15] == 1'b0 && wSum[15] == 1'b1) ? MAX:
                (iA[15] == 1'b1 && wSum[15] == 1'b0) ? MIN:
                wSum[15:0];
endmodule
