#include <stdio.h>

#include "EasyPIO.h"
#include "fbdisplay.h"

#define START_SCREEN 0
#define INIT_LEVEL_ONE 1
#define PLAY_LEVEL_ONE 2


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
    

  while(1) {
    //Grab the game_state from the FPGA w SPI.
    //For now, we're always playing the game
    game_state  = (count == 0)? INIT_LEVEL_ONE:next_state;
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
  Sprite life, time, mark;
  makeLifeBar(&life,9);
  makeTimerBar(&time,300,430);
  makeTimerMark(&mark,315,415);
       
  Sprite arrow;
  makeRightArrow(&arrow,300,500,BLUE);
  Sprite leftarrow;
  makeLeftArrow(&leftarrow,410,500,BLUE);
  Sprite uparrow;
  makeUpArrow(&uparrow,520,500,BLUE);
  Sprite downarrow;
  makeDownArrow(&downarrow,630,500,GREEN);

  while(1) {
   /* 
    clearContents(screen,screensize_in_int);
    placeSprite(screen, &life);
    placeSprite(screen,&arrow);
    placeSprite(screen,&leftarrow);
    placeSprite(screen,&uparrow);
    placeSprite(screen,&downarrow);
    placeSprite(screen,&time);
    placeSprite(screen,&mark);

        

    updateScreen(fbp,screen,screensize_in_int);

    moveSpriteRight(&mark);*/
  }
  free(arrow.pixel_arr);
  free(leftarrow.pixel_arr);
  free(uparrow.pixel_arr);
  free(downarrow.pixel_arr);   
  free(time.pixel_arr);
  free(life.pixel_arr);
  free(mark.pixel_arr);
       
  // cleanup
  tearDownFrameBuffer(fbp, fbfd, screensize_in_int);
  return 0;
}
 
