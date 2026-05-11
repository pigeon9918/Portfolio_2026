/*******************************************************************
  - Project          : 2026 graduation Project
  - File name        : Queue_cir.v
  - Description      : FIFO with Register
  - Owner            : Dongjun.Kim
  - Revision history : 1) 2026.04.11 : Initial release
*******************************************************************/

`timescale 1ns/10ps

module Queue_cir #(
  // Parameter
  parameter DATA_WIDTH = 32
) (
  input  wire iClk,
  input  wire iRsn,

  // Input (Push) - 가속기 코어에서 들어오는 부분
  input  wire i_in_Val,
  input  wire [DATA_WIDTH - 1:0] i_in_Data, // [수정] 데이터 폭 지정
  output wire o_in_Ready,

  // Output (Pop) - DMA로 나가는 부분
  output wire o_out_Val,
  output wire [DATA_WIDTH - 1:0] o_out_Data, // [수정] wire로 변경
  input  wire i_out_Ready
);

  reg [4:0] rd_ptr, wr_ptr;
  reg [DATA_WIDTH - 1:0] mem [0:15];

  wire w_is_empty, w_is_full;
  wire is_read_setup, is_write_setup;

  assign w_is_empty = (rd_ptr == wr_ptr);
  assign w_is_full  = (wr_ptr[4] != rd_ptr[4]) && (wr_ptr[3:0] == rd_ptr[3:0]);

  assign o_in_Ready = ~w_is_full;
  assign o_out_Val  = ~w_is_empty;

  assign is_read_setup  = i_out_Ready & ~w_is_empty;
  assign is_write_setup = i_in_Val & ~w_is_full;

  // [핵심 발전] FWFT (First-Word Fall-Through) 적용
  // 항상 읽기 포인터가 가리키는 값을 미리 출력 포트에 올려놓습니다.
  // DMA는 o_out_Val이 1일 때 언제든지 지연 없이 이 데이터를 가져갈 수 있습니다.
  assign o_out_Data = mem[rd_ptr[3:0]];

  integer i;

  // --- WRITE PLANE (Push) ---
  always @(posedge iClk or negedge iRsn) begin
    if(!iRsn) begin
      wr_ptr <= 0;
      for(i=0; i<16; i = i + 1)
        mem[i] <= 0;
    end
    else if(is_write_setup) begin
      mem[wr_ptr[3:0]] <= i_in_Data;
      wr_ptr <= wr_ptr + 1;
    end
  end

  // --- READ PLANE (Pop) ---
  always @(posedge iClk or negedge iRsn) begin
    if(!iRsn) begin
      rd_ptr <= 0;
    end
    else if(is_read_setup) begin
      // 데이터는 이미 assign으로 빠져나가고 있으므로, 여기선 포인터만 이동시킵니다.
      rd_ptr <= rd_ptr + 1; 
    end
  end

endmodule