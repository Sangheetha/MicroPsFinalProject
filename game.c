#include <stdio.h>

#include "fbdisplay.h"

#define START_SCREEN 0
#define PLAY_GAME_ONE 1

int main(int argc, char* argv[]) {
  //Setup Frame Buffer
  int* fbp;
  int fbfd;
  long int screensize_in_int;
  setUpFrameBuffer(&fbp,&screensize_in_int, &fbfd);
  
  int game_state;

  while(1) {
    //Grab the game_state from the FPGA w SPI.
    //For now, we're always playing the game
    game_state = PLAY_GAME_ONE

    if (game_state = START_SCREEN) {
        //Render start screen
    } 
    else if (game_state = PLAY_GAME_ONE) {
        
    }
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
    int screen[1920*1080]; //Temp array to hold next state of screen
    
    clearContents(screen,screensize_in_int);
    placeSprite(screen, &life);
    placeSprite(screen,&arrow);
    placeSprite(screen,&leftarrow);
    placeSprite(screen,&uparrow);
    placeSprite(screen,&downarrow);
    placeSprite(screen,&time);
    placeSprite(screen,&mark);

        

    updateScreen(fbp,screen,screensize_in_int);

    moveSpriteRight(&mark);
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
 
