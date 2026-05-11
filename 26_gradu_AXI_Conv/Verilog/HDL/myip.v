/*******************************************************************
  - Project          : 2026 graduation Project
  - File name        : myip.v
  - Description      : AXI wrapper for 3x3 convolution ip
  - Owner            : Dongjun.Kim
  - Revision history : 1) 2026.04.11 : Initial release
*******************************************************************/

`timescale 1 ns / 1 ps

  module myip #
  (
    // Users to add parameters here

    // User parameters ends
    // Do not modify the parameters beyond this line


    // Parameters of Axi Slave Bus Interface S00_AXI
    parameter integer C_S00_AXI_DATA_WIDTH  = 32,
    parameter integer C_S00_AXI_ADDR_WIDTH  = 6,

    // Parameters of Axi Master Bus Interface M00_AXIS
    parameter integer C_M00_AXIS_TDATA_WIDTH  = 32,
    parameter integer C_M00_AXIS_START_COUNT  = 32,

    // Parameters of Axi Slave Bus Interface S00_AXIS
    parameter integer C_S00_AXIS_TDATA_WIDTH  = 32
  )
  (
    // Users to add ports here

    // User ports ends
    // Do not modify the ports beyond this line


    // Ports of Axi Slave Bus Interface S00_AXI
    input wire  s00_axi_aclk,
    input wire  s00_axi_aresetn,
    input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
    input wire [2 : 0] s00_axi_awprot,
    input wire  s00_axi_awvalid,
    output wire  s00_axi_awready,
    input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
    input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
    input wire  s00_axi_wvalid,
    output wire  s00_axi_wready,
    output wire [1 : 0] s00_axi_bresp,
    output wire  s00_axi_bvalid,
    input wire  s00_axi_bready,
    input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
    input wire [2 : 0] s00_axi_arprot,
    input wire  s00_axi_arvalid,
    output wire  s00_axi_arready,
    output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
    output wire [1 : 0] s00_axi_rresp,
    output wire  s00_axi_rvalid,
    input wire  s00_axi_rready,

    // Ports of Axi Master Bus Interface M00_AXIS
    input wire  m00_axis_aclk,
    input wire  m00_axis_aresetn,
    output wire  m00_axis_tvalid,
    output wire [C_M00_AXIS_TDATA_WIDTH-1 : 0] m00_axis_tdata,
    output wire [(C_M00_AXIS_TDATA_WIDTH/8)-1 : 0] m00_axis_tstrb,
    output wire  m00_axis_tlast,
    input wire  m00_axis_tready,

    // Ports of Axi Slave Bus Interface S00_AXIS
    input wire  s00_axis_aclk,
    input wire  s00_axis_aresetn,
    output wire  s00_axis_tready,
    input wire [C_S00_AXIS_TDATA_WIDTH-1 : 0] s00_axis_tdata,
    input wire [(C_S00_AXIS_TDATA_WIDTH/8)-1 : 0] s00_axis_tstrb,
    input wire  s00_axis_tlast,
    input wire  s00_axis_tvalid
  );

// Instantiation of Axi Bus Interface S00_AXI
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg0;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg1;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg2;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg3;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg4;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg5;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg6;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg7;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg8;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg9;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg10;
  wire [C_S00_AXI_DATA_WIDTH-1:0] w_slv_reg11;

  wire w_core_done;

  wire [15:0] w_out_x_cnt;
  wire [15:0] w_out_y_cnt;

  myip_slave_lite_v1_0_S00_AXI # ( 
    .C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
    .C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
  ) myip_slave_lite_v1_0_S00_AXI_inst (
    .o_slv_reg0(w_slv_reg0),
    .o_slv_reg1(w_slv_reg1),
    .o_slv_reg2(w_slv_reg2),
    .o_slv_reg3(w_slv_reg3),
    .o_slv_reg4(w_slv_reg4),
    .o_slv_reg5(w_slv_reg5),
    .o_slv_reg6(w_slv_reg6),
    .o_slv_reg7(w_slv_reg7),
    .o_slv_reg8(w_slv_reg8),
    .o_slv_reg9(w_slv_reg9),
    .o_slv_reg10(w_slv_reg10),
    .o_slv_reg11(w_slv_reg11),
    .i_done(w_core_done),
    .i_cnt({w_out_x_cnt, w_out_y_cnt}),
    .i_m00_axis_tready(m00_axis_tready),

    .S_AXI_ACLK(s00_axi_aclk),
    .S_AXI_ARESETN(s00_axi_aresetn),
    .S_AXI_AWADDR(s00_axi_awaddr),
    .S_AXI_AWPROT(s00_axi_awprot),
    .S_AXI_AWVALID(s00_axi_awvalid),
    .S_AXI_AWREADY(s00_axi_awready),
    .S_AXI_WDATA(s00_axi_wdata),
    .S_AXI_WSTRB(s00_axi_wstrb),
    .S_AXI_WVALID(s00_axi_wvalid),
    .S_AXI_WREADY(s00_axi_wready),
    .S_AXI_BRESP(s00_axi_bresp),
    .S_AXI_BVALID(s00_axi_bvalid),
    .S_AXI_BREADY(s00_axi_bready),
    .S_AXI_ARADDR(s00_axi_araddr),
    .S_AXI_ARPROT(s00_axi_arprot),
    .S_AXI_ARVALID(s00_axi_arvalid),
    .S_AXI_ARREADY(s00_axi_arready),
    .S_AXI_RDATA(s00_axi_rdata),
    .S_AXI_RRESP(s00_axi_rresp),
    .S_AXI_RVALID(s00_axi_rvalid),
    .S_AXI_RREADY(s00_axi_rready)
  );

  // Add user logic here

  // m00_axis_tkeep 신호는 tstrb로 대체하여 사용 (모든 바이트 유효)
  assign m00_axis_tstrb = 4'b1111;

  // 프레임 처리가 끝났음을 AXI-Lite Status 레지스터로 전달 (선택 사항)
  // assign w_core_done = m00_axis_tlast; 

  // Conv_AXIS_Wrapper 인스턴스화
  Conv_AXIS_Wrapper #(
    .BUFFER_DEPTH(2048),
    .DATA_WIDTH(32)
    ) u_Conv_AXIS_Wrapper (
    // System Clock & Reset (스트림 기준으로 통일)
    .aclk          (s00_axis_aclk),
    .aresetn       (s00_axis_aresetn & (~w_slv_reg0[4])),

    // AXI4-Stream Slave (Input from DMA)
    .s_axis_tdata  (s00_axis_tdata),
    .s_axis_tvalid (s00_axis_tvalid),
    .s_axis_tlast  (s00_axis_tlast),
    .s_axis_tready (s00_axis_tready),

    // AXI4-Stream Master (Output to DMA)
    .m_axis_tdata  (m00_axis_tdata),
    .m_axis_tvalid (m00_axis_tvalid),
    .m_axis_tlast  (m00_axis_tlast),
    .m_axis_tkeep  (), // 외부에서 tstrb로 고정했으므로 여기선 비워둠
    .m_axis_tready (m00_axis_tready),

    // Control & Configuration (AXI-Lite Slave Registers 매핑)
    .iEn           (w_slv_reg0[0]),         // Reg0 Bit[0]: IP Enable
    .iMaxWidth     (w_slv_reg1[15:0]),      // Reg1 하위 16비트: Width
    .iMaxHeight    (w_slv_reg1[31:16]),     // Reg1 상위 16비트: Height
    .oDone         (w_core_done),

    .o_out_x_cnt(w_out_x_cnt),
    .o_out_y_cnt(w_out_y_cnt),

    .iCoeffR0      (w_slv_reg3[23:0]),      // Reg3 ~ Reg11: Filter Coefficients
    .iCoeffR1      (w_slv_reg4[23:0]),
    .iCoeffR2      (w_slv_reg5[23:0]),

    .iCoeffG0      (w_slv_reg6[23:0]),
    .iCoeffG1      (w_slv_reg7[23:0]),
    .iCoeffG2      (w_slv_reg8[23:0]),

    .iCoeffB0      (w_slv_reg9[23:0]),
    .iCoeffB1      (w_slv_reg10[23:0]),
    .iCoeffB2      (w_slv_reg11[23:0])
  );

  // User logic ends

  endmodule
