module arrowspace(input  logic clk,
			      input  logic reset,
					input logic c1_data_in,
					output logic cclk,
					output logic pulse_out,
					output logic done,
				  output logic win,
				  output logic [2:0] total_life,
				  output logic [3:0] key_out,
				  output logic [1:0] level);

// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Top level Arrowspace game module

	logic [3:0] next_key;
	logic seq_reset, key_matches, count_en, seq_done;
	logic [15:0] key_seq;
	
	logic timer_done, lose_life;
	
	//Output to LEDs for testing
	//logic [1:0] level;
	//logic [2:0] total_life;

	logic [3:0] key_pressed;
	assign count_en = key_matches;
	assign key_out = key_pressed;
	// Timer for each level
	/*
	
	logic [5:0] timer;
	always_ff@(posedge clk, posedge reset)
		if (reset) timer <= 6'b0;
		else 			timer <= timer + 6'b1;
	assign timer_done = (timer == 6'b111111); 
	*/
	
	logic [27:0] timer;
	logic clk_test;
	logic [3:0] timer_test;
	always_ff@(posedge clk, posedge reset)
		if (reset) timer <= 28'b0;
		else 			timer <= timer + 28'b1;
	assign timer_done = (timer == 28'b1111111111111111111111111111); 
	
	/*
	always_ff@(posedge clk_test, posedge reset)
		if (reset) timer_test <= 3'b0;
		else 			timer_test <= timer_test + 3'b1;
	assign timer_done = (timer_test == 3'b111);
	*/
	controller control1(clk,reset,c1_data_in,cclk,pulse_out,done,key_pressed);
	key_match_fsm match1(clk, reset, key_pressed, next_key, seq_done, timer_done, key_matches, seq_reset, lose_life);
	next_key_gen next1(clk, reset, seq_reset, count_en, key_seq, next_key, seq_done);
	key_seq_gen   seq1(clk, reset, timer_done, key_seq, level);

	assign win = (level == 2'b10);
	
	always_ff@(negedge clk, posedge reset)
		if(reset) total_life <= 3'b111;
		else if(lose_life) total_life <= total_life - 3'b1;
					  
endmodule 				  

