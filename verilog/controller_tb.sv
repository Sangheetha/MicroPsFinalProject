module testbench();
	
	logic clk, reset, c1_data_in, cclk, pulse_out, done;
	logic [3:0] key_pressed;
	
	// generate clock
	always
		begin
			clk = 1; #5; clk = 0; #5;
		end
		
	// instantiate device under test
	controller dut(clk, reset, c1_data_in, cclk, pulse_out, done, key_pressed);

	initial
		begin
			reset = 1; #10; reset = 0;
		end
endmodule 
