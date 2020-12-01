//Copyright 1986-2017 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2017.4 (lin64) Build 2086221 Fri Dec 15 20:54:30 MST 2017
//Date        : Fri Nov 20 14:38:15 2020
//Host        : ELO-arch running 64-bit Manjaro Linux
//Command     : generate_target design_1_wrapper.bd
//Design      : design_1_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module design_1_wrapper
   (aclk_0,
    aresetn_0,
    irq_0);
  input aclk_0;
  input aresetn_0;
  output irq_0;

  wire aclk_0;
  wire aresetn_0;
  wire irq_0;

  design_1 design_1_i
       (.aclk_0(aclk_0),
        .aresetn_0(aresetn_0),
        .irq_0(irq_0));
endmodule
