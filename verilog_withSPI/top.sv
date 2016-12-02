module top(input  logic clk,
		input  logic reset,
       	input  logic sck,
       	input  logic sdi,
		input  logic load,
		input  logic c1_data_in,
       	output logic sdo,
       	output logic done,
		output logic cclk,
		output logic pulse_out,
		output logic [1:0] level,
		output logic [2:0] total_life,
		output logic [3:0] key_pressed,
		output logic correct_key_check,
		output logic wrong_key_check,
		output logic key_match_check,
		output logic seq_reset_check);
// Top level module for Arrowspace game

//INPUTS: 
// reset      = system reset
// sck        = SPI clock from Pi
// sdi        = SPI data in from Pi (MOSI)
// load       = SPI data is loading in
// c1_data_in = controller data
//OUTPUTS:
// sdo         = SPI data out to Pi (MISO)
// done        = SPI data is being sent to Pi
// cclk        = controller clock
// pulse_out   = controller pulse
// level       = current game level
// total_life  = current life level
// key_pressed = current key being pressed
// _check      = signals for debugging

	logic [7:0] opcode;
	logic [23:0] reply;

	spi someSPI(sck,sdi,sdo,done,opcode,reply);
	core someCore(clk,sck,reset,load,opcode,c1_data_in,done,reply,cclk, pulse_out, level, total_life, key_pressed, correct_key_check, wrong_key_check, key_match_check, seq_reset_check);		

endmodule

module spi(input logic sck,
       	input logic sdi,
       	output logic sdo,
       	input logic done,
       	output logic [7:0] opcode,
       	input logic [23:0] reply);
// SPI module for communication between FPGA and Pi

	logic wasdone, sdodelayed;
	logic[23:0] replycap;

	always_ff@(posedge sck)
    	if (!wasdone) {replycap,opcode} = {reply,opcode[6:0],sdi};
    	else {replycap,opcode} = {replycap[22:0],opcode,sdi};

	//sdo should change on negative edge of sck
	always_ff@(negedge sck)
    	begin
        	wasdone = done;
        	sdodelayed = replycap[22];
    	end

	//when done is first asserted, shift out msb before clock edge
	assign sdo = (done & !wasdone)? reply[23] : sdodelayed;
endmodule

module core(input  logic clk,
			input  logic sck,
			input  logic reset,
        	input  logic load,
        	input  logic[7:0] opcode,
	        input  logic c1_data_in,
			output logic done,
        	output logic[23:0] reply,
			output logic cclk,
			output logic pulse_out,
			output logic [1:0] level,
			output logic [2:0] total_life,
			output logic [3:0] key_pressed,
			output logic correct_key_check,
			output logic wrong_key_check,
			output logic key_match_check,
			output logic seq_reset_check);

  logic [7:0] activeopcode;
  logic [7:0] count;
  logic pulse_load;
  logic done_en, pulse_done;
  logic [20:0] done_count; 
  
  //Take in opcode as long as load is high
  always_ff@(posedge clk)
	 if(load) activeopcode <= opcode;

  //Pi starts receiving 
  always_ff@(posedge clk, posedge load)
    if (load) done <= 1'b0;
    else done <=1'b1;
	 
	always_ff@(posedge sck, posedge reset)
		if (reset) done_count = 21'b0;
		else if (load) done_count = 21'b0;
		else done_count <= done_count + 21'b1;
	assign done_en = (done_count == 21'd24);
	
	pulse p_done(clk, reset,done_en,pulse_done);	//resets correct or wrong key
	
	gamelogic game1(clk,reset,c1_data_in, activeopcode, pulse_done, cclk, pulse_out, reply, level, total_life, key_pressed, correct_key_check, wrong_key_check, key_match_check);
	
	assign seq_reset_check = pulse_done;
endmodule