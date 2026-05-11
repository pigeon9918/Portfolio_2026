/*******************************************************************
  - Project          : 2026 graduation Project
  - File name        : FIFO.v
  - Description      : FIFO with SPSRAM
  - Owner            : Dongjun.Kim
  - Revision history : 1) 2026.04.11 : Initial release
*******************************************************************/

`timescale 1ns/10ps

module FIFO #(

  // Parameter
  parameter FIFO_DEPTH = 16,
  parameter DATA_WIDTH = 32 ) (

  // Clock & reset
  input  wire                       iClk,     // Rising edge
  input  wire                       iRsn,     // Sync. & low reset


  // SP-SRAM Input & Output
  input  wire                       iCsn,     // Chip selected @ Low
  input  wire                       iWrn,     // 0:Write, 1:Read

  
  input                             iRstPtr,

  input  wire [DATA_WIDTH-1:0]      iWrDt,    // Write data
  output wire [DATA_WIDTH-1:0]      oRdDt,    // Read data
  output reg                        oVal
  );

  function integer log_b2(input integer iDepth);
  begin

    log_b2 = 0;

    while (iDepth)
    begin
      log_b2 = log_b2  + 1;
      iDepth = iDepth >> 1;
    end

  end
  endfunction

  reg   [log_b2(FIFO_DEPTH - 1) - 1: 0]   rWrPtr;
  reg   [log_b2(FIFO_DEPTH - 1) - 1 : 0]  rRdPtr;
  wire  [log_b2(FIFO_DEPTH - 1) - 1 : 0]  wSramAddress;

  assign wSramAddress = iWrn ? rRdPtr : rWrPtr;

  wire wValidIn;
  assign wValidIn = ~iCsn & ~iWrn;

  wire wValidOut;
  assign wValidOut = ~iCsn & iWrn;
  
  always @(posedge iClk or negedge iRsn) begin
    if(!iRsn)
      rWrPtr <= 0;
    else if(iRstPtr)
      rWrPtr <= 0;
    else if(wValidIn)
      if(rWrPtr == FIFO_DEPTH-1)
        rWrPtr <= 0;
      else
        rWrPtr <= rWrPtr + 1;
  end

  always @(posedge iClk or negedge iRsn) begin
    if(!iRsn)
      rRdPtr <= 0;
    else if(iRstPtr)
      rRdPtr <= 0;
    else if(wValidOut)
      if(rRdPtr == FIFO_DEPTH-1)
        rRdPtr <= 0;
      else
        rRdPtr <= rRdPtr + 1;
  end

  always @(posedge iClk or negedge iRsn) begin
    if(!iRsn)
      oVal <= 0;
    else
      oVal <= wValidOut;
  end
  
  SpSram #(
    .SRAM_DEPTH(FIFO_DEPTH), .DATA_WIDTH(DATA_WIDTH)
    ) internal_sram (
      // Clock & reset
      .iClk(iClk),     // Rising edge
      .iRsn(iRsn),     // Sync. & low reset
      // SP-SRAM Input & Output
      .iCsn(iCsn),     // Chip selected @ Low
      .iWrn(iWrn),     // 0:Write, 1:Read
      .iAddr(wSramAddress),    // 32bit data address

      .iWrDt(iWrDt),    // Write data
      .oRdDt(oRdDt)    // Read data
  );

endmodule