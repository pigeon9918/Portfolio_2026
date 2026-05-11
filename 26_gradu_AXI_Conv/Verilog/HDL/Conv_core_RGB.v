/*******************************************************************
  - Project          : 2026 graduation Project
  - File name        : Conv_core_RGB.v
  - Description      : RGBA 3x3 convolution with AXIS
  - Owner            : Dongjun.Kim
  - Revision history : 1) 2026.04.11 : Initial release
*******************************************************************/

`timescale 1ns/10ps

module Conv_core_RGB #(
  // Parameter
  parameter BUFFER_DEPTH = 240,
  parameter DATA_WIDTH = 32 ) (

  // Clock & reset
  input  wire                       iClk,     // Rising edge
  input  wire                       iRsn,     // Sync. & low reset

  // Input & Output
  input  wire                       iEn,    
  input  wire  [15:0]               iMaxWidth, 
  input  wire  [15:0]               iMaxHeight,
  
  input  wire  [23:0]               iCoeffR0,
  input  wire  [23:0]               iCoeffR1,
  input  wire  [23:0]               iCoeffR2,
  
  input  wire  [23:0]               iCoeffG0,
  input  wire  [23:0]               iCoeffG1,
  input  wire  [23:0]               iCoeffG2,
  
  input  wire  [23:0]               iCoeffB0,
  input  wire  [23:0]               iCoeffB1,
  input  wire  [23:0]               iCoeffB2,
  
  output wire                       oVal,
  output wire                       oDone,
  output wire                       oReady,
  
  input  wire                       iVal, 
  input  wire  [23:0]               iData,

  input  wire                       iOutReady,
  output wire  [23:0]               oData
  );
  
  reg rReadyInternal;
  
  wire wIncreaseWidth;
  reg [23:0] rWidthCounter;
  wire wIncreaseHeight;
  reg [23:0] rHeightCounter;
  wire wIncreasePointer;
  reg [1:0] rBufferPointer;
  reg [1:0] rBufferPointer_q1, rBufferPointer_q2, rBufferPointer_q3;

  reg [23:0] rLastLineCounter;

  wire [DATA_WIDTH-1:0] wFIFO_Out [2:0];
  wire [2:0] wWrn;
  reg [2:0] qWrn;
  wire wCsn;
  
  wire wFIFO_Valid [2:0];

  wire wReadDone;
  
  integer color, i, j;
  reg [7:0] rPixelWindow [0:2][0:2][0:2]; // [color][row][col]
  reg [7:0] rPaddedWindow [0:2][0:2][0:2];
  
  reg [7:0] rPixelWindowInput [0:2][0:2]; // [color][row]
  
  wire wShiftWindow;
  reg signed [7:0] rFilterWindow [0:2][0:2][0:2];
  wire wShiftFilter, wLoadFilter;

  wire wResetPtr;
  
  wire wFirstLinePadding, wLastLinePadding, wFirstPixelPadding, wLastPixelPadding;
  wire wEnableMac;

  reg signed [20:0] rAcc [0:2];
  reg signed [20:0] rSum[0:2][0:2];
  (* use_dsp = "yes" *) reg signed [16:0] rMacWindow [0:2][0:2][0:2];

  wire wDone;
  reg [10:1] rDone_q;
  
  reg [10:1] rFirstLinePadding_q, rLastLinePadding_q, rFirstPixelPadding_q, rLastPixelPadding_q;
  reg [10:1] rEnableMac_q, rShiftFilter_q, rLoadFilter_q;
  reg [23:0] rInData_q;
    
  reg [10:1] rMacVal_q;
  reg rDone;

  // 데이터가 유효하고, 동시에 다음 단(DMA)이 받을 준비가 되었을 때만 파이프라인 전진
  wire wAdvance;

  /*******************************************************************************/
  /*                            instances                                        */
  /*******************************************************************************/
  
  FIFO #(
    .FIFO_DEPTH(BUFFER_DEPTH),
    .DATA_WIDTH(DATA_WIDTH)
  ) input_Buffer_0 (
    // Clock & reset
    .iClk(iClk),              // Rising edge
    .iRsn(iRsn),              // Sync. & low reset

    // SP-SRAM Input & Output
    .iCsn(wCsn),               // Chip selected @ Low
    .iWrn(wWrn[0]),           // 0:Write, 1:Read

    .iRstPtr(wResetPtr),

    .iWrDt(iData),            // Write data
    .oRdDt(wFIFO_Out[0]),     // Read data
    .oVal(wFIFO_Valid[0])
  );

  FIFO #(
    .FIFO_DEPTH(BUFFER_DEPTH),
    .DATA_WIDTH(DATA_WIDTH)
  ) input_Buffer_1 (
    // Clock & reset
    .iClk(iClk),            // Rising edge
    .iRsn(iRsn),            // Sync. & low reset

    // SP-SRAM Input & Output
    .iCsn(wCsn),             // Chip selected @ Low
    .iWrn(wWrn[1]),         // 0:Write, 1:Read

    .iRstPtr(wResetPtr),

    .iWrDt(iData),          // Write data
    .oRdDt(wFIFO_Out[1]),   // Read data
    .oVal(wFIFO_Valid[1])
  );

  FIFO #(
    .FIFO_DEPTH(BUFFER_DEPTH),
    .DATA_WIDTH(DATA_WIDTH)
  ) input_Buffer_2 (
    // Clock & reset
    .iClk(iClk),            // Rising edge
    .iRsn(iRsn),            // Sync. & low reset

    // SP-SRAM Input & Output
    .iCsn(wCsn),             // Chip selected @ Low
    .iWrn(wWrn[2]),         // 0:Write, 1:Read

    .iRstPtr(wResetPtr),

    .iWrDt(iData),          // Write data
    .oRdDt(wFIFO_Out[2]),   // Read data
    .oVal(wFIFO_Valid[2])
  );

  /*******************************************************************************/
  /*                            Control planes                                   */
  /*******************************************************************************/
  
  // pipeline control signals
  assign wAdvance = iEn & (iVal | (rLastLineCounter != 0)) & iOutReady & ~rDone;
  
  assign wIncreaseWidth   = wAdvance;
  assign wShiftWindow     = wAdvance;
  assign wIncreasePointer = (rWidthCounter == iMaxWidth) & wAdvance;
  assign wResetPtr        = (rWidthCounter == iMaxWidth) & wAdvance;

  assign wIncreaseHeight  = (rWidthCounter == iMaxWidth) & iEn & wAdvance;
  assign wShiftFilter     = (rWidthCounter == iMaxWidth) & iEn & wAdvance;
  
  assign wMacVal          = (rHeightCounter > 0 & iEn ) || (rLastLineCounter != 0);
  assign wLoadFilter = (rHeightCounter == 1 & rWidthCounter == 0) && iEn;
  
  // padding
  assign wFirstLinePadding  = (rHeightCounter == 1) & wAdvance;
  assign wLastLinePadding   = (rHeightCounter == 0) & wAdvance;
  assign wFirstPixelPadding = (rWidthCounter == 0) & wAdvance;
  assign wLastPixelPadding  = (rWidthCounter == iMaxWidth) & wAdvance;
  
  // SRAM control
  assign wCsn     = ~(wAdvance);
  assign wWrn[0]  = ~(rBufferPointer == 0);
  assign wWrn[1]  = ~(rBufferPointer == 1);
  assign wWrn[2]  = ~(rBufferPointer == 2);

  // Done output
  assign wReadDone  = (rHeightCounter == iMaxHeight) && (rWidthCounter == iMaxWidth) && iEn;
  assign wDone      = (rLastLineCounter == iMaxWidth + 1);

  // output signals
  assign oVal   = rMacVal_q[6] & ~oDone & (iVal | (rLastLineCounter != 0));
  assign oDone  = rDone;
  assign oReady = rReadyInternal & iOutReady;
  
  always @(posedge iClk or negedge iRsn) begin : __READY_SIG_REG__
    if(!iRsn) begin
      rReadyInternal <= 1;
    end
    else if(wReadDone) begin
      rReadyInternal <= 0;
    end
    else if(oDone) begin
      rReadyInternal <= 1;
    end
  end

  always @(posedge iClk or negedge iRsn) begin : __DONE_SIG_REG__
    if(!iRsn) begin
      rDone <= 0;
    end
    else if(rDone_q[6]) begin
      rDone <= 1;
    end
  end

  always @(posedge iClk or negedge iRsn) begin : __INPUT_DELAY_REG__
    if(!iRsn) begin
      rInData_q <= 0;
    end
    else if(wAdvance) begin
      rInData_q <= iData;
    end
  end

  always @(posedge iClk or negedge iRsn) begin : __WIDTH_COUNTER__
    if(!iRsn) begin
      rWidthCounter <= 0;
    end
    else if(wIncreaseWidth) begin
      if(rWidthCounter == iMaxWidth) begin
        rWidthCounter <= 0;
      end
      else begin
        rWidthCounter <= rWidthCounter + 1;
      end
    end
  end

  always @(posedge iClk or negedge iRsn) begin : __HEIGHT_COUNTER__
    if(!iRsn) begin
      rHeightCounter <= 0;
    end
    else if(wIncreaseHeight) begin
      if(rHeightCounter == iMaxHeight) begin
        rHeightCounter <= 0;
      end
      else begin
        rHeightCounter <= rHeightCounter + 1;
      end
    end
  end

  always @(posedge iClk or negedge iRsn) begin : __BUFFER_POINTER__
    if(!iRsn) begin
      rBufferPointer <= 1;
    end
    else if(wIncreasePointer) begin
      if(rBufferPointer == 2) begin
        rBufferPointer <= 0;
      end
      else begin
        rBufferPointer <= rBufferPointer + 1;
      end
    end
  end
  
  always @(posedge iClk or negedge iRsn) begin : __DELAY_CHAIN__
    if (!iRsn) begin
      rFirstLinePadding_q   <= 0;
      rLastLinePadding_q    <= 0;
      rFirstPixelPadding_q  <= 0;
      rLastPixelPadding_q   <= 0;
      rEnableMac_q          <= 0;
      rShiftFilter_q        <= 0;
      rLoadFilter_q         <= 0;
      rMacVal_q             <= 0;
      rDone_q               <= 0;
      rBufferPointer_q1     <= 0;
      rBufferPointer_q2     <= 0;
      rBufferPointer_q3     <= 0;
      qWrn                  <= 0;
    end
    else if (wAdvance) begin
      rFirstLinePadding_q   <= { rFirstLinePadding_q[9:1], wFirstLinePadding };
      rLastLinePadding_q    <= { rLastLinePadding_q[9:1], wLastLinePadding };
      rFirstPixelPadding_q  <= { rFirstPixelPadding_q[9:1], wFirstPixelPadding };
      rLastPixelPadding_q   <= { rLastPixelPadding_q[9:1], wLastPixelPadding };
      rEnableMac_q          <= { rEnableMac_q[9:1], wEnableMac };
      rShiftFilter_q        <= { rShiftFilter_q[9:1], wShiftFilter };
      rLoadFilter_q         <= { rLoadFilter_q[9:1], wLoadFilter };
      rMacVal_q             <= { rMacVal_q[9:1], wMacVal };
      rDone_q               <= { rDone_q[9:1], wDone };
      rBufferPointer_q3     <= rBufferPointer_q2;
      rBufferPointer_q2     <= rBufferPointer_q1;
      rBufferPointer_q1     <= rBufferPointer;
      qWrn <= wWrn;
    end
  end

  /*******************************************************************************/
  /*                            Data plane                                       */
  /*******************************************************************************/

  always @(posedge iClk or negedge iRsn) begin : __PIXEL_WINDOW__
    if(!iRsn) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          for(j=0; j<3; j=j+1) begin
            rPixelWindow[color][i][j] <= 0;
          end
        end
      end
    end
    else if(wShiftWindow) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          for(j=0; j<2; j=j+1) begin
            rPixelWindow[color][i][j] <= rPixelWindow[color][i][j+1];
          end
          rPixelWindow[color][i][2] <= rPixelWindowInput[color][i];
        end
      end
    end
  end

  always @(*) begin : __WINDOW_LOADER__
    for(i=0; i<3; i=i+1) begin
      rPixelWindowInput[0][i] = wFIFO_Out[i][7:0];    // Red
      rPixelWindowInput[1][i] = wFIFO_Out[i][15:8];   // Green
      rPixelWindowInput[2][i] = wFIFO_Out[i][23:16];  // Blue
    end
    
    for(color=0; color<3; color=color+1) begin
      if (~qWrn[0]) rPixelWindowInput[color][0] = (color==0) ? rInData_q[7:0] : (color==1) ? rInData_q[15:8] : rInData_q[23:16];
      if (~qWrn[1]) rPixelWindowInput[color][1] = (color==0) ? rInData_q[7:0] : (color==1) ? rInData_q[15:8] : rInData_q[23:16];
      if (~qWrn[2]) rPixelWindowInput[color][2] = (color==0) ? rInData_q[7:0] : (color==1) ? rInData_q[15:8] : rInData_q[23:16];
    end
  end

  always @(*) begin : __PADDED_WINDOW__
    // default value
    for(color=0; color<3; color=color+1) begin
      for(i=0; i<3; i=i+1) begin
        for(j=0; j<3; j=j+1) begin
          rPaddedWindow[color][i][j] = rPixelWindow[color][i][j];
        end
      end
    end

    if (rFirstLinePadding_q[3]) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          rPaddedWindow[color][0][i] = 8'd0; 
        end
      end
    end
    
    if (rLastLinePadding_q[3]) begin
      for(color=0; color<3; color=color+1) begin
        case (rBufferPointer_q3)
          2'd0: for(i=0; i<3; i=i+1) begin
                  rPaddedWindow[color][0][i] = 8'd0;
                end
          2'd1: for(i=0; i<3; i=i+1) begin
                  rPaddedWindow[color][1][i] = 8'd0;
                end
          2'd2: for(i=0; i<3; i=i+1) begin
                  rPaddedWindow[color][2][i] = 8'd0;
                end
        endcase
      end
    end

    if (rFirstPixelPadding_q[3]) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          rPaddedWindow[color][i][0] = 8'd0; 
        end
      end
    end

    if (rLastPixelPadding_q[3]) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          rPaddedWindow[color][i][2] = 8'd0; 
        end
      end
    end
  end

  always @(posedge iClk or negedge iRsn) begin : __FILTER_WINDOW__
    if(!iRsn) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          for(j=0; j<3; j=j+1) begin
            rFilterWindow[color][i][j] <= 0;
          end
        end
      end
    end
    else if(wAdvance) begin
      if(rLoadFilter_q[2]) begin
        // Red Coeffs
        rFilterWindow[0][0][0] <= iCoeffR0[23:16]; rFilterWindow[0][0][1] <= iCoeffR0[15:8]; rFilterWindow[0][0][2] <= iCoeffR0[7:0];
        rFilterWindow[0][1][0] <= iCoeffR1[23:16]; rFilterWindow[0][1][1] <= iCoeffR1[15:8]; rFilterWindow[0][1][2] <= iCoeffR1[7:0];
        rFilterWindow[0][2][0] <= iCoeffR2[23:16]; rFilterWindow[0][2][1] <= iCoeffR2[15:8]; rFilterWindow[0][2][2] <= iCoeffR2[7:0];
        // Green Coeffs
        rFilterWindow[1][0][0] <= iCoeffG0[23:16]; rFilterWindow[1][0][1] <= iCoeffG0[15:8]; rFilterWindow[1][0][2] <= iCoeffG0[7:0];
        rFilterWindow[1][1][0] <= iCoeffG1[23:16]; rFilterWindow[1][1][1] <= iCoeffG1[15:8]; rFilterWindow[1][1][2] <= iCoeffG1[7:0];
        rFilterWindow[1][2][0] <= iCoeffG2[23:16]; rFilterWindow[1][2][1] <= iCoeffG2[15:8]; rFilterWindow[1][2][2] <= iCoeffG2[7:0];
        // Blue Coeffs
        rFilterWindow[2][0][0] <= iCoeffB0[23:16]; rFilterWindow[2][0][1] <= iCoeffB0[15:8]; rFilterWindow[2][0][2] <= iCoeffB0[7:0];
        rFilterWindow[2][1][0] <= iCoeffB1[23:16]; rFilterWindow[2][1][1] <= iCoeffB1[15:8]; rFilterWindow[2][1][2] <= iCoeffB1[7:0];
        rFilterWindow[2][2][0] <= iCoeffB2[23:16]; rFilterWindow[2][2][1] <= iCoeffB2[15:8]; rFilterWindow[2][2][2] <= iCoeffB2[7:0];
      end
      else if(rShiftFilter_q[3]) begin
        for(color=0; color<3; color=color+1) begin
          for(j=0; j<3; j=j+1) begin
            rFilterWindow[color][2][j] <= rFilterWindow[color][1][j];
            rFilterWindow[color][1][j] <= rFilterWindow[color][0][j];
            rFilterWindow[color][0][j] <= rFilterWindow[color][2][j];
          end
        end
      end
    end
  end

  always @(posedge iClk) begin : __MAC_PP_REG__
    if(!iRsn) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          for(j=0; j<3; j=j+1) begin
            rMacWindow[color][i][j] <= 0;
          end
        end
      end
    end
    else if(wAdvance) begin
      for(color=0; color<3; color=color+1) begin
        for(i=0; i<3; i=i+1) begin
          for(j=0; j<3; j=j+1) begin
            rMacWindow[color][i][j] <= $signed({1'b0, rPaddedWindow[color][i][j]}) * rFilterWindow[color][i][j];
          end
        end
      end
    end
  end

  always @(posedge iClk) begin : __SUM_PP_REG__
    if(!iRsn) begin
      for(color=0; color<3; color=color+1) 
        for(i=0; i<3; i= i+ 1)
          rSum[color][i] <= 0;
    end
    else if(wAdvance) begin
      for(color=0; color<3; color=color+1) begin
        rSum[color][0] <= rMacWindow[color][0][0] + rMacWindow[color][0][1] + rMacWindow[color][0][2];
        rSum[color][1] <= rMacWindow[color][1][0] + rMacWindow[color][1][1] + rMacWindow[color][1][2];
        rSum[color][2] <= rMacWindow[color][2][0] + rMacWindow[color][2][1] + rMacWindow[color][2][2];
      end
    end
  end

  always @(posedge iClk) begin : __ACCUMAULATE_PP_REG__
    if(!iRsn) begin
      for(color=0; color<3; color=color+1) rAcc[color] <= 0;
    end
    else if(wAdvance) begin
      for(color=0; color<3; color=color+1) begin
        rAcc[color] <= rSum[color][0] + rSum[color][1] + rSum[color][2];
      end
    end
  end

  assign oData[7:0]   = (rAcc[0] > 255) ? 8'hFF : (rAcc[0] < 0) ? 8'h00 : rAcc[0][7:0];   // r
  assign oData[15:8]  = (rAcc[1] > 255) ? 8'hFF : (rAcc[1] < 0) ? 8'h00 : rAcc[1][7:0];   // g
  assign oData[23:16] = (rAcc[2] > 255) ? 8'hFF : (rAcc[2] < 0) ? 8'h00 : rAcc[2][7:0];   // b

  always @(posedge iClk or negedge iRsn) begin : __LAST_LINE_COUNTER__
    if (!iRsn) begin
      rLastLineCounter <= 0;
    end
    else if ((~oReady & iOutReady) | wReadDone) begin
      if (rLastLineCounter == iMaxWidth + 1) begin
        rLastLineCounter <= 0;
      end
      else if (rLastLineCounter != iMaxWidth + 1) begin
        rLastLineCounter <= rLastLineCounter + 1;
      end
    end
  end  

endmodule
