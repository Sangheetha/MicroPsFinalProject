		   
module key_match_fsm(input  logic clk,
			     input  logic reset,
				 input  logic [3:0] key_pressed,
				 input  logic [3:0] next_key,
				 input  logic seq_done,
				 input logic timer_done,
				 output logic key_matches,
				 output logic seq_reset,
				 output logic lose_life,
				 output logic start);
		   
// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Determine if player input matches next key in the sequence
	
	typedef enum logic [3:0] {S0,S1,S2,S3,S4,S5,S6,S7} statetype;
	statetype state, next;

	// state register
	always_ff@(posedge clk, posedge reset)
		if(reset) state <= S0; 
		else		 state <= next;
	
	// next state logic
	always_comb
		case(state)
			// scan state
			S0: if(timer_done)						next = S5;
				else if(seq_done)				      next = S3;
				else if(key_pressed == next_key) next = S1;
				else if (key_pressed != 0)  	  	next = S2;
				else 					     	  			next = S0;	
				
			// matched state
			S1: if(timer_done)			  next = S5;	 		 	  
				 else 						  next = S4;
			
			// reset state 	 
			S2: if(timer_done)			  next = S5;
				else							  next = S4;
			
			// sequence success state
			S3: if(timer_done)			  next = S0;
				  else 						  next = S3;
			
			// wait state
			S4: if(timer_done) 			  next = S5;	
				else if(key_pressed == 0) next = S0;
				else 						 	  next = S4;
				
			// sequence fail state
			S5: 								  next = S0;
			
			// initial state, start game
			S6: if(key_pressed == 4'h5)  next = S7;
				 else							  next = S6;
			S7: 								  next = S0;
			default:   						  next = S0;

		endcase
	
	// output logic
	assign key_matches = (state == S1);
	assign seq_reset = (state == S2) | timer_done;
	assign lose_life = (state == S5); 
	assign start = (state == S7);
	
endmodule 

module next_key_gen(input  logic clk,
			   input  logic reset,
			   input  logic seq_reset,
			   input  logic count_en,
				input logic [15:0] key_seq,
			   output logic [3:0] next_key,
			   output logic seq_done);
			   
// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Outputs the next key (key to match) in the sequence					

	logic [3:0] count, index;
				
	// Enabled counter, index of the sequence  
	always_ff@(posedge clk, posedge reset)
		if(reset) 	count <= 3'b0;
		else if(seq_reset) count <= 3'b0;
		else if(count_en) 		count <= count + 3'h4;
	
	// Reverse order so key_seq can be written MSB = first key
	always_comb
		case(count)
			4'b0000: index = 4'b1100;
			4'b0100: index = 4'b1000;
			4'b1000: index = 4'b0100;
			4'b1100: index = 4'b0000;
			default: index = 4'b0000;
		endcase
		
	assign next_key = {key_seq[index+3], key_seq[index+2],key_seq[index+1],key_seq[index]};
	assign seq_done = (next_key == 4'hF);
		
endmodule 	

module key_seq_gen(input logic clk,
							input logic reset,
							input logic timer_done,
							input logic start,
							output logic [15:0] key_seq,
							output logic [1:0] level);

// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Generates the next key sequence and level

	// Enabled counter, current level
    always_ff@(posedge clk, posedge reset)
		if(reset) 	level <= 2'b0;
		else if(timer_done | start) level <= level + 2'b1;
	
	always_comb
		case(level)
			//2'b0: key_seq = 16'h0000; // Default for initial state
			2'b0: key_seq = 16'h11FF;
			2'b1: key_seq = 16'h32FF; //LEFT, DOWN, LEFT
			2'b10: key_seq = 16'h422F; //RIGHT, DOWN, DOWN
			2'b11: key_seq = 16'h212F; // DOWN, UP, DOWN 
			default: key_seq = 16'hFFFF;
		endcase

endmodule

