/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : FIR_FSM_Controller.v
  - Description      : FSM of Fir filter
*******************************************************************/

/**
  * main FSM of fir filter module
  * control p_state and cnt signal
  */

`timescale 1ns/10ps

module FIR_FSM_Controller (
  input wire iClk_12M,            // System Clock (12MHz)
  input wire iRsn,                // Active Low Reset
  
  // --- Control Interface ---
  input wire iEnSample600k,       // 600kHz Sampling Pulse
  input wire iCoeffUpdateFlag,    // High: Update Mode, Low: Run Mode
  
  output reg [1:0] oPstate,
  output reg [3:0] oCnt,
  output wire oSumEn
);

  // =================================================================
  // 1. Parameters & State Definition
  // =================================================================
  localparam p_Idle   = 2'b00; 
  localparam p_Write  = 2'b01; 
  localparam p_Mac    = 2'b10; 
  localparam p_Sum    = 2'b11; 

  reg [1:0] rNxtState;
  
  // =================================================================
  // 2. State Register (Sequential)
  // =================================================================
  always @(posedge iClk_12M or negedge iRsn) begin
      if (!iRsn) 
        oPstate <= p_Idle;
      else       
        oPstate <= rNxtState;
  end

  // =================================================================
  // 3. Next State Logic (Combinational)
  // =================================================================
  always @(*) begin
    case (oPstate)
      p_Idle: begin
        if (iCoeffUpdateFlag)
          rNxtState = p_Write; // Priority 1: Update
        else if (iEnSample600k)
          rNxtState = p_Mac;    // Priority 2: Sample
        else
          rNxtState = p_Idle;   
      end

      p_Write: begin // Update Mode
        if (!iCoeffUpdateFlag)
          rNxtState = p_Idle;
        else
          rNxtState = p_Write;
      end

      p_Mac: begin // Calculation Loop (Run Mode)
        if (oCnt == 4'd11)
          rNxtState = p_Sum;
        else
          rNxtState = p_Mac;
      end
      
      p_Sum: begin // Done
        rNxtState = p_Idle;
      end
      
      default:
        rNxtState = p_Idle;
    endcase
  end

  // =================================================================
  // 4. Counter Logic (Sequential)
  // =================================================================
  always @(posedge iClk_12M or negedge iRsn) begin
    if (!iRsn) begin
        oCnt <= 4'd0;
    end
    else begin
        // Reset counter when transitioning from Idle to Mac state
        if (oPstate == p_Idle && iEnSample600k) begin
            oCnt <= 4'd0;
        end
        // Increment counter while in Mac state
        else if (oPstate == p_Mac) begin
            if (oCnt != 4'd11) // Count from 0 to 9
              oCnt <= oCnt + 1;
        end
    end
  end

  // =================================================================
  // 5. Output Logic
  // =================================================================
  assign oSumEn = (oPstate == p_Sum) ? 1'b1 :
                  (oPstate == p_Idle) ? 1'b1 : 
                  1'b0;

endmodule