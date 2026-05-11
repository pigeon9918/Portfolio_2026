
# # Create work library
# vlib work

# # Compile source files
# vlog Conv_core_RGB.v
# vlog FIFO.v
# vlog SpSram.v
# vlog myip_slave_lite_v1_0_S00_AXI.v
# vlog myip.v

# # Compile testbench
# vlog tb_myip.sv

# Start simulation
vsim -voptargs="+acc" tb_myip

# Add waves
add wave -noupdate -radix hexadecimal /tb_myip/clk
add wave -noupdate -radix hexadecimal /tb_myip/rst_n

add wave -noupdate -radix hexadecimal -divider {AXI-Lite}
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_awaddr
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_awvalid
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_awready
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_wdata
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_wvalid
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_wready
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_bvalid
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_bready
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_araddr
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_arvalid
add wave -noupdate -radix hexadecimal /tb_myip/s00_axi_arready

add wave -noupdate -radix hexadecimal /tb_myip/uut/myip_slave_lite_v1_0_S00_AXI_inst/slv_reg0
add wave -noupdate -radix hexadecimal /tb_myip/uut/myip_slave_lite_v1_0_S00_AXI_inst/i_cnt

add wave -noupdate -radix hexadecimal -divider {Input Stream}
add wave -noupdate -radix hexadecimal /tb_myip/s00_axis_tvalid
add wave -noupdate -radix hexadecimal /tb_myip/s00_axis_tready
add wave -noupdate -radix hexadecimal -radix hexadecimal /tb_myip/s00_axis_tdata
add wave -noupdate -radix hexadecimal /tb_myip/s00_axis_tlast

add wave -noupdate -radix hexadecimal -divider {Output buffer}
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/Output_Buffer/i_in_Data
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/Output_Buffer/o_out_Data
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/Output_Buffer/w_is_empty
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/Output_Buffer/w_is_full

add wave -noupdate -radix hexadecimal -divider {Output Stream}
add wave -noupdate -radix hexadecimal /tb_myip/m00_axis_tvalid
add wave -noupdate -radix hexadecimal /tb_myip/m00_axis_tready
add wave -noupdate -radix hexadecimal -radix hexadecimal /tb_myip/m00_axis_tdata
add wave -noupdate -radix hexadecimal /tb_myip/m00_axis_tlast

add wave -noupdate -radix hexadecimal -divider {Internal}
add wave -noupdate -radix unsigned /tb_myip/exp_x
add wave -noupdate -radix unsigned /tb_myip/exp_y

add wave -noupdate -divider {<R Windows>}
add wave -noupdate -label {Red Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[0][0]
add wave -noupdate -label {Red Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[0][1]
add wave -noupdate -label {Red Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[0][2]
add wave -noupdate -divider {<R Filters>}
add wave -noupdate -label {Red Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[0][0]
add wave -noupdate -label {Red Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[0][1]
add wave -noupdate -label {Red Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[0][2]

add wave -noupdate -divider {<G Windows>}
add wave -noupdate -label {Green Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[1][0]
add wave -noupdate -label {Green Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[1][1]
add wave -noupdate -label {Green Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[1][2]
add wave -noupdate -divider {<G Filters>}
add wave -noupdate -label {Green Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[1][0]
add wave -noupdate -label {Green Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[1][1]
add wave -noupdate -label {Green Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[1][2]

add wave -noupdate -divider {<B Windows>}
add wave -noupdate -label {Blue Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[2][0]
add wave -noupdate -label {Blue Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[2][1]
add wave -noupdate -label {Blue Window} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rPaddedWindow[2][2]
add wave -noupdate -divider {<B Filters>}
add wave -noupdate -label {Blue Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[2][0]
add wave -noupdate -label {Blue Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[2][1]
add wave -noupdate -label {Blue Filter} -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rFilterWindow[2][2]

add wave -noupdate -radix hexadecimal -divider {Internal State}
add wave -noupdate -radix unsigned /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/iMaxWidth
add wave -noupdate -radix unsigned /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/iMaxHeight
add wave -noupdate -radix unsigned /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rWidthCounter
add wave -noupdate -radix unsigned /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rHeightCounter
add wave -noupdate -radix unsigned /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/rLastLineCounter

add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/iVal
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/oVal
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/oReady
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/wReadDone
add wave -noupdate -radix hexadecimal /tb_myip/uut/u_Conv_AXIS_Wrapper/u_Conv_core/oDone

# Run simulation
run -all
