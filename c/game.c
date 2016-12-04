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

#define MAX_KEYS 16

#define TIMER_MAX 0xFFFFF

#define LOAD_PIN 23
#define DONE_PIN 24
#define LEVEL_INFO_OPCODE 0x10
#define KEY_MATCH_OPCODE 0x20
#define GAME_STATUS_OPCODE 0x30

/////////////////////////////////////
///SPI Functions Prototype 
/////////////////////////////////////

int getStartInfo(GameScreen *);
int getLevelInfoOne(GameScreen *);
int getLevelInfoTwo(GameScreen *);
int keyMatch(size_t *, GameScreen *);
void generateKeys(GameScreen *, size_t *, size_t);
int sendKeySequenceOne(GameScreen *);
int sendKeySequenceTwo(GameScreen *); 

///////////////////////////////////
//Main
///////////////////////////////////

int main(int argc, char* argv[]) {
  //Setup I/O
  pioInit();
  spiInit(1000000,0);

  pinMode(LOAD_PIN,OUTPUT);
  pinMode(DONE_PIN,INPUT);
  
  //Setup Frame Buffer
  int* fbp;
  int fbfd;
  long int screensize_in_int;
  setUpFrameBuffer(&fbp,&screensize_in_int, &fbfd);
  
  int game_state;
  int next_state;
  Sprite sp; 
  int level;
  GameScreen screen;
  screen.key_seq[0] = 3;
  screen.key_seq[1] = 3;
  screen.key_seq[2] = 0x3;
  screen.key_seq[3] = 0xF;
  screen.key_seq[4] = 0xF;
  screen.key_seq[5] = 0xF;//0x1;
  screen.key_seq[6] = 0xF;//0x4;
  screen.key_seq[7] = 0xF;//4;
  screen.key_seq[8] = 0xF;//1;
  screen.key_seq[9] = 0xF;//2;
  screen.key_seq[10] = 0xF;//1;
  screen.key_seq[11] = 0xF;//3;
  screen.key_seq[12] = 0xF;//3;
  screen.key_seq[13] = 0xF;
  screen.key_seq[14] = 0xF;
  screen.key_seq[15] = 0xF;
  intializeScreen(&screen);

  printf("before whilei\n");

  size_t timer_data;
  int timer_count;
  int start;

  int not_close_game = 1;
    
  next_state = START_SCREEN ;
  clearContents(screen.pixel_arr,screensize_in_int);

  while(not_close_game) {
    game_state  = next_state;
    switch(game_state)
    {   
        case START_SCREEN:
           makeStartScreen(&screen);
           next_state = getStatusInfo(&screen);
           break;
        case INIT_LEVEL_ONE:
            screen.mode = 1;
            //Initialize level for one-player
            clearContents(screen.pixel_arr,screensize_in_int); 
            //start = getLevelInfoOne(&screen);
            start = sendKeySequenceOne(&screen);
            printf("Initializing a level: %d\n",screen.level);
            if (start == -1) {
                next_state = DEATH_SCREEN_ONE;
                break;
            }
            initializeLevel(&screen);
            updateGameScreenSinglePlayer(&screen);
            next_state = PLAY_LEVEL_ONE;
            break;
        case PLAY_LEVEL_ONE:
            //Play level for one-player
            //Check if the level is over, whether a wrong/correct button was
            //pressed.
            keyMatch(&timer_data,&screen);
            setTimerPos(&(screen.timer_mark_1), timer_data, TIMER_MAX, 300);
            if (timer_data > 0xF7000 | screen.life_1 == 0) {
                clearSprites(&screen);
                printf("%x\n",timer_data);
                next_state = INIT_LEVEL_ONE;
                screen.key_seq[0] = ((screen.key_seq[0] + 1)%4) + 1;
                break;
            } else {
                if(screen.correct_key_1) {
                    if (screen.arrow_index_1 < screen.size) {
                        changeArrowColor(&(screen.key_arr_1[screen.arrow_index_1]),GREEN);
                        screen.arrow_index_1++;
                    }
                } else if (screen.wrong_key_1) {
                    resetKeys(&screen,1);
                }
            }
            updateGameScreenSinglePlayer(&screen);
            break;
        case INIT_LEVEL_TWO:
            screen.mode = 2;
            //Initialize level for two players
            clearContents(screen.pixel_arr,screensize_in_int);
            start = sendKeySequenceTwo(&screen);
            next_state = PLAY_LEVEL_TWO;
            break;
        case PLAY_LEVEL_TWO:
            makeChar(&sp,'A',500,500);
            placeSprite(screen.pixel_arr,&sp);
            break;
        case DEATH_SCREEN_ONE:
            clearContents(screen.pixel_arr,screensize_in_int);
            next_state = getStatusInfo(&screen);
            break;
        case DEATH_SCREEN_TWO:
            next_state = getStatusInfo(&screen);
            break;
        default:
            break;
   }
   updateScreen(fbp,screen.pixel_arr,screensize_in_int);
  }
  // cleanup
  tearDownFrameBuffer(fbp, fbfd, screensize_in_int);
  return 0;
}

///////////////////////////////////
// Functions
///////////////////////////////////

int getStatusInfo(GameScreen *g) {
    char status,start,death,one,two;
    
    digitalWrite(LOAD_PIN,1);

    spiSendReceive(GAME_STATUS_OPCODE);
    spiSendReceive(0x11);
    spiSendReceive(0xFF);
    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0x0);
    spiSendReceive(0x0);
    spiSendReceive(0);
    spiSendReceive(0);


    digitalWrite(LOAD_PIN,0);

    while(!digitalRead(DONE_PIN));

    status = (spiSendReceive(0)>>4)&0xF;
    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);

    start = status & 0b1000;
    death = status & 0b0100;
    one = status & 0b0010;
    two = status & 0b0001;
    
    if (start) {
        return START_SCREEN;
    } else if (death) {
        death = (g->mode == 1)? DEATH_SCREEN_ONE:DEATH_SCREEN_TWO;
        return death;
    } else if (one) {
        return INIT_LEVEL_ONE;
    } else if (two) {
        return INIT_LEVEL_TWO;
    } else {
        return -1;
    }

}

int getLevelInfoOne(GameScreen *g) {
    
    char death_life_level;
    char messArr[MAX_KEYS/2];
    size_t i, index, offset;
    int current_key;


    digitalWrite(LOAD_PIN,1);

    spiSendReceive(LEVEL_INFO_OPCODE);
    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);

    /*spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);*/


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

int keyMatch(size_t * timer_data, GameScreen * g) {

   char match_timer_info, timer_info_1, timer_info_2, life_info;

   char opcode = KEY_MATCH_OPCODE;
   int level;
 
   digitalWrite(LOAD_PIN, 1);
    
   spiSendReceive(0x20);
   spiSendReceive(0);
   spiSendReceive(0);
   spiSendReceive(0);
   spiSendReceive(0);

   spiSendReceive(0);
   spiSendReceive(0);
   spiSendReceive(0);
   spiSendReceive(0);


   digitalWrite(LOAD_PIN,0);

   while(!digitalRead(DONE_PIN));

   life_info = spiSendReceive(0);
   match_timer_info = spiSendReceive(0);
   timer_info_1 = spiSendReceive(0);
   timer_info_2 = spiSendReceive(0);

   g->correct_key_1 = (match_timer_info >> 7)&0x1;
   g->wrong_key_1 = (match_timer_info >> 6)&0x1;
   g->life_1 = (life_info >>4)&0x7;


   *timer_data = ((match_timer_info & 0xF) << 16) | (timer_info_1 << 8) | timer_info_2;
   return level;
}

void generateKeys(GameScreen * g, size_t * key_seq, size_t numKeys) {
    size_t i,rand_key;
    srand(time(NULL));
    for (i = 0; i < numKeys; i++) {
        rand_key = (rand()%4);
    } 
    for (; i < 20; i++) {
        rand_key = 0xF;
    }

}

int sendKeySequenceOne(GameScreen* g) {
    char death_life_level,message;
    int current_key;
    int key_y,line_num;

    digitalWrite(LOAD_PIN, 1);
    spiSendReceive(0x10);
    
    size_t i;
    for (i = 0; i < MAX_KEYS; i = i+2) {
        message = (g->key_seq[i] << 4) | g->key_seq[i+1];
        spiSendReceive(message);
    }
    digitalWrite(LOAD_PIN, 0);

    while(!digitalRead(DONE_PIN));
    death_life_level = spiSendReceive(0);

    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);
   
    g->life_1 = (death_life_level >> 4)&0x7; //Extract life

    if (g->life_1 == 0) {
        return -1;
    }
    g->level = death_life_level&0xF; //Extract level
    
    for (i = 0; i < MAX_KEYS; i++) {
        current_key = g->key_seq[i];
        if (current_key != 0xF) {
            key_y = (i < 7)? 500: 610;
            line_num = i%7;
            addSpriteToGame(current_key, g, 300 + line_num*110, key_y);
        } else {
            break;
         }
    }

    return 0;
}
 
int sendKeySequenceTwo(GameScreen* g) {
    char death_life_level,message;
    int current_key;
    
    digitalWrite(LOAD_PIN, 1);
    spiSendReceive(0x10);
    
    size_t i;
    for (i = 0; i < MAX_KEYS; i = i+2) {
        message = (g->key_seq[i] << 4) | g->key_seq[i+1];
        spiSendReceive(message);
    }
    digitalWrite(LOAD_PIN, 0);

    while(!digitalRead(DONE_PIN));
    death_life_level = spiSendReceive(0);

    spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);
   
    g->life_1 = (death_life_level >> 4)&0x7; 
    g->life_2 = death_life_level&0x7;

    //If either player has died, we return
    if (g->life_1 == 0) {
        return -1;
    } else if (g->life_2 == 0) {
        return -2;
    }

    g->level = death_life_level&0xF; //Extract level
    
    for (i = 0; i < MAX_KEYS; i++) {
        current_key = g->key_seq[i];
        if (current_key != 0xF) {
            addSpriteToGame(current_key, g, 300 + i*110, 500);
        } else {
            break;
         }
    }

    return 0;
}
