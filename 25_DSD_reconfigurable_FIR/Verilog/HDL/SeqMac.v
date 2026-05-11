/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : SeqMAc.v
  - Description      : sequence multiply accumulate unit
*******************************************************************/

/**
  * multiply signal and coefficient, then accumulate the result
  */

`timescale 1ns/10ps
module SeqMac #(
  // -- Module-specific constants --
  parameter DATA_WIDTH  = 16,
  parameter SIG_WIDTH  = 3
)(
  input                         iClk,
  input                         iRsn,
  input                         iEnSample,
  input                         iEnMul,
  input                         iEnAdd,
  input                         iEnAcc,
  // input                 [3:0]   iCnt,
  input         signed  [DATA_WIDTH-1:0]  iCoeff,
  input         signed  [SIG_WIDTH-1:0]   iDelay,
  output  wire  signed  [DATA_WIDTH-1:0]  oAccOut
);
  localparam IDLE = 4'b1111;
  localparam ACC_WIDTH = DATA_WIDTH + SIG_WIDTH + 2; 

  wire signed [ACC_WIDTH - 1: 0]  wMulOut;
  wire signed [ACC_WIDTH - 1: 0]  wAddOut;

  reg   signed  [ACC_WIDTH + 3:0]  rAccOut;

  // conditional multiplier
  assign wMulOut = (iEnMul == 1'b1) ? (iCoeff * iDelay) : $signed({(ACC_WIDTH - 1){1'b0}});
  
  // Adder
  assign wAddOut = (iEnAdd == 1'b1) ? (wMulOut + rAccOut) : $signed({(ACC_WIDTH - 1){1'b0}});

  // Accumulate
  always @(posedge iClk) begin
    if (!iRsn) begin
      rAccOut <= 0;
    end
    else begin
      if (iEnSample == 1'b1)
        rAccOut <= 0;
      else if (iEnAcc == 1'b1)
        rAccOut <= wAddOut;
        // case(iCnt) 
        //   IDLE: rAccOut <= rAccOut;
        //   default: rAccOut <= wAddOut;
        // endcase
      else rAccOut <= rAccOut;
    end
  end

  // saturation check
  saturation_fixer #(
    .IN_M(ACC_WIDTH + 4), 
    .OUT_N(DATA_WIDTH) 
  ) fixer(
    .i_num(rAccOut),
    .o_num(oAccOut)
  );

endmodule