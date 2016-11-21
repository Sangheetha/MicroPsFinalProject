module gamelogic(input  logic clk,
			      input  logic reset,
					input logic c1_data_in,
					input logic [7:0] activeopcode,
					output logic cclk,
					output logic pulse_out,
				   output logic [23:0] reply,
					output logic [1:0] level,
					output logic [2:0] total_life);
// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Arrowspace game logic module

	logic [3:0] next_key, key_pressed;
	logic [15:0] key_seq;
	//logic [1:0] level;
	//logic [2:0] total_life;	
	logic seq_reset, key_matches, count_en, seq_done;
	logic timer_done, lose_life;
	
	//pass to reply
	logic [23:0] level_reply;
	logic [23:0] key_match_reply;
	logic death = 0; //death not implemented yet
	
	assign count_en = key_matches;
	
	// Timer for each level
	// Each level is ~6 seconds
	logic [27:0] timer;
	logic clk_test;
	logic [3:0] timer_test;
	always_ff@(posedge clk, posedge reset)
		if (reset) timer <= 28'b0;
		else 			timer <= timer + 28'b1;
	assign timer_done = (timer == 28'b1111111111111111111111111111); 
	
	controller control1(clk,reset,c1_data_in,cclk,pulse_out,key_pressed);
	key_match_fsm match1(clk, reset, key_pressed, next_key, seq_done, timer_done, key_matches, seq_reset, lose_life);
	next_key_gen next1(clk, reset, seq_reset, count_en, key_seq, next_key, seq_done);
	key_seq_gen   seq1(clk, reset, timer_done, key_seq, level);

	always_ff@(negedge clk, posedge reset)
		if(reset) total_life <= 3'b111;
		else if(lose_life) total_life <= total_life - 3'b1;
				
	assign level_reply = {death, total_life,2'b00,level,key_seq};
	assign key_match_reply = {key_matches, 7'b0};
	
  always_comb
	casez(activeopcode)
    	8'b0001zzzz:reply = level_reply;
		8'b0010zzzz:reply = key_match_reply;
    	default: reply = 24'hDDDDDD;
	endcase
	
endmodule 				  

