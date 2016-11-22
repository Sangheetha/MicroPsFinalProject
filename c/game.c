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

int getLevelInfo(GameScreen *);
void keyMatch(int *, int *, int *);

////////////////////////////////////
//Main
///////////////////////////////////

int main(int argc, char* argv[]) {
  //Setup I/O
  pioInit();
  spiInit(244000,0);

  pinMode(LOAD_PIN,OUTPUT);
  pinMode(DONE_PIN,INPUT);
  
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
  int timer_done;
  int wrong_button = 0;
  int change_arrow = 0;
  int start;

  int not_close_game = 1;
    
  //Before start screen is made, we start with p1 game
  next_state = INIT_LEVEL_ONE;

  while(not_close_game) {
    //Grab the game_state from the FPGA w SPI.
    //For now, we're always playing the game
    game_state  = next_state;
    switch(game_state)
    {   
        case START_SCREEN:
           //Render start screen lomo
           break;
        case INIT_LEVEL_ONE:
            //Initialize level for one-player
            clearContents(screen.pixel_arr,screensize_in_int);
            if (count!=0) {
                clearSprites(&screen);
            }
            //TODO: Get life, get arrows, fake stuff here for now
            start = getLevelInfo(&screen);
            addSpriteToGame(LIFE_BAR,&screen,0,0); 
            addSpriteToGame(TIMER_BAR,&screen,300,430);
            addSpriteToGame(TIMER_MARK,&screen,300,430);
            updateGameScreenSinglePlayer(&screen);
            next_state = (start == 0)?PLAY_LEVEL_ONE:INIT_LEVEL_ONE;
            
            break;
        case PLAY_LEVEL_ONE:
            //Play level for one-player
            //Check if the level is over, whether a wrong/correct button was
            //pressed.
            timer_done = levelOver(&screen);
            keyMatch(&timer_done,&change_arrow,&wrong_button);
            if (timer_done) {
                printf("Time is up!\n");
                next_state = INIT_LEVEL_ONE;
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

int getLevelInfo(GameScreen *g) {
    
    char death_life_level;
    char messArr[MAX_KEYS/2];
    size_t i, index, offset;
    int current_key;

    digitalWrite(LOAD_PIN,1);

    spiSendReceive(LEVEL_INFO_OPCODE);

    digitalWrite(LOAD_PIN,0);

    while(!digitalRead(DONE_PIN));
    death_life_level = spiSendReceive(0);

    for (i = 0; i < MAX_KEYS/2; i++) {
        messArr[i] = spiSendReceive(0);
        if (messArr[i] == 0) {
            return -1;
        }
    }
    
    g->life_1 = (death_life_level >> 4)&0x7; //Extract life
    g->level = death_life_level&0xF; //Extract level


    for (i = 0; i < MAX_KEYS; i ++) {
        index = i/2;
        offset = 1 -i%2;
        current_key = (messArr[index] >> 4*offset)&0xF;
        if (current_key == 0xF) {
            break;
        } else {
            addSpriteToGame(current_key, g, 300 + i*110, 500);
        }
    }

    return 0;
}

void keyMatch(int * timer_done, int * change_arrow, int * wrong_key) {

   char match_info;

   char opcode = KEY_MATCH_OPCODE&(*timer_done);
 
   digitalWrite(LOAD_PIN, 1);
    
   spiSendReceive(0b00110000);

   digitalWrite(LOAD_PIN,0);

   while(!digitalRead(DONE_PIN));

   match_info = spiSendReceive(0);
   spiSendReceive(0);
   spiSendReceive(0);

   *change_arrow = (match_info >> 7)&0x1;
   *wrong_key = (match_info)&0x1;
   if (match_info != 0) {
    printf("match info is %x\n",match_info);
   }

}

 
