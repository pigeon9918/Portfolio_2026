/*******************************************************************
  - Project          : 2026 graduation Project
  - File name        : tb_myip.sv
  - Description      : tb for myip(3x3 convolution ip)
  - Owner            : Dongjun.Kim
  - Revision history : 1) 2026.04.11 : Initial release
*******************************************************************/

`timescale 1ns / 1ps

module tb_myip();

    // Parameters
    parameter integer C_S00_AXI_DATA_WIDTH = 32;
    parameter integer C_S00_AXI_ADDR_WIDTH = 6;
    parameter integer C_S00_AXIS_TDATA_WIDTH = 32;
    parameter integer C_M00_AXIS_TDATA_WIDTH = 32;

    // Clock and Reset
    reg clk;
    reg rst_n;

    // AXI-Lite Slave Interface
    reg [C_S00_AXI_ADDR_WIDTH-1:0] s00_axi_awaddr;
    reg [2:0] s00_axi_awprot;
    reg s00_axi_awvalid;
    wire s00_axi_awready;
    reg [C_S00_AXI_DATA_WIDTH-1:0] s00_axi_wdata;
    reg [(C_S00_AXI_DATA_WIDTH/8)-1:0] s00_axi_wstrb;
    reg s00_axi_wvalid;
    wire s00_axi_wready;
    wire [1:0] s00_axi_bresp;
    wire s00_axi_bvalid;
    reg s00_axi_bready;
    reg [C_S00_AXI_ADDR_WIDTH-1:0] s00_axi_araddr;
    reg [2:0] s00_axi_arprot;
    reg s00_axi_arvalid;
    wire s00_axi_arready;
    wire [C_S00_AXI_DATA_WIDTH-1:0] s00_axi_rdata;
    wire [1:0] s00_axi_rresp;
    wire s00_axi_rvalid;
    reg s00_axi_rready;

    // AXI-Stream Slave Interface (Input)
    wire s00_axis_tready;
    reg [C_S00_AXIS_TDATA_WIDTH-1:0] s00_axis_tdata;
    reg [(C_S00_AXIS_TDATA_WIDTH/8)-1:0] s00_axis_tstrb;
    reg s00_axis_tlast;
    reg s00_axis_tvalid;

    // AXI-Stream Master Interface (Output)
    wire m00_axis_tvalid;
    wire [C_M00_AXIS_TDATA_WIDTH-1:0] m00_axis_tdata;
    wire [(C_M00_AXIS_TDATA_WIDTH/8)-1:0] m00_axis_tstrb;
    wire m00_axis_tlast;
    reg m00_axis_tready;

    // Instantiate UUT
    myip # (
        .C_S00_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
        .C_S00_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH),
        .C_S00_AXIS_TDATA_WIDTH(C_S00_AXIS_TDATA_WIDTH),
        .C_M00_AXIS_TDATA_WIDTH(C_M00_AXIS_TDATA_WIDTH)
    ) uut (
        .s00_axi_aclk(clk),
        .s00_axi_aresetn(rst_n),
        .s00_axi_awaddr(s00_axi_awaddr),
        .s00_axi_awprot(s00_axi_awprot),
        .s00_axi_awvalid(s00_axi_awvalid),
        .s00_axi_awready(s00_axi_awready),
        .s00_axi_wdata(s00_axi_wdata),
        .s00_axi_wstrb(s00_axi_wstrb),
        .s00_axi_wvalid(s00_axi_wvalid),
        .s00_axi_wready(s00_axi_wready),
        .s00_axi_bresp(s00_axi_bresp),
        .s00_axi_bvalid(s00_axi_bvalid),
        .s00_axi_bready(s00_axi_bready),
        .s00_axi_araddr(s00_axi_araddr),
        .s00_axi_arprot(s00_axi_arprot),
        .s00_axi_arvalid(s00_axi_arvalid),
        .s00_axi_arready(s00_axi_arready),
        .s00_axi_rdata(s00_axi_rdata),
        .s00_axi_rresp(s00_axi_rresp),
        .s00_axi_rvalid(s00_axi_rvalid),
        .s00_axi_rready(s00_axi_rready),

        .s00_axis_aclk(clk),
        .s00_axis_aresetn(rst_n),
        .s00_axis_tready(s00_axis_tready),
        .s00_axis_tdata(s00_axis_tdata),
        .s00_axis_tstrb(s00_axis_tstrb),
        .s00_axis_tlast(s00_axis_tlast),
        .s00_axis_tvalid(s00_axis_tvalid),

        .m00_axis_aclk(clk),
        .m00_axis_aresetn(rst_n),
        .m00_axis_tvalid(m00_axis_tvalid),
        .m00_axis_tdata(m00_axis_tdata),
        .m00_axis_tstrb(m00_axis_tstrb),
        .m00_axis_tlast(m00_axis_tlast),
        .m00_axis_tready(m00_axis_tready)
    );

    // Clock Generation
    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    // AXI-Lite Write Task
    task axi_write(input [C_S00_AXI_ADDR_WIDTH-1:0] addr, input [C_S00_AXI_DATA_WIDTH-1:0] data);
        begin
            @(posedge clk);
            s00_axi_awaddr = addr;
            s00_axi_awvalid = 1;
            s00_axi_wdata = data;
            s00_axi_wstrb = 4'hf;
            s00_axi_wvalid = 1;
            s00_axi_bready = 1;
            
            wait(s00_axi_awready && s00_axi_wready);
            @(posedge clk);
            s00_axi_awvalid = 0;
            s00_axi_wvalid = 0;
            
            wait(s00_axi_bvalid);
            @(posedge clk);
            s00_axi_bready = 0;
            $display("AXI-Lite Write: Addr=0x%h, Data=0x%h", addr, data);
        end
    endtask

    // AXI-Lite Read Task
    task axi_read(input [C_S00_AXI_ADDR_WIDTH-1:0] addr, output [C_S00_AXI_DATA_WIDTH-1:0] data);
        begin
            @(posedge clk);
            s00_axi_araddr = addr;
            s00_axi_arvalid = 1;
            s00_axi_rready = 1;
            
            wait(s00_axi_arready);
            @(posedge clk);
            s00_axi_arvalid = 0;
            
            wait(s00_axi_rvalid);
            data = s00_axi_rdata;
            @(posedge clk);
            s00_axi_rready = 0;
            $display("AXI-Lite Read: Addr=0x%h, Data=0x%h", addr, data);
        end
    endtask

    // Test Image Parameters
    localparam WIDTH = 27;
    localparam HEIGHT = 34;
    reg [31:0] test_image [0:WIDTH*HEIGHT-1];
    reg [31:0] expected_image [0:WIDTH*HEIGHT-1]; // 정답 이미지 배열 추가
    integer x, y;

    // Convolution Variables
    integer kernel[0:2][0:2];
    integer ky, kx, py, px;
    integer sum_ch2, sum_ch1, sum_ch0;
    integer val_ch2, val_ch1, val_ch0;
    reg [7:0] p_ch2, p_ch1, p_ch0;

    // Simulation Control
    integer i, j;
    reg [C_S00_AXI_DATA_WIDTH-1:0] read_val;
    integer output_cnt = 0;
    integer error_cnt = 0;

    initial begin
        // Initialize Signals
        rst_n = 0;
        s00_axi_awaddr = 0; s00_axi_awprot = 0; s00_axi_awvalid = 0;
        s00_axi_wdata = 0; s00_axi_wstrb = 0; s00_axi_wvalid = 0;
        s00_axi_bready = 0;
        s00_axi_araddr = 0; s00_axi_arprot = 0; s00_axi_arvalid = 0;
        s00_axi_rready = 0;
        
        s00_axis_tdata = 0; s00_axis_tstrb = 4'hf; s00_axis_tlast = 0; s00_axis_tvalid = 0;
        m00_axis_tready = 1;

        // Generate Test Image Pattern
        for (y=0; y<HEIGHT; y=y+1) begin
            for (x=0; x<WIDTH; x=x+1) begin
                test_image[y*WIDTH + x] = {8'(y+1), 8'(x+1), 8'(y+x+1), 8'd0};
            end
        end

        // 3x3 Kernel Define (예시: Edge Detection 또는 Sharpen. 원하는 커널 값으로 변경하세요)
        kernel[0][0] =  2; kernel[0][1] = -1; kernel[0][2] =  2;
        kernel[1][0] = -1; kernel[1][1] =  5; kernel[1][2] = -1;
        kernel[2][0] =  2; kernel[2][1] = -1; kernel[2][2] =  2;

        // --- Generate Expected Image (3x3 Conv + Saturation) ---
        for (y=0; y<HEIGHT; y=y+1) begin
            for (x=0; x<WIDTH; x=x+1) begin
                sum_ch2 = 0;
                sum_ch1 = 0;
                sum_ch0 = 0;

                for (ky=-1; ky<=1; ky=ky+1) begin
                    for (kx=-1; kx<=1; kx=kx+1) begin
                        py = y + ky;
                        px = x + kx;

                        // Zero-Padding Boundary Check
                        if (py >= 0 && py < HEIGHT && px >= 0 && px < WIDTH) begin
                            // 각 픽셀 채널 추출 (32bit: {ch2, ch1, ch0, 8'd0})
                            p_ch2 = test_image[py*WIDTH + px][31:24];
                            p_ch1 = test_image[py*WIDTH + px][23:16];
                            p_ch0 = test_image[py*WIDTH + px][15:8];

                            // MAC 연산
                            sum_ch2 = sum_ch2 + (kernel[ky+1][kx+1] * p_ch2);
                            sum_ch1 = sum_ch1 + (kernel[ky+1][kx+1] * p_ch1);
                            sum_ch0 = sum_ch0 + (kernel[ky+1][kx+1] * p_ch0);
                        end
                    end
                end

                // Saturation: 0 미만은 0, 255 초과는 255
                val_ch2 = (sum_ch2 < 0) ? 0 : (sum_ch2 > 255) ? 255 : sum_ch2;
                val_ch1 = (sum_ch1 < 0) ? 0 : (sum_ch1 > 255) ? 255 : sum_ch1;
                val_ch0 = (sum_ch0 < 0) ? 0 : (sum_ch0 > 255) ? 255 : sum_ch0;

                // 정답 배열에 저장
                expected_image[y*WIDTH + x] = {val_ch2[7:0], val_ch1[7:0], val_ch0[7:0], 8'd0};
            end
        end
        // ---------------------------------------------------------

        // Reset
        #50;
        rst_n = 1;
        #50;

        $display("--- AXI-Lite Configuration Start ---");
        // IP 설정 구간 (만약 하드웨어 IP가 Convolution을 하도록 레지스터 세팅을 바꿔야 한다면 아래 값을 수정해야 합니다)
        axi_write(6'h0C, 24'h02FF02); // R Coeff 0
        axi_write(6'h10, 24'hFF05FF); // R Coeff 1 (blur)
        axi_write(6'h14, 24'h02FF02); // R Coeff 2
        
        axi_write(6'h18, 24'h02FF02); // G Coeff 0
        axi_write(6'h1C, 24'hFF05FF); // G Coeff 1 (blur)
        axi_write(6'h20, 24'h02FF02); // G Coeff 2
        
        axi_write(6'h24, 24'h02FF02); // B Coeff 0
        axi_write(6'h28, 24'hFF05FF); // B Coeff 1 (blur)
        axi_write(6'h2C, 24'h02FF02); // B Coeff 2

        // Set Image Size (Width-1, Height-1)
        axi_write(6'h04, {16'(HEIGHT-1), 16'(WIDTH-1)});

        // Enable IP and Pulse Core Reset
        axi_write(6'h00, 32'h00000011); // [0]=En, [4]=Core Reset
        
        $display("--- Image Data Transfer Start ---");
        // Transfer Image Data with Random Stalling
        for (y=0; y<HEIGHT; y=y+1) begin
            for (x=0; x<WIDTH; x=x+1) begin
                // Random stall
                while ($urandom_range(0, 4) == 0) begin
                    s00_axis_tvalid = 0;
                    @(posedge clk);
                end
                
                s00_axis_tdata = {test_image[y*WIDTH + x]};
                s00_axis_tvalid = 1;
                s00_axis_tlast = (y == HEIGHT-1 && x == WIDTH-1);
                
                @(posedge clk);
                while (!s00_axis_tready) @(posedge clk);
            end
        end
        s00_axis_tvalid = 0;
        s00_axis_tlast = 0;

        $display("--- Waiting for Completion ---");
        
        while (!m00_axis_tlast) @(posedge clk);

        // Wait for Done status in reg2
        read_val = 0;
        while (read_val[0] == 0) begin
            axi_read(6'h30, read_val);
            axi_read(6'h34, read_val);
            axi_read(6'h08, read_val);
            #100;
        end

        $display("--- Simulation Finished ---");
        $display("Total Pixels Received: %d", output_cnt);
        if (error_cnt == 0 && output_cnt == WIDTH*HEIGHT)
            $display("SUCCESS: All pixels matched Convolution output!");
        else
            $display("FAILURE: Errors detected or missing pixels. Errors: %d", error_cnt);
            
        #100;
        $finish;
    end

    // Monitor Output Stream and Verify Against expected_image
    integer exp_x = 0;
    integer exp_y = 0;
    reg [31:0] exp_data;
    
    always @(posedge clk) begin
        if (m00_axis_tvalid && m00_axis_tready) begin
            exp_data = expected_image[exp_y*WIDTH + exp_x]; // expected_image와 비교하도록 수정
            
            if (m00_axis_tdata !== exp_data) begin
                $display("XX ERROR at Pixel (%d, %d): Exp=0x%h, Got=0x%h", exp_x, exp_y, exp_data, m00_axis_tdata);
                error_cnt = error_cnt + 1;
            end
            else 
                $display("OO Correct at Pixel (%d, %d): Got=0x%h", exp_x, exp_y, exp_data);
            
            output_cnt = output_cnt + 1;
            
            // Increment expected coordinates
            if (exp_x == WIDTH-1) begin
                exp_x = 0;
                exp_y = exp_y + 1;
            end else begin
                exp_x = exp_x + 1;
            end
        end
    end

    // Random Output Backpressure
    initial begin
        force m00_axis_tready = 0;
        $display("Force applied: tready = %b", m00_axis_tready);
        
        repeat(500) @(posedge clk);
        release m00_axis_tready;
        $display("Force released");
        
        forever @(posedge clk) begin
            if ($urandom_range(0, 9) == 0)
                m00_axis_tready <= 0;
            else
                m00_axis_tready <= 1;
        end
    end
        
endmodule