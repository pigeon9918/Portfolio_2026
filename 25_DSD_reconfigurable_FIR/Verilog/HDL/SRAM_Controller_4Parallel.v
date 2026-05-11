/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : SRAM_Controller_4Parallel.v
  - Description      : 4-Parallel Memory Controller Module
*******************************************************************/

/**
  * SRAM arbitration module
  * generate SRAM write, read, off signals
  * depend on p_state & top moudle sram sginal
  */

`timescale 1ns/10ps

module SRAM_Controller_4Parallel (
    // System
    input wire [1:0] iPstate,   // Run 모드용 내부 주소 (0~9)

    // External Interface (from Testbench/MCU)
    input wire [5:0] iAddrRam,      // External Address (0~63)
    output reg [3:0] oAddrRam,
    input wire iCsnRam,             // External Chip Select
    input wire iWrnRam,             // External Write Enable
    input wire [3:0] iCnt,

    // Output to RAM
    output reg [0:3] oCsnRam,
    output reg [0:3] oWrnRam

);
    wire [1:0] wBankSel;
    assign wBankSel = iAddrRam[5:4];
    // =================================================================
    // 1. Parameters (Standard SRAM Control)
    // =================================================================
    // 주의: 실제 SRAM 동작에 맞춰 값을 설정했습니다.
    // CS=0, WR=1 -> READ
    // CS=0, WR=0 -> WRITE
    localparam MEM_READ  = 2'b01; 
    localparam MEM_WRITE = 2'b00;
    localparam MEM_NO_OP = 2'b11; // CS=1 (Deselect)

    // =================================================================
    // 2. Bank Selection (4-Parallel Interleaving)
    // =================================================================
    // 하위 2비트를 사용하여 4개의 뱅크를 선택합니다.

    localparam p_Idle = 2'b00;
    localparam p_Write  = 2'b01;
    localparam p_Mac= 2'b10;
    localparam p_Sum = 2'b11;

    // =================================================================
    // 3. Control Signal Generation (User Logic Applied)
    // =================================================================    
    always @(*) begin
         // RAM 1
        if(iPstate == p_Mac && {iCsnRam, iWrnRam} == MEM_READ && iCnt < 10) begin
            {oCsnRam[0], oWrnRam[0]} = MEM_READ;
            {oCsnRam[1], oWrnRam[1]} = MEM_READ;
            {oCsnRam[2], oWrnRam[2]} = MEM_READ;
            {oCsnRam[3], oWrnRam[3]} = MEM_READ;
            oAddrRam = iCnt;
        end
        else
        if(iPstate == p_Write && {iCsnRam, iWrnRam} == MEM_WRITE && iCnt < 10) begin
            
            {oCsnRam[0], oWrnRam[0]} = MEM_NO_OP; 
            {oCsnRam[1], oWrnRam[1]} = MEM_NO_OP;
            {oCsnRam[2], oWrnRam[2]} = MEM_NO_OP;
            {oCsnRam[3], oWrnRam[3]} = MEM_NO_OP;
            case(wBankSel)
                2'b00: {oCsnRam[0], oWrnRam[0]} = MEM_WRITE;
                2'b01: {oCsnRam[1], oWrnRam[1]} = MEM_WRITE;
                2'b10: {oCsnRam[2], oWrnRam[2]} = MEM_WRITE;
                2'b11: {oCsnRam[3], oWrnRam[3]} = MEM_WRITE;
            endcase
            oAddrRam = iAddrRam[3:0];
        end
        else begin
            {oCsnRam[0], oWrnRam[0]} = MEM_NO_OP;
            {oCsnRam[1], oWrnRam[1]} = MEM_NO_OP;
            {oCsnRam[2], oWrnRam[2]} = MEM_NO_OP;
            {oCsnRam[3], oWrnRam[3]} = MEM_NO_OP;
            oAddrRam = iAddrRam[3:0];
        end
    end


endmodule