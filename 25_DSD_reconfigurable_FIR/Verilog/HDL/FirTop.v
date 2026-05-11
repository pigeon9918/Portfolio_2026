/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : FirTop.v
  - Description      : FIR top module
*******************************************************************/

/**
  * FIR top module
  * instantiate all sub modules
  */

`timescale 1ns/10ps

module FirTop(
  input wire iClk_12M,
  input wire iRsn,

  input wire iEnSample600k,
  input wire iCoeffUpdateFlag,
  input wire iCsnRam,
  input wire iWrnRam,
  input wire [5:0] iAddrRam,
  input wire [15:0] iWrDtRam,
  input wire [5:0] iNumOfCoeff,

  input wire signed [2:0] iFirln,

  output reg signed [15:0] oFirOut
);

  /* ================================================================= */
  /* inner wiring */
  wire [3:0] wCnt;
  wire [1:0] wPstate;

  wire [0:3] wCsnRam;
  wire [0:3] wWrnRam;

  wire [3:0] wAddrRam;

  wire [3:0] wEnMul;
  wire [3:0] wEnAdd;
  wire [3:0] wEnAcc;

  wire  signed [15:0] wCoeff [0:3];
  wire  signed [15:0] wMacOut [0:3];

  wire  signed [3:0] rMacInput [0:3]; // Changed from reg to wire
  reg   signed [3:0] rMacInput_temp [0:3]; // Temporary reg for combinatorial logic

  wire wSumEn;
  wire  signed [15:0] wMacSum01;
  wire  signed [15:0] wMacSum23;
  wire  signed [15:0] wFinalSum; 
  /* ================================================================= */
  /* inner regs */
  // middle is 31
    reg signed [2:0] rDelayCain [0:62];
    reg signed [2:0] rDelayedFirln;
  
    /* ================================================================= */
    /* inner logic glue */
  
    /* ================================================================= */
    /* Tap wiring */
    wire signed [16:0] wTap [31:0];
    genvar k;
    generate
      for (k=1; k<32; k = k + 1) begin: GEN_PAIRS
        assign wTap[k] = rDelayCain[31 + k] + rDelayCain[31 - k];
      end
    endgenerate
    // middle is 0
    // first & end is 31
    assign wTap[0] = rDelayCain[31];
  
    /* ================================================================= */
    /* Delay chain */
    integer i;
    always @(posedge iClk_12M) begin
      if (!iRsn) begin
        // synchronous reset: clear all registers
        for (i = 0; i < 63; i = i + 1) begin
          rDelayCain[i] <= 3'b000;
          rDelayedFirln <= 3'b000;
        end
      end
      // shifting
      else begin 
        if (iEnSample600k) begin // given code
          rDelayedFirln <= iFirln;
          rDelayCain[0] <= rDelayedFirln;
          for (i = 0; i < 62; i = i + 1) begin
            rDelayCain[i+1] <= rDelayCain[i];
          end
        end
      end
    end

  /* ================================================================= */
  /* output FF */
  always @(posedge iClk_12M) begin
    if (!iRsn) begin
      oFirOut <= {16{1'b0}};
    end
    // shifting
    else begin 
      if (iEnSample600k) begin // given code
        oFirOut <= wFinalSum;
      end
    end
  end

  /* ================================================================= */
  /* tap mux for mac */
  always @(*) begin
    // Default assignments for rMacInput_temp to avoid latches
    rMacInput_temp[0] = 4'd0;
    rMacInput_temp[1] = 4'd0;
    rMacInput_temp[2] = 4'd0;
    rMacInput_temp[3] = 4'd0;

    case(wCnt)
      1: begin
        rMacInput_temp[0] = wTap[0];
        rMacInput_temp[1] = wTap[10];
        rMacInput_temp[2] = wTap[20];
        rMacInput_temp[3] = wTap[30];
      end
      2: begin
        rMacInput_temp[0] = wTap[1];
        rMacInput_temp[1] = wTap[11];
        rMacInput_temp[2] = wTap[21];
        rMacInput_temp[3] = wTap[31];
      end
      3: begin
        rMacInput_temp[0] = wTap[2];
        rMacInput_temp[1] = wTap[12];
        rMacInput_temp[2] = wTap[22];
        // wTap[32] is out of bounds for wTap[31:0], assign 0
        rMacInput_temp[3] = 4'd0; 
      end
      4: begin
        rMacInput_temp[0] = wTap[3];
        rMacInput_temp[1] = wTap[13];
        rMacInput_temp[2] = wTap[23];
        // wTap[33] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      5: begin
        rMacInput_temp[0] = wTap[4];
        rMacInput_temp[1] = wTap[14];
        rMacInput_temp[2] = wTap[24];
        // wTap[34] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      6: begin
        rMacInput_temp[0] = wTap[5];
        rMacInput_temp[1] = wTap[15];
        rMacInput_temp[2] = wTap[25];
        // wTap[35] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      7: begin
        rMacInput_temp[0] = wTap[6];
        rMacInput_temp[1] = wTap[16];
        rMacInput_temp[2] = wTap[26];
        // wTap[36] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      8: begin
        rMacInput_temp[0] = wTap[7];
        rMacInput_temp[1] = wTap[17];
        rMacInput_temp[2] = wTap[27];
        // wTap[37] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      9: begin
        rMacInput_temp[0] = wTap[8];
        rMacInput_temp[1] = wTap[18];
        rMacInput_temp[2] = wTap[28];
        // wTap[38] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      10: begin
        rMacInput_temp[0] = wTap[9];
        rMacInput_temp[1] = wTap[19];
        rMacInput_temp[2] = wTap[29];
        // wTap[39] is out of bounds, assign 0
        rMacInput_temp[3] = 4'd0;
      end
      default: begin
        // rMacInput_temp already defaults to 4'd0
      end
    endcase
  end

  // Assign the temporary reg array to the wire rMacInput
  assign rMacInput[0] = rMacInput_temp[0];
  assign rMacInput[1] = rMacInput_temp[1];
  assign rMacInput[2] = rMacInput_temp[2];
  assign rMacInput[3] = rMacInput_temp[3];

  /* ================================================================= */
  /* tap mux for mac */
  FIR_FSM_Controller fsm(
    .iClk_12M(iClk_12M),
    .iRsn(iRsn),

    .iEnSample600k(iEnSample600k),
    .iCoeffUpdateFlag(iCoeffUpdateFlag),

    .oPstate(wPstate),
    .oCnt(wCnt),
    .oSumEn(wSumEn)
  );

  /* ================================================================= */
  /* Controllers */
  SeqMacController secCtrl(
    .iCnt(wCnt),
    .iPstate(wPstate),
    .iNumOfCoeff(iNumOfCoeff),
    .oEnMul(wEnMul),
    .oEnAdd(wEnAdd),
    .oEnAcc(wEnAcc)
  );

  SRAM_Controller_4Parallel MemCtrl(
    .iPstate(wPstate),
    .iAddrRam(iAddrRam),
    .oAddrRam(wAddrRam),
    .iCsnRam(iCsnRam),
    .iCnt(wCnt),
    .iWrnRam(iWrnRam),
    .oCsnRam(wCsnRam),
    .oWrnRam(wWrnRam)
  );

  /* ================================================================= */
  /* macs */
  SeqMac #(
    .SIG_WIDTH(4),
    .DATA_WIDTH(16)
  ) mac0(
    .iClk(iClk_12M),
    .iEnSample(iEnSample600k),
    .iRsn(iRsn),
    .iEnMul(wEnMul[0]),
    .iEnAdd(wEnAdd[0]),
    .iEnAcc(wEnAcc[0]),
    // .iCnt(wCnt),
    .iCoeff(wCoeff[0]),
    .iDelay(rMacInput[0]),
    .oAccOut(wMacOut[0])
  );

  SeqMac #(
    .SIG_WIDTH(4),
    .DATA_WIDTH(16)
  ) mac1(
    .iClk(iClk_12M),
    .iEnSample(iEnSample600k),
    .iRsn(iRsn),
    .iEnMul(wEnMul[1]),
    .iEnAdd(wEnAdd[1]),
    .iEnAcc(wEnAcc[1]),
    // .iCnt(wCnt),
    .iCoeff(wCoeff[1]),
    .iDelay(rMacInput[1]),
    .oAccOut(wMacOut[1])
  );

  SeqMac #(
    .SIG_WIDTH(4),
    .DATA_WIDTH(16)
  ) mac2(
    .iClk(iClk_12M),
    .iEnSample(iEnSample600k),
    .iRsn(iRsn),
    .iEnMul(wEnMul[2]),
    .iEnAdd(wEnAdd[2]),
    .iEnAcc(wEnAcc[2]),
    // .iCnt(wCnt),
    .iCoeff(wCoeff[2]),
    .iDelay(rMacInput[2]),
    .oAccOut(wMacOut[2])
  );

  SeqMac #(
    .SIG_WIDTH(4),
    .DATA_WIDTH(16)
  ) mac3(
    .iClk(iClk_12M),
    .iEnSample(iEnSample600k),
    .iRsn(iRsn),
    .iEnMul(wEnMul[3]),
    .iEnAdd(wEnAdd[3]),
    .iEnAcc(wEnAcc[3]),
    // .iCnt(wCnt),
    .iCoeff(wCoeff[3]),
    .iDelay(rMacInput[3]),
    .oAccOut(wMacOut[3])
  );

  /* ================================================================= */
  /* Rams */
  SpSram #(
    .DATA_WIDTH(16),
    .SRAM_DEPTH(10)
  ) SPRAM0 (
    .iClk(iClk_12M),
    .iRsn(iRsn),
    .iCsn(wCsnRam[0]),
    .iWrn(wWrnRam[0]),
    .iAddr(wAddrRam),
    .iWrDt(iWrDtRam),
    .oRdDt(wCoeff[0])
  );

  SpSram #(
    .DATA_WIDTH(16),
    .SRAM_DEPTH(10)
  ) SPRAM1 (
    .iClk(iClk_12M),
    .iRsn(iRsn),
    .iCsn(wCsnRam[1]),
    .iWrn(wWrnRam[1]),
    .iAddr(wAddrRam),
    .iWrDt(iWrDtRam),
    .oRdDt(wCoeff[1])
  );

  SpSram #(
    .DATA_WIDTH(16),
    .SRAM_DEPTH(10)
  ) SPRAM2 (
    .iClk(iClk_12M),
    .iRsn(iRsn),
    .iCsn(wCsnRam[2]),
    .iWrn(wWrnRam[2]),
    .iAddr(wAddrRam),
    .iWrDt(iWrDtRam),
    .oRdDt(wCoeff[2])
  );

  SpSram #(
    .DATA_WIDTH(16),
    .SRAM_DEPTH(2)
  ) SPRAM3 (
    .iClk(iClk_12M),
    .iRsn(iRsn),
    .iCsn(wCsnRam[3]),
    .iWrn(wWrnRam[3]),
    .iAddr(wAddrRam[0]),
    .iWrDt(iWrDtRam),
    .oRdDt(wCoeff[3])
  );
  
  /* ================================================================= */
  /* Adders for output */
  AdderWithSat Add01 (
    .iEn(wSumEn),
    .iA(wMacOut[0]),
    .iB(wMacOut[1]),
    .oSum(wMacSum01)
  );
  AdderWithSat Add23 (
    .iEn(wSumEn),
    .iA(wMacOut[2]),
    .iB(wMacOut[3]),
    .oSum(wMacSum23)
  );
  AdderWithSat AddTotal (
    .iEn(wSumEn),
    .iA(wMacSum01),
    .iB(wMacSum23),
    .oSum(wFinalSum)
  );

endmodule

// SignalName[Array_Index][Bit\_Index]