/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : SeqMAcController.v
  - Description      : sequence MAC controller
*******************************************************************/

/**
  * control 4 mac units by iNumOfCoeff
  * compare cnt and iNumOfCoeff values
*/

`timescale 1ns/10ps

module SeqMacController (
  input           [3:0]   iCnt,
  input           [1:0]   iPstate,
  input           [5:0]   iNumOfCoeff,
  output  wire    [3:0]   oEnMul,
  output  wire    [3:0]   oEnAdd,
  output  wire    [3:0]   oEnAcc
);
  localparam ON  = 3'b111;
  localparam OFF = 3'b000;
  localparam p_Idle   = 2'b00; 
  localparam p_Write  = 2'b01; 
  localparam p_Mac    = 2'b10; 
  localparam p_Sum    = 2'b11; 

  wire [3:0] wNumForMac [3:0];
  wire [5:0] wNumOfHalf;

  assign wNumOfHalf = iNumOfCoeff[5:1] + iNumOfCoeff[0];

  assign wNumForMac[3] =  (wNumOfHalf > 40) ? (10) : 
                          (wNumOfHalf > 30) ? wNumOfHalf - 30 : 
                          0;
  assign wNumForMac[2] =  (wNumOfHalf > 30) ? (10) : 
                          (wNumOfHalf > 20) ? wNumOfHalf - 20 : 
                          0;
  assign wNumForMac[1] =  (wNumOfHalf > 20) ? (10) : 
                          (wNumOfHalf > 10) ? wNumOfHalf - 10 : 
                          0;
  assign wNumForMac[0] =  (wNumOfHalf > 10) ? (10) : 
                          wNumOfHalf;
  
  assign {oEnMul[3], oEnAdd[3], oEnAcc[3]} = (iPstate == p_Mac && iCnt > 0 && iCnt <= wNumForMac[3]) ? ON : OFF;
  assign {oEnMul[2], oEnAdd[2], oEnAcc[2]} = (iPstate == p_Mac && iCnt > 0 && iCnt <= wNumForMac[2]) ? ON : OFF;
  assign {oEnMul[1], oEnAdd[1], oEnAcc[1]} = (iPstate == p_Mac && iCnt > 0 && iCnt <= wNumForMac[1]) ? ON : OFF;
  assign {oEnMul[0], oEnAdd[0], oEnAcc[0]} = (iPstate == p_Mac && iCnt > 0 && iCnt <= wNumForMac[0]) ? ON : OFF;
endmodule

  // localparam NUM_OF_MACS  = 4,
  // localparam CHUNK_SIZE = 10
  // integer i;
  // integer offset;
  // always @(*) begin
  //   for (i = 0; i < NUM_MACS; i = i + 1) begin
  //       offset = i * CHUNK_SIZE; // 0, 10, 20, 30 ...

  //       if (iNumOfCoeff > (offset + CHUNK_SIZE)) begin
  //           // 예: i=0일 때, 입력값이 10보다 크면 -> 10
  //           wNumForMac[i] = CHUNK_SIZE;
  //       end
  //       else if (iNumOfCoeff > offset) begin
  //           // 예: i=0일 때, 입력값이 5면 (5 > 0) -> 5 - 0 = 5
  //           wNumForMac[i] = iNumOfCoeff - offset;
  //       end
  //       else begin
  //           // 예: i=1일 때, 입력값이 5면 (5 > 10) 거짓 -> 0
  //           wNumForMac[i] = 0;
  //       end
  //   end
  // end