#include <stdio.h>

#include "EasyPIO.h"
#include "fbdisplay.h"

#define START_SCREEN 0
#define INIT_LEVEL_ONE 1
#define PLAY_LEVEL_ONE 2
#define INIT_LEVEL_TWO 3
#define PLAY_LEVEL_TWO 4
#define DEATH_SCREEN_ONE 5
#define DEATH_SCREEN_TWO 6

#define MAX_KEYS 4

#define TIMER_MAX 0x3FFFFF

#define LOAD_PIN 23
#define DONE_PIN 24
#define LEVEL_INFO_OPCODE 0x10
#define KEY_MATCH_OPCODE 0x20

/////////////////////////////////////
///SPI Functions Prototype 
/////////////////////////////////////

int getLevelInfo(GameScreen *);
void keyMatch(size_t *, int *, int *);

////////////////////////////////////
//Main
///////////////////////////////////

int main(int argc, char* argv[]) {
  //Setup I/O
  pioInit();
  spiInit(488000,0);

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
  Sprite sp;
  screen.size = 0;
  screen.arrow_index_1 = 0;
  screen.arrow_index_2 = 0;
  printf("before whilei\n");

  int count = 0;
  size_t timer_data;
  int timer_count;
  int wrong_button = 0;
  int change_arrow = 0;
  int start;

  int not_close_game = 1;
    
  //Before start screen is made, we start with p1 game
  next_state = START_SCREEN;

  while(not_close_game) {
    //Grab the game_state from the FPGA w SPI.
    //For now, we're always playing the game
    game_state  = next_state;

    switch(game_state)
    {   
        case START_SCREEN:
           makeNum(&sp,5,800,500); 
           placeSprite(screen.pixel_arr,&sp);
           start = getStartInfo(&screen);
           next_state = (start == 0)?INIT_LEVEL_ONE:START_SCREEN;
           break;
        case INIT_LEVEL_ONE:
            //Initialize level for one-player
            clearContents(screen.pixel_arr,screensize_in_int); 
            start = getLevelInfo(&screen);
                printf("Initializing a level: %d\n",screen.level);
            if (start == -1) {
                next_state = DEATH_SCREEN_ONE;
                break;
            }
            addSpriteToGame(LIFE_BAR_1,&screen,0,0); 
            addSpriteToGame(LEVEL,&screen,1000,50);
            addSpriteToGame(TIMER_BAR_1,&screen,300,430);
            addSpriteToGame(TIMER_MARK_1,&screen,300,430);
            updateGameScreenSinglePlayer(&screen);
            next_state = PLAY_LEVEL_ONE;
            break;
        case PLAY_LEVEL_ONE:
            //Play level for one-player
            //Check if the level is over, whether a wrong/correct button was
            //pressed.
            keyMatch(&timer_data,&change_arrow,&wrong_button);
            setTimerPos(&(screen.timer_mark_1), timer_data, TIMER_MAX, 300);
            if (timer_data > 0x3f6000) {
                clearSprites(&screen);
                printf("%x\n",timer_data);
                next_state = INIT_LEVEL_ONE;
                break;
            } else {
                if(change_arrow) {
                    changeArrowColor(&(screen.key_arr[screen.arrow_index_1]),GREEN);
                    screen.arrow_index_1++;
                } else if (wrong_button) {
                    if (screen.arrow_index_1 < screen.size) {
                        resetKeys(&screen);
                    }
                }
            }
            updateGameScreenSinglePlayer(&screen);
            break;
        case INIT_LEVEL_TWO:
            break;
        case PLAY_LEVEL_TWO:
            break;
        case DEATH_SCREEN_ONE:
            clearContents(screen.pixel_arr,screensize_in_int);
            start = getStartInfo(&screen);
            next_state = (start == 0)?DEATH_SCREEN_ONE:START_SCREEN;
            break;
        case DEATH_SCREEN_TWO:
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

int getStartInfo(GameScreen *g) {
    digitalWrite(LOAD_PIN,1);

    spiSendReceive(LEVEL_INFO_OPCODE);

    digitalWrite(LOAD_PIN,0);

    while(!digitalRead(DONE_PIN));

    spiSendReceive(0);

    if (spiSendReceive(0) == 0) {
        return -1;
    } else {
        return 0;
    }


}

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
    }
   
    g->life_1 = (death_life_level >> 4)&0x7; //Extract life
    if (g->life_1 == 0) {
        return -1;
    }
    g->level = death_life_level&0xF; //Extract level

    for (i = 0; i < MAX_KEYS; i++) {
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

void keyMatch(size_t * timer_data, int * change_arrow, int * wrong_key) {

   char match_timer_info, timer_info_1, timer_info_2;

   char opcode = KEY_MATCH_OPCODE;
 
   digitalWrite(LOAD_PIN, 1);
    
   spiSendReceive(0x20);

   digitalWrite(LOAD_PIN,0);

   while(!digitalRead(DONE_PIN));

   match_timer_info = spiSendReceive(0);
   timer_info_1 = spiSendReceive(0);
   timer_info_2 = spiSendReceive(0);

   *change_arrow = (match_timer_info >> 7)&0x1;
   *wrong_key = (match_timer_info >> 6)&0x1;

   *timer_data = ((match_timer_info & 0x3F) << 16) | (timer_info_1 << 8) | (timer_info_2);
   //printf("timer data is %x\n", *timer_data);
}

 
