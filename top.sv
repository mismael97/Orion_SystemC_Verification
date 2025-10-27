module top(
  input  logic clk,
  input  logic rst_n,
  input  logic en,
  input  logic [2:0] op,
  input  logic [8-1:0] a,
  input  logic [8-1:0] b,
  input  logic carry_in,
  input  logic sat_enable,
  input  logic [1:0] cmp_mode,
  input  logic [2:0] shift_amt,
  output logic [8-1:0] y,
  output logic carry_out,
  output logic zero,
  output logic negative,
  output logic cmp_out
);

    rtl u_rtl (.*);
endmodule
