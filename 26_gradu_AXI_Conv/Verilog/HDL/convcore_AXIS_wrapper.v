/*******************************************************************
  - Project          : 2026 graduation Project
  - File name        : convcore_AXIS_wrapper.v
  - Description      : AXIS wrapper for 3x3 Conv_core_RGB
  - Owner            : Dongjun.Kim
  - Revision history : 1) 2026.04.11 : Initial release
*******************************************************************/

`timescale 1ns/10ps

module Conv_AXIS_Wrapper #(
    parameter BUFFER_DEPTH = 240,
    parameter DATA_WIDTH = 32
)(
    // System Clock & Reset
    input  wire        aclk,
    input  wire        aresetn,

    // AXI4-Stream Slave (Input from DMA)
    input  wire [31:0] s_axis_tdata,
    input  wire        s_axis_tvalid,
    input  wire        s_axis_tlast,   // DMA에서 들어오지만 내부 코어에서는 쓰지 않음 (무시)
    output wire        s_axis_tready,

    // AXI4-Stream Master (Output to DMA)
    output wire [31:0] m_axis_tdata,
    output wire        m_axis_tvalid,
    output wire        m_axis_tlast,
    output wire [3:0]  m_axis_tkeep,   // DMA에 4바이트 모두 유효함을 알림
    input  wire        m_axis_tready,

    // Control & Configuration (CPU에서 AXI-Lite 레지스터를 통해 인가할 신호들)
    input  wire        iEn,
    input  wire [15:0] iMaxWidth,
    input  wire [15:0] iMaxHeight,
    output wire        oDone,

    output reg [15:0]  o_out_x_cnt,
    output reg [15:0]  o_out_y_cnt,
    
    input  wire [23:0] iCoeffR0, input wire [23:0] iCoeffR1, input wire [23:0] iCoeffR2,
    input  wire [23:0] iCoeffG0, input wire [23:0] iCoeffG1, input wire [23:0] iCoeffG2,
    input  wire [23:0] iCoeffB0, input wire [23:0] iCoeffB1, input wire [23:0] iCoeffB2
);

    // 내부 신호 선언
    
    // 32비트(4바이트) 모두 유효한 데이터임을 명시
    assign m_axis_tkeep = 4'b1111; 

    // -------------------------------------------------------------------------
    // 2. Convolution Core Instance
    // -------------------------------------------------------------------------
    
    wire wCoreVal, wBufferReady;
    wire [31:0] wCoreData, w_core_iData;

    // assign w_core_iData = {s_axis_tdata[7:0], s_axis_tdata[15:8], s_axis_tdata[23:16], s_axis_tdata[31:24]};
    assign w_core_iData = s_axis_tdata;

    wire wReady, wiVal;
    
    Conv_core_RGB #(
        .BUFFER_DEPTH(BUFFER_DEPTH),
        .DATA_WIDTH(DATA_WIDTH)
    ) u_Conv_core (
        .iClk       (aclk),
        .iRsn       (aresetn),
        
        // Handshake & Data Input
        .iEn        (iEn),
        .iVal       (s_axis_tvalid),
        .oReady     (s_axis_tready),
        .iData      (w_core_iData[23:0]),

        // Configurations
        .iMaxWidth  (iMaxWidth),
        .iMaxHeight (iMaxHeight),
        .iCoeffR0(iCoeffR0), .iCoeffR1(iCoeffR1), .iCoeffR2(iCoeffR2),
        .iCoeffG0(iCoeffG0), .iCoeffG1(iCoeffG1), .iCoeffG2(iCoeffG2),
        .iCoeffB0(iCoeffB0), .iCoeffB1(iCoeffB1), .iCoeffB2(iCoeffB2),

        // Handshake & Data Output
        .oVal       (wCoreVal),
        .iOutReady  (wBufferReady),
        // .oData      (wCoreData[31:8]),
        .oData      (wCoreData[23:0]),
        .oDone      (oDone) // Wrapper 밖으로 빼지 않고 무시해도 무방함
    );
    
    // assign wCoreData[7:0] = 8'h00;
    assign wCoreData[31:24] = 8'hFF;

    Queue_cir Output_Buffer(
        .iClk(aclk),
        .iRsn(aresetn),

        // Input (Push) - 가속기 코어에서 들어오는 부분
        .i_in_Val(wCoreVal),
        .i_in_Data(wCoreData),
        .o_in_Ready(wBufferReady),

        // Output (Pop) - DMA로 나가는 부분
        .o_out_Val(m_axis_tvalid),
        .o_out_Data(m_axis_tdata),
        .i_out_Ready(m_axis_tready)
    );

    // -------------------------------------------------------------------------
    // 3. TLAST Generation Logic (End of Frame)
    // -------------------------------------------------------------------------
    // DMA가 하나의 프레임을 다 받았음을 알 수 있도록, 이미지의 가장 마지막 픽셀에서 TLAST를 발생

    always @(posedge aclk) begin
        if (!aresetn) begin
            o_out_x_cnt <= 16'd0;
            o_out_y_cnt <= 16'd0;
        end 
        else if (m_axis_tvalid && m_axis_tready) begin
            // 픽셀이 출력될 때마다 카운트 증가
            if (o_out_x_cnt == iMaxWidth) begin
                o_out_x_cnt <= 16'd0;
                if (o_out_y_cnt == iMaxHeight) begin
                    o_out_y_cnt <= 16'd0; // 한 프레임 완료, 카운터 리셋
                end else begin
                    o_out_y_cnt <= o_out_y_cnt + 1'b1;
                end
            end else begin
                o_out_x_cnt <= o_out_x_cnt + 1'b1;
            end
        end
    end
    
    // 수정 제안
    wire is_last_pixel = (o_out_x_cnt == iMaxWidth) && (o_out_y_cnt == iMaxHeight);
    assign m_axis_tlast = is_last_pixel && m_axis_tvalid;

endmodule