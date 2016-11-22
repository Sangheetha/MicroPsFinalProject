module top(input logic clk,
       	input logic sck,
       	input logic sdi,
       	output logic sdo,
       	input logic load,
       	output logic done,
		input logic reset,
		input logic c1_data_in,
		output logic cclk,
		output logic pulse_out,
		output logic [1:0] level,
		output logic [2:0] total_life);
//INPUTS: 
// c1_data_in = controller data
//OUTPUTS:
// cclk = controller clock
// pulse_out = controller pulse

	logic [7:0] opcode;
	logic [23:0] reply;

	spi someSPI(sck,sdi,sdo,done,opcode,reply);
	core someCore(clk,load,opcode,done,reply,reset,c1_data_in, cclk, pulse_out, level, total_life);
		
endmodule

module spi(input logic sck,
       	input logic sdi,
       	output logic sdo,
       	input logic done,
       	output logic [7:0] opcode,
       	input logic [23:0] reply);

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

module core(input logic clk,
        	input logic load,
        	input logic[7:0] opcode,
	      output logic done,
        	output logic[23:0] reply,
			input logic reset,
			input logic c1_data_in,
			output logic cclk,
			output logic pulse_out,
			output logic [1:0] level,
			output logic [2:0] total_life);

  logic [7:0] activeopcode;
  logic [7:0] count;

  //Take in opcode as long as load is high
  always_ff@(posedge clk)
	 if(load) activeopcode <= opcode;

	 
	 /*
	 //Default reply
  always_comb
	casez(activeopcode)
    	8'b0001zzzz:reply = 24'h888D;
		8'b0010zzzz:reply = 24'hDAD0;
    	default: reply = 24'h0;
	endcase
	*/
  //counter
  always_ff@(posedge clk, posedge load)
    if (load) done <= 1'b0;
    else done <=1'b1;
	 	
	gamelogic game1(clk,reset,c1_data_in, activeopcode, cclk, pulse_out, reply, level, total_life);
	
endmodule
			  
				  