/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : ReConf_FirFilter_tb.v
  - Description      : Testbench for Reconfigurable FIR Filter
*******************************************************************/

/**
  * testbench for Reconfigurable FIR Filter
  * instantiate FirTop module
  * apply test scenarios
  */

`timescale 1ns/10ps

module ReConf_FirFilter_tb;

  /***********************************************
  // Wire & Register Declarations
  ***********************************************/
  // System signals
  reg iClk_12M;
  reg iRsn;

  localparam NUMCOEFF = 6'd33;

  localparam MEM_READ  = 2'b01; 
  localparam MEM_WRITE = 2'b00;
  localparam MEM_NO_OP = 2'b11; // CS=1 (Deselect)
  
  // DUT Inputs
  reg iEnSample600k;
  reg iCoeffUpdateFlag;
  reg iCsnRam;
  reg iWrnRam;
  reg [5:0] iAddrRam;
  reg signed [15:0] iWrDtRam;
  reg [5:0] iNumOfCoeff;
  reg signed [2:0] iFirIn;

  // DUT Output
  wire signed [15:0] oFirOut;

  // Instantiate the DUT (Device Under Test)
  FirTop uut (
    .iClk_12M(iClk_12M),
    .iRsn(iRsn),
    .iEnSample600k(iEnSample600k),
    .iCoeffUpdateFlag(iCoeffUpdateFlag),
    .iCsnRam(iCsnRam),
    .iWrnRam(iWrnRam),
    .iAddrRam(iAddrRam),
    .iWrDtRam(iWrDtRam),
    .iNumOfCoeff(iNumOfCoeff),
    .iFirln(iFirIn), // Note: DUT has a port named iFirln
    .oFirOut(oFirOut)
  );

  /***********************************************
  // Clock Generation (12MHz)
  ***********************************************/
  localparam FREQ = 12.0;     // 12MHz
  localparam CLK_PERIOD = (1000.0 / FREQ);

  initial begin
    iClk_12M = 1'b0;
    forever #(CLK_PERIOD/2) iClk_12M = ~iClk_12M;
  end

  /***********************************************
  // Main Test Scenario
  ***********************************************/
  initial begin
    $display("=======================================");
    $display("==== Testbench Simulation Starting ====");
    $display("=======================================");

    // 1. Initialize all inputs
    initialize_signal();
    
    // 2. Apply system reset
    reset_DUT();

    // 3. Load coefficients into SRAM
    load_coefficients();
    
    // 4. Run FIR filter with an impulse input
    // run_fir_test(3'd1, 6'd63);

    // run_fir_test(3'd1, 6'd21);
    // run_fir_test(-3'd1, 6'd21);
    
    // run_fir_test(3'd3, 6'd21);
    // run_fir_test(-3'd3, 6'd21);

    // run_fir_test(3'd1, 6'd21);
    // run_fir_test(-3'd1, 6'd21);

    // 5. saturation test
    fir_saturation_test();

    $display("======================================");
    $display("==== Test Scenario Complete      ====");
    $display("======================================");
    #1000;
    $stop;
  end


  /***********************************************
  // Task: Initialize all testbench drivers
  ***********************************************/
  task initialize_signal;
  begin
    iEnSample600k   = 1'b0;
    iCoeffUpdateFlag  = 1'b0;
    // Set SRAM for wait
    {iCsnRam, iWrnRam} = MEM_NO_OP;
    iAddrRam        = 6'd0;
    iWrDtRam        = 16'd0;
    iNumOfCoeff     = NUMCOEFF;
    iFirIn          = 3'd0;
  end
  endtask

  /***********************************************
  // Task: Apply active-low reset
  ***********************************************/
  task reset_DUT;
  begin
    $display("[%0t] Applying active-low reset...", $time);
    iRsn = 1'b1;
    #(2 * CLK_PERIOD);
    iRsn = 1'b0; // Assert reset
    #(5 * CLK_PERIOD);
    iRsn = 1'b1; // De-assert reset
    $display("[%0t] Reset released.", $time);
    #(2 * CLK_PERIOD);
  end
  endtask

  /***********************************************
  // Task: Load Coefficients into SRAM
  ***********************************************/
  task load_coefficients;
    reg signed [15:0] coeff [0:32];
    reg [3:0] i;
  begin
    // Define coefficients
    coeff[0]  = 16'd500;
    coeff[1]  = 16'd206;
    coeff[2]  = -16'd102;
    coeff[3]  = 16'd0;
    coeff[4]  = 16'd48;
    coeff[5]  = -16'd37;
    coeff[6]  = 16'd0;
    coeff[7]  = 16'd24;
    coeff[8]  = -16'd19;
    coeff[9]  = 16'd0;

    coeff[10]  = 16'd13;
    coeff[11]  = -16'd11;
    coeff[12]  = 16'd0;
    coeff[13]  = 16'd7;
    coeff[14]  = -16'd6;
    coeff[15]  = 16'd0;
    coeff[16]  = 16'd3;
    coeff[17]  = 16'd0;
    coeff[18]  = 16'h7000;
    coeff[19]  = 16'h7000;

    coeff[20]  = 16'h7000;
    coeff[21]  = 16'h7000;
    coeff[22]  = 16'h7000;
    coeff[23]  = 16'h7000;
    coeff[24]  = 16'h7000;
    coeff[25]  = 16'h7000;
    coeff[26]  = 16'h7000;
    coeff[27]  = 16'h7000;
    coeff[28]  = 16'h7000;
    coeff[29]  = 16'h7000;

    coeff[30]  = 16'h7000;
    coeff[31]  = 16'h7000;

    $display("[%0t] Starting coefficient update...", $time);
    
    // Set flag to enter p_Write state in FSM
    iCoeffUpdateFlag = 1'b1;
    @(posedge iClk_12M);
    @(posedge iClk_12M);

    // Activate SRAM for writing
    {iCsnRam, iWrnRam} = MEM_WRITE;

    // Write 10 coefficients to Bank 0
    for (i = 0; i < 10; i = i + 1) begin
      iAddrRam <= {2'b00, i}; // Bank 0, Address i
      iWrDtRam <= coeff[i];
      $display("[%0t] Writing coeff[%0d]=%d to Addr=%b", $time, i, coeff[i], iAddrRam);
      @(posedge iClk_12M);
    end
    for (i = 0; i < 10; i = i + 1) begin
      iAddrRam <= {2'b01, i}; // Bank 0, Address i
      iWrDtRam <= coeff[10 + i];
      $display("[%0t] Writing coeff[%0d]=%d to Addr=%b", $time, i, coeff[10 + i], iAddrRam);
      @(posedge iClk_12M);
    end
    for (i = 0; i < 10; i = i + 1) begin
      iAddrRam <= {2'b10, i}; // Bank 0, Address i
      iWrDtRam <= coeff[20 + i];
      $display("[%0t] Writing coeff[%0d]=%d to Addr=%b", $time, i, coeff[20 + i], iAddrRam);
      @(posedge iClk_12M);
    end
    for (i = 0; i < 2; i = i + 1) begin
      iAddrRam <= {2'b11, i}; // Bank 0, Address i
      iWrDtRam <= coeff[30 + i];
      $display("[%0t] Writing coeff[%0d]=%d to Addr=%b", $time, i, coeff[30 + i], iAddrRam);
      @(posedge iClk_12M);
    end

    @(posedge iClk_12M);
    // Deactivate SRAM and return to Idle state
    iCoeffUpdateFlag = 1'b0;
    {iCsnRam, iWrnRam} = MEM_READ;
    iAddrRam = 6'b00_0000;
    iWrDtRam = 16'd0;

    $display("[%0t] Coefficient update finished.", $time);
    #(5 * CLK_PERIOD); // Wait for FSM to settle in Idle
  end
  endtask

  /***********************************************
  // Task: Generate 600kHz sample enable and run test
  ***********************************************/
  task run_fir_test(input signed [2:0] inValue, input [5:0] numOfCoeff);
  begin
    $display("[%0t] Starting FIR run test with scaled impulse input.", $time);
    
    // Wait for a few sample periods first
    repeat(3) begin
      makeEnSample();
    end
    
    // scaled impulse
    $display("[%0t] >>> Applying iFirIn=%d", $time, inValue);
    {iCsnRam, iWrnRam} = MEM_READ;
    iNumOfCoeff = numOfCoeff;
    iFirIn = inValue;
    makeEnSample();
    // release after one sample cycle
    iFirIn = 3'd0;
    $display("[%0t] >>> scaled impulse ended, iFirIn=0", $time);

    // Let it run for many sample periods to see the output ring out
    repeat (66) begin
      makeEnSample();
    end
  end
  endtask

  /***********************************************
  // Task: Generate 600kHz sample enable and run test
  ***********************************************/
  task fir_saturation_test();
  begin
    $display("[%0t] Starting FIR run test with scaled impulse input.", $time);
    
    // Wait for a few sample periods first
    repeat(3) begin
      makeEnSample();
    end

    {iCsnRam, iWrnRam} = MEM_READ;
    iNumOfCoeff = 63;
    // make impulse
    $display("[%0t] >>> Applying iFirIn=3", $time);
    iFirIn = 3'd3;
    makeEnSample();
    // release after one sample cycle
    $display("[%0t] >>> scaled impulse ended, iFirIn=0", $time);
    iFirIn = 3'd0;

    // wait for center tap
    repeat(30) makeEnSample();
    
    // make impulse
    $display("[%0t] >>> Applying iFirIn=3", $time);
    iFirIn = 3'd3;
    makeEnSample();
    // release after one sample cycle
    $display("[%0t] >>> scaled impulse ended, iFirIn=0", $time);
    iFirIn = 3'd0;

    // Let it run for many sample periods to see the output ring out
    repeat (66) begin
      makeEnSample();
    end
  end
  endtask

  // generate EnSample signal
  task makeEnSample();
    localparam SAMPLE_PERIOD_CYCLES = 20; // 12MHz / 600kHz = 20
    integer cycle_count;
    begin
      // timing sync on clock
      @(posedge iClk_12M);
      // count 19 clock cycle
      for(cycle_count = 0; cycle_count < SAMPLE_PERIOD_CYCLES -1; cycle_count = cycle_count + 1) begin
          @(posedge iClk_12M);
      end
      // make En signal
      iEnSample600k = 1'b1;
      @(posedge iClk_12M);
      iEnSample600k = 1'b0;
    end
  endtask

endmodule