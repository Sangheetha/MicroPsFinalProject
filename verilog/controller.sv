module controller(input  logic clk,
				  input logic reset,
				  input logic c1_data_in,
				  output logic cclk,
				  output logic pulse_out,
				  output logic done,
				  output logic [3:0] key_pressed);
// Nancy Wei 11/14/16
// nwei@g.hmc.edu

	logic [17:0] c_count;
	logic pulse_en;

	//***Counter for controller clock and pulse enable
	always_ff@(posedge clk, posedge reset)
		if(reset) c_count <= 18'b0;
		else 		 c_count <= c_count + 18'b1;
		
	//***Generate pulse_out and cclk
	assign pulse_en = c_count[17]; // ~150 Hz
	assign cclk = c_count[8]; // ~78 kHz
	
	// For testbench
	//assign pulse_en = c_count[5];
	//assign cclk = c_count[0];
	
	
	//***Generate output pulse and data gathering pulse
	logic pulse_first, pulse_second;
	pulse c_pulse(cclk, reset, pulse_en, pulse_first);	
	
	always_ff@(posedge cclk, posedge reset)
		if(reset) pulse_second <= 0;
		else 		 pulse_second <= pulse_first;

	always_ff@(posedge cclk, posedge reset)
		if(reset) pulse_out <= 0;
		else 		 pulse_out <= pulse_second;
		
	//***Generate get_data signal
	logic get_data;
	logic [17:0] b_count;
	always_ff@(posedge cclk, posedge reset)
		if(reset) b_count <= 18'b0;
		else if(pulse_first) b_count <= 18'b0;
		else 						b_count <= b_count + 18'b1;
		
	assign get_data = (b_count <= 18'h8) & (b_count >= 18'h1);
	
	//***Receive controller data on negedge
	logic [7:0] c1_decode;
	always_ff@(negedge cclk, posedge reset)
		if(reset) 	 c1_decode <= 8'hFF;
		else if(get_data) c1_decode <= {c1_decode[6:0], c1_data_in};
	
	//***Change key_pressed only when data is done 
	assign done = (b_count == 18'h9);
	logic [7:0] c1_key;
	always_ff@(posedge cclk)
		if(reset) c1_key <= 8'hFF;
		else if(done) c1_key <= c1_decode;
	
	//***Decoder
	always_comb
		casez(c1_key)
			8'b0zzzzzzz: key_pressed = 4'h5; //A
			8'b10zzzzzz: key_pressed = 4'h6; //B
			8'b110zzzzz: key_pressed = 4'h7; //SELECT
			8'b1110zzzz: key_pressed = 4'h8; //START
			8'b11110zzz: key_pressed = 4'h1; //UP
			8'b111110zz: key_pressed = 4'h2; //DOWN
			8'b1111110z: key_pressed = 4'h3; //LEFT
			8'b11111110: key_pressed = 4'h4; //RIGHT
			default: 	 key_pressed = 4'h0; //NONE
		endcase
	
	
endmodule 	
	
	
module pulse(input  logic clk,	
				 input  logic reset,
				 input  logic A,
				 output logic Y);
// Nancy Wei 9/23/16
// nwei@g.hmc.edu
// Pulses signal Y for one clock cycle

	typedef enum logic [1:0] {S0,S1,S2} statetype;
	statetype state, next;
	
	// state register
	always_ff@(posedge clk, posedge reset)
		if(reset) state <= S0;
		else		 state <= next;
	
	// next state logic
	always_comb
		case(state)
			S0: if(A)  next = S1;
				 else   next = S0;
			S1: 		  next = S2;
			S2: if(~A) next = S0;
				 else   next = S2;
			default:   next = S0;
		endcase
	
	// output logic
	assign Y = (state == S1);
endmodule 