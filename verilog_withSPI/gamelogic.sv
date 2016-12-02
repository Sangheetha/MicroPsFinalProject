module gamelogic(input  logic clk,
			     input  logic reset,
				 input  logic c1_data_in,
				 input  logic [7:0] activeopcode,
				 input  logic pulse_load,
				 output logic cclk,
				 output logic pulse_out,
				 output logic [23:0] reply,
				 output logic [1:0] level,
				 output logic [2:0] total_life,
				 output logic [3:0] key_pressed,
				 output logic correct_key_check,
				 output logic wrong_key_check,
				 output logic key_match_check);
				 //output logic seq_reset_check);
// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Arrowspace game logic module

	logic [3:0] next_key;
	logic [15:0] key_seq;
	logic seq_reset, key_matches, count_en, seq_done;
	logic timer_done, lose_life, start;
	logic correct_key, wrong_key;
	logic death;

	assign count_en = key_matches;
	assign death = 0; //death not implemented yet
	
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
	key_match_fsm match1(clk, reset, key_pressed, next_key, seq_done, timer_done, key_matches, seq_reset, lose_life,start);
	next_key_gen next1(clk, reset, seq_reset, count_en, key_seq, next_key, seq_done);
	key_seq_gen   seq1(clk, reset, timer_done, start, key_seq, level);

	// Current life points logic
	always_ff@(negedge clk, posedge reset)
		if(reset) 		   total_life <= 3'b111;
		else if(lose_life) total_life <= total_life - 3'b1;
				
	// Handles timing to send correct_key and wrong_key to Pi			
	always_ff@(negedge clk, posedge reset)
		if(reset) 			 correct_key <= 1'b0;
		else if(pulse_load)  correct_key <= 1'b0;
		else		  		 correct_key <= correct_key + key_matches;
	
	always_ff@(negedge clk, posedge reset)
		if(reset) 			 wrong_key <= 1'b0;
		else if(pulse_load)  wrong_key <= 1'b0;
		else				 wrong_key <= wrong_key + seq_reset;			
	
	// SPI reply logic
   always_comb
		casez(activeopcode)
			8'b0001zzzz:reply = {death, total_life,2'b00,level,key_seq}; // level_reply
			8'b0010zzzz:reply = {correct_key, wrong_key, 22'b0}; 			 // key_match_reply
    	default: reply = 24'h0;
	endcase
	
	// For debugging
	assign correct_key_check = correct_key;
	assign wrong_key_check = wrong_key;
	assign key_match_check = key_matches;
	//assign seq_reset_check = pulse_load;
	
endmodule 				  

