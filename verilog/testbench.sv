module testbench();
	
	logic clk, reset, win;
	logic [3:0] key_pressed;
	logic passed;
	
	// generate clock
	always
		begin
			clk = 1; #5; clk = 0; #5;
		end
		
	// instantiate device under test
	arrowspace dut(clk, reset, key_pressed, win);

	
  // apply inputs
  initial begin 
  reset = 1; #20; reset = 0;
  
  key_pressed = 4'b0;
  #10;
  
  // First key match
  key_pressed = 4'b1;
  #20;
  key_pressed = 4'b0;
  #20;
  
  //trigger a reset
  key_pressed = 4'b1;
  #20;
  key_pressed = 4'b0;
  #20;
  
  // restart
  key_pressed = 4'b1;
  #20;
  key_pressed = 4'b0;
  #20;
  key_pressed = 4'b10;
  #20;
  key_pressed = 4'b0;
  #20;
  key_pressed = 4'b11;
  #20;
  key_pressed = 4'b0;
  #20;
  
  end
endmodule 
