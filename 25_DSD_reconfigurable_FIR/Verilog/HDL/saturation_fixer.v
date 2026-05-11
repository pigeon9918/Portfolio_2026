/*******************************************************************
  - Project          : 2025 Team Project_Reconf_FIR
  - File name        : saturation_fixer.v
  - Description      : saturation fixer module
*******************************************************************/

/**
  * saturation fixer module
  * fix overflow/underflow issue
  */

`timescale 1ns/10ps

module saturation_fixer #(
  parameter IN_M = 18,
  parameter OUT_N = 16
)(
  input signed [IN_M-1:0] i_num,
  output reg signed [OUT_N-1:0] o_num
);
  localparam signed [OUT_N-1:0] MAX_OUT = {1'b0, {(OUT_N-1){1'b1}} };
  localparam signed [OUT_N-1:0] MIN_OUT = {1'b1, {(OUT_N-1){1'b0}} };

  always @(*) begin
    if(i_num > MAX_OUT) begin
      o_num = MAX_OUT;
    end
    else if(i_num < MIN_OUT) begin
      o_num = MIN_OUT;
    end
    else begin
      o_num = i_num[OUT_N-1:0];
    end
  end

endmodule

/*
  assign wSum0 = rMul0 + rMul1;
  wSum0 = (rMul0[0] == rMul1[0] && wSum[0] != rMul0[0]) ? rMul0 + rMul1 : MAX_SIZE ;
*/