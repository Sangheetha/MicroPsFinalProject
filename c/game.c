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
///SPI and KeyGen Functions Prototype 
/////////////////////////////////////

int getStartInfo(GameScreen *);
void keyMatch(size_t *, GameScreen *);
int sendKeySequenceOne(GameScreen *);
int sendKeySequenceTwo(GameScreen *); 

void updateKeys(GameScreen *);
void generateKeys(GameScreen *);
void processKeyLine(GameScreen*, char*, ssize_t);

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
  

  srand(time(NULL));
  int blink = 0;
  int game_state;
  int next_state;
  GameScreen screen;
  


  size_t timer_data;
  int timer_count;
  int start;
  int color = BLUE;
  Sprite sp;

  int not_close_game = 1;
    
  next_state = START_SCREEN;//START_SCREEN;
  clearContents(screen.pixel_arr,screensize_in_int);

  while(not_close_game) {
    game_state  = next_state;
    switch(game_state)
    {   
        case START_SCREEN:
           clearContents(screen.pixel_arr,screensize_in_int);
           screen.level = 0;
           initializeScreen(&screen);

           makeStartScreen(&screen, color);
           blink = (blink + 1)%4;
           if (blink == 0) {
            if (color == GREEN) {color = BLUE;} else {color=GREEN;}}
           next_state = getStatusInfo(&screen);
           break;
        case INIT_LEVEL_ONE:
            screen.mode = 1;
            //Initialize level for one-player
            clearContents(screen.pixel_arr,screensize_in_int); 
            //generateKeys(&screen);
            updateKeys(&screen);
            start = sendKeySequenceOne(&screen);
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
            setTimerPos(&(screen.timer_mark_1), timer_data, TIMER_MAX, TIME_1_X_POS);
            if (timer_data > 0xFb000 | screen.life_1 == 0) {
                clearSprites(&screen);
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
            updateKeys(&screen);
            //generateKeys(&screen);
            start = sendKeySequenceTwo(&screen);
            if (start == -1) {
                next_state = DEATH_SCREEN_TWO;
                break;
            }
            initializeLevelTwo(&screen);
            updateGameScreenTwoPlayer(&screen);
            next_state = PLAY_LEVEL_TWO;
            break;
        case PLAY_LEVEL_TWO:
            keyMatch(&timer_data,&screen);
            setTimerPos(&(screen.timer_mark_1),timer_data,TIMER_MAX,TIME_1_X_POS);
            setTimerPos(&(screen.timer_mark_2),timer_data,TIMER_MAX, TIME_2_X_POS);
            
            //If timer is up or a player died, go back to init state
            if (timer_data > 0xF7000 | screen.life_1 == 0 | screen.life_2 == 0) {
                clearSprites(&screen);
                next_state = INIT_LEVEL_TWO;
                screen.key_seq[0] = ((screen.key_seq[0] + 1)%4) + 1;
                break;
            } else {
                // Else, check key press data to update screen.

                if (screen.correct_key_1) {
                    if (screen.arrow_index_1 < screen.size) {
                        changeArrowColor(&(screen.key_arr_1[screen.arrow_index_1]),GREEN);
                        screen.arrow_index_1++;
                    }
                } else if (screen.wrong_key_1) {
                    resetKeys(&screen,1);
                }
                if (screen.correct_key_2) {
                    if (screen.arrow_index_2 < screen.size) {
                        changeArrowColor(&(screen.key_arr_2[screen.arrow_index_2]),GREEN);
                        screen.arrow_index_2++;
                    }
                } else if (screen.wrong_key_2) {
                    resetKeys(&screen,2);
                }
            }
            updateGameScreenTwoPlayer(&screen);
            break;
        case DEATH_SCREEN_ONE:
            clearContents(screen.pixel_arr,screensize_in_int);
            makeDeathScreenOne(&screen);
            next_state = getStatusInfo(&screen);
            break;
        case DEATH_SCREEN_TWO:
            clearContents(screen.pixel_arr,screensize_in_int);
            makeDeathScreenTwo(&screen);
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
void keyMatch(size_t * timer_data, GameScreen * g) {

   char match_timer_info, timer_info_1, timer_info_2, life_info;
 
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
   g->correct_key_2 = (match_timer_info >> 5)&0x1;
   g->wrong_key_2 = (match_timer_info >> 4)&0x1;

   g->life_1 = (life_info >>4)&0x7;
  g->life_2 = life_info&0x7;


   *timer_data = ((match_timer_info & 0xF) << 16) | (timer_info_1 << 8) | timer_info_2;
}

void generateKeys(GameScreen * g) {
    size_t i,rand_key;
    if (g->num_keys > 15) {
        g->num_keys = 15;
    }
    for (i = 0; i < g->num_keys; i++) {
        rand_key = (rand()%4) + 1;
        g->key_seq[i] = rand_key;
    } 
    for (; i < MAX_KEYS; i++) {
        rand_key = 0xF;
        g->key_seq[i] = rand_key;
    }

}

int sendKeySequenceOne(GameScreen* g) {
    char death_life,message,level;
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
    death_life = spiSendReceive(0);

    level = spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);
   
    g->life_1 = (death_life >> 4)&0x7; //Extract life
 
    if (g->life_1 == 0) {
        return -1;
    }
    g->level = level; //Extract level
    
    for (i = 0; i < MAX_KEYS; i++) {
        current_key = g->key_seq[i];
        if (current_key != 0xF) {
            key_y = (i < 7)? 500: 610;
            line_num = i%7;
            addSpriteToGame(current_key, g, TIME_1_X_POS + line_num*110, key_y);
        } else {
            break;
         }
    }

    return 0;
}
 
int sendKeySequenceTwo(GameScreen* g) {
    char death_life,message,level;
    int current_key, key_y, line_num;
    
    digitalWrite(LOAD_PIN, 1);
    spiSendReceive(0x10);
    
    size_t i;
    for (i = 0; i < MAX_KEYS; i = i+2) {
        message = (g->key_seq[i] << 4) | g->key_seq[i+1];
        spiSendReceive(message);
    }
    digitalWrite(LOAD_PIN, 0);

    while(!digitalRead(DONE_PIN));
    death_life = spiSendReceive(0);

    level = spiSendReceive(0);
    spiSendReceive(0);
    spiSendReceive(0);
   
    g->life_1 = (death_life >> 4)&0x7; 
    g->life_2 = death_life&0x7;
    //If either player has died, we return
    if (g->life_1 == 0 && g->life_2 == 0) {
        g->winner = 0;
        return -1;
    } else if (g->life_2 == 0) {
        g->winner = 1;
        return -1;
    } else if (g->life_1 == 0) {
        g->winner = 2;
        return -1;
    }

    g->level = level; //Extract level
       
    for (i = 0; i < MAX_KEYS; i++) {
        current_key = g->key_seq[i];
        if (current_key != 0xF) {
            key_y = (i < 5)? 500: 610;
            line_num = i%5;
            addSpriteToGame(current_key, g, TIME_1_X_POS + line_num*110, key_y);
        } else {
            break;
         }
    }

    return 0;
}

void updateKeys(GameScreen * g) {
    FILE *fp;
    char* line = NULL;
    size_t len = 0;
    int count = 0;
    ssize_t read;

    fp = fopen("/home/pi/key_seq.dat","r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(0);
    }

    while ((read = getline(&line,&len,fp)) !=-1){
        if (count == g->level) {
            processKeyLine(g,line,read);
            g->key_src = 1;
            return;
        }
        count++;
    }

    g->key_src = 0;
    generateKeys(g);
}

void processKeyLine(GameScreen * g, char * line, ssize_t len) {
   size_t i;
   int key;
   len = len - 1;

   for (i = 0; i < len; i++) {
     key = line[i] - 48; //Most encodings have a difference of 48 between '1' and 1
     if (key >  0 && key < 5) {
        g->key_seq[i] = key;
     } else {
       //the key sequence terminates early
        g->key_seq[i] = 0xF;
    }
   }

   for (; i < MAX_KEYS; i++) {
     g->key_seq[i] = 0xF;
   }

}
