#include <stdio.h>

#include "fbdisplay.h"

#define START_SCREEN 0
#define INIT_LEVEL_ONE 1
#define PLAY_LEVEL_ONE 2


int main(int argc, char* argv[]) {
  //Setup Frame Buffer
  int* fbp;
  int fbfd;
  long int screensize_in_int;
  setUpFrameBuffer(&fbp,&screensize_in_int, &fbfd);
  
  int game_state;
  GameScreen screen;
  screen.size = 0;
  printf("before whilei\n");

  int count = 0;
  while(1) {
    //Grab the game_state from the FPGA w SPI.
    //For now, we're always playing the game
    game_state = (count == 0)? INIT_LEVEL_ONE:PLAY_LEVEL_ONE;

    switch(game_state)
    {   
        case START_SCREEN:
           //Render start screen lomo
        case INIT_LEVEL_ONE:
            //Initialize level for one-player
            //TODO: Get life, get arrows, fake stuff here
            screen.life_1 = 10;
            addSpriteToGame(LIFE_BAR,&screen,0,0); 
            addSpriteToGame(TIMER_BAR,&screen,300,430);
            addSpriteToGame(TIMER_MARK,&screen,300,430);
            updateGameScreenSinglePlayer(&screen);
        case PLAY_LEVEL_ONE:
            moveSpriteRight(&(screen.timer_mark_1));
            updateGameScreenSinglePlayer(&screen);
            //Play level for one-player
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
 
