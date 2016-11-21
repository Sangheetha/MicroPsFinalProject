#include <stdio.h>

#include "EasyPIO.h"
#include "fbdisplay.h"

#define START_SCREEN 0
#define INIT_LEVEL_ONE 1
#define PLAY_LEVEL_ONE 2

#define MAX_KEYS 4

#define LOAD_PIN 23
#define DONE_PIN 24
#define LEVEL_INFO_OPCODE 0x10
#define KEY_MATCH_OPCODE 0x20

/////////////////////////////////////
///SPI Functions Prototype 
/////////////////////////////////////

void getLevelInfo(GameScreen *);

////////////////////////////////////
//Main
///////////////////////////////////

int main(int argc, char* argv[]) {
  //Setup I/O
  pioInit();
  spiInit(1000000,0);
  
  //Setup Frame Buffer
  int* fbp;
  int fbfd;
  long int screensize_in_int;
  setUpFrameBuffer(&fbp,&screensize_in_int, &fbfd);
  
  int game_state;
  int next_state;
  GameScreen screen;
  screen.size = 0;
  screen.arrow_index = 0;
  printf("before whilei\n");

  int count = 0;

  int wrong_button;
  int change_arrow;
    
  //Before start screen is made, we start with p1 game
  next_state = INIT_LEVEL_ONE;

  while(1) {
    //Grab the game_state from the FPGA w SPI.
    //For now, we're always playing the game
    game_state  = next_state;
    change_arrow = (count%50)? 0:1;
    wrong_button = (count == 76)? 1:0;

    switch(game_state)
    {   
        case START_SCREEN:
           //Render start screen lomo
        case INIT_LEVEL_ONE:
            //Initialize level for one-player
            
            clearContents(screen.pixel_arr,screensize_in_int);
            //TODO: Get life, get arrows, fake stuff here for now
            screen.life_1 = 10;
            addSpriteToGame(LIFE_BAR,&screen,0,0); 
            addSpriteToGame(TIMER_BAR,&screen,300,430);
            addSpriteToGame(TIMER_MARK,&screen,300,430);
            addSpriteToGame(RIGHT_ARROW,&screen,300,500);
            addSpriteToGame(DOWN_ARROW,&screen,410,500);
            addSpriteToGame(DOWN_ARROW,&screen,520,500);
            addSpriteToGame(LEFT_ARROW,&screen,630,500);
            updateGameScreenSinglePlayer(&screen);
            next_state = PLAY_LEVEL_ONE;
        case PLAY_LEVEL_ONE:
            //Play level for one-player
            //Check if the level is over, whether a wrong/correct button was
            //pressed.
            if (levelOver(&screen)) {
                printf("Time is up!\n");
                next_state = INIT_LEVEL_ONE;
                clearSprites(&screen); 
                //Send stuff to FPGA saying so
            } else {
                //TODO: Use SPI to initialize change_arrow and wrong_button
                if(change_arrow) {
                    changeArrowColor(&(screen.key_arr[screen.arrow_index]),GREEN);
                    screen.arrow_index++;
                } else if (wrong_button) {
                    resetKeys(&screen);
                }
            }
            moveSpriteRight(&(screen.timer_mark_1));
            updateGameScreenSinglePlayer(&screen);
            break;
        default:
            break;
   } 
    
   updateScreen(fbp,screen.pixel_arr,screensize_in_int);
   count++;
  }
  // cleanup
  tearDownFrameBuffer(fbp, fbfd, screensize_in_int);
  return 0;
}

///////////////////////////////////
// Functions
///////////////////////////////////

void getLevelInfo(GameScreen *g) {
    
    char death_life_level;
    char messArr[MAX_KEYS/2];
    size_t i, index, offset;
    int current_key;

    digitalWrite(LOAD_PIN,1);

    spiSendReceive(LEVEL_INFO_OPCODE);

    digitalWrite(LOAD_PIN,0);

    while(!digitalRead(DONE_PIN));

    death_life_level = spiSendReceive(0);
    for (i = 0; i < MAX_KEYS; i++) {
        messArr[i] = spiSendReceive(0);
    }
    
    g->life_1 = (death_life_level >> 4)&0x7; //Extract life
    g->level = death_life_level&0xF; //Extract level

    for (i = 0; i < MAX_KEYS; i ++) {
        index = i/2;
        offset = i%2;
        current_key = (messArr[index] >> 4*offset)&0xF;
        if (current_key == 0xF) {
            break;
        } else {
            addSpriteToGame(current_key, g, 300 + i*110, 500);
        }
    }
}

 
