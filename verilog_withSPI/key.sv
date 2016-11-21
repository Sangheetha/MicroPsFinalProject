		   
module key_match_fsm(input  logic clk,
			     input  logic reset,
				 input  logic [3:0] key_pressed,
				 input  logic [3:0] next_key,
				 input  logic seq_done,
				 input logic timer_done,
				 output logic key_matches,
				 output logic seq_reset,
				 output logic lose_life);
		   
// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Determine if player input matches next key in the sequence


	
	typedef enum logic [3:0] {S0,S1,S2,S3,S4,S5} statetype;
	statetype state, next;

	// state register
	always_ff@(posedge clk, posedge reset)
		if(reset) state <= S0;
		else		 state <= next;
	
	// next state logic
	always_comb
		case(state)
			// scan state
			S0: if(timer_done)				next = S5;
				else if(seq_done)				      next = S3;
				else if(key_pressed == next_key)  next = S1;
				else if (key_pressed != 0)  	  next = S2;
				else 					     	  next = S0;	
				
			// matched state
			S1: if(timer_done)				next = S5;	 		 	  
				 else 							next = S4;
			
			// reset state 	 
			S2: if(timer_done)				next = S5;
				else								next = S4;
			
			// sequence success state
			S3: if(timer_done)			  next = S0;
				  else 						 next = S3;
			
			// wait state
			S4: if(timer_done) 			 next = S5;	
				else if(key_pressed == 0) 		 	  next = S0;
				else 						 	  next = S4;
				
			// sequence fail state
			S5: 									next = S0;
			
			default:   							  next = S0;

		endcase
	
	// output logic
	assign key_matches = (state == S1);
	assign seq_reset = (state == S2) | timer_done;
	assign lose_life = (state == S5); 
	
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

	logic [3:0] count;
				
	// Enabled counter, index of the sequence  
	always_ff@(posedge clk, posedge reset)
		if(reset) 	count <= 3'b0;
		else if(seq_reset) count <= 3'b0;
		else if(count_en) 		count <= count + 3'h4;
		
	assign next_key = {key_seq[count+3], key_seq[count+2],key_seq[count+1],key_seq[count]};
	assign seq_done = (next_key == 4'hF);
		
endmodule 	

module key_seq_gen(input logic clk,
							input logic reset,
							input logic timer_done,
							output logic [15:0] key_seq,
							output logic [1:0] level);

// Nancy Wei 11/14/16
// nwei@g.hmc.edu
// Generates the next key sequence and level

	// Enabled counter, current level
    always_ff@(posedge clk, posedge reset)
		if(reset) 	level <= 2'b0;
		else if(timer_done) level <= level + 2'b1;
	
	always_comb
		case(level)
			2'b0: key_seq = 16'hFF23; //LEFT, DOWN
			2'b1: key_seq = 16'hF224; //RIGHT, DOWN, DOWN
			2'b10: key_seq = 16'hF212; // DOWN, UP, DOWN 
			default: key_seq = 16'hFFFF;
		endcase

endmodule

