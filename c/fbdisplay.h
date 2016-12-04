#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define SCREEN_HEIGHT 1080
#define SCREEN_WIDTH 1920

#define BLACK 0x0
#define WHITE 0xffffff
#define BLUE 0xf0
#define GREEN 0x42f450
#define RED 0xff3300
#define YELLOW 0xffd400
#define BACKGROUND BLACK

#define LIFE_BAR_HEIGHT 75
#define LIFE_BAR_MAX_WIDTH 400
#define LIFE_POINTS 7
#define LIFE_BAR_X_POS 50
#define LIFE_BAR_Y_POS 50

#define CHAR_HEIGHT 75
#define CHAR_WIDTH 50
#define CHAR_THICK CHAR_WIDTH/4

#define LEVEL_WIDTH (CHAR_WIDTH*5 +50)
#define LEVEL_HEIGHT CHAR_HEIGHT
#define LEVEL_X_POS 1750
#define LEVEL_Y_POS 50

#define ARROW_HEIGHT_WIDTH 100

#define TIME_HEIGHT 50
#define TIME_WIDTH 450
#define TIME_Y_POS 430
#define TIME_1_X_POS 200
#define TIME_2_X_POS 1200


#define LIFE_BAR_1 0
#define RIGHT_ARROW 4
#define LEFT_ARROW 3
#define DOWN_ARROW 2
#define UP_ARROW 1
#define TIMER_BAR_1 5
#define TIMER_MARK_1 6
#define LEVEL 7
#define LIFE_BAR_2 8
#define TIMER_BAR_2 9
#define TIMER_MARK_2 10
#define INVALID_SPRITE 0xF

struct Sprite {
    int * pixel_arr;
    int width;
    int height;
    int x_pos;
    int y_pos;
 };

typedef struct Sprite Sprite;

struct GameScreen {
    int pixel_arr[SCREEN_WIDTH*SCREEN_HEIGHT];
    size_t key_seq[20];
    size_t level;
    size_t size;
    int winner;
    int mode;
    int key_src;
    Sprite level_sprite;
    Sprite level_num_0;
    Sprite level_num_1;

    size_t arrow_index_1;
    int correct_key_1;
    int wrong_key_1;
    int life_1;
    Sprite timer_bar_1;
    Sprite timer_mark_1;
    Sprite life_bar_1;
    Sprite key_arr_1[20];
    
    size_t arrow_index_2;
    int correct_key_2;
    int wrong_key_2;
    int life_2;
    Sprite timer_bar_2;
    Sprite timer_mark_2;
    Sprite life_bar_2;
    Sprite key_arr_2[20];
};

typedef struct GameScreen GameScreen;


void initializeScreen(GameScreen *g) {
    g->size = 0;
    g->arrow_index_1 = 0;
    g->arrow_index_2 = 0;
    g->winner = 0;
}

void clearContents(int *fbp, long int screensize_int) {

    int* lp;
    for (lp = fbp; lp < fbp + screensize_int; lp++) {
        *lp = BLACK;
    }
}

void updateScreen(int *fbp, int* screen, long int screensize_int) {
    long int index;
    for (index = 0; index < screensize_int; index++) {
        fbp[index] = screen[index];
    }
}

//Returns the pixel at a specific col, row of a Sprite
int getPixelAt(int col, int row, Sprite* sp) {
    
    return sp->pixel_arr[row*sp->width + col];

}

void setPixelAt(int col, int row, Sprite* sp, int color) {

    sp->pixel_arr[row*sp->width + col] = color;
}

void placeSprite(int* screen_pointer, Sprite* sp) {
    
    size_t row, col;
    size_t yStart = sp->y_pos;
    size_t xStart = sp->x_pos;

    for (row = 0; row < sp->height; row++) {
        
        size_t yPos = yStart + row; 

        for (col = 0; col < sp->width; col++) {
            int pixel = getPixelAt(col,row,sp);
            size_t xPos = xStart + col;

            long int screenIndex = yPos*SCREEN_WIDTH + xPos;
            *(screen_pointer + screenIndex) = pixel;
        }
    }
}

void makeLevel(Sprite* sp, size_t xPos, size_t yPos) {
    
    sp->width = LEVEL_WIDTH;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*LEVEL_WIDTH*sizeof(int));

    size_t letter1_col = 0;
    size_t letter2_col = CHAR_WIDTH + 10;
    size_t letter3_col = letter2_col + CHAR_WIDTH + 10;
    size_t letter4_col = letter3_col + CHAR_WIDTH + 10;
    size_t letter5_col = letter4_col + CHAR_WIDTH + 10;

    size_t row, col,col_temp;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'L'
                if (col < CHAR_THICK || 
                    row > LEVEL_HEIGHT - CHAR_THICK) {
                    setPixelAt(col,row,sp,WHITE);
                }

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'E'
                if (col_temp < CHAR_THICK ||
                    row < CHAR_THICK ||
                    row > LEVEL_HEIGHT - CHAR_THICK ||
                    (row > LEVEL_HEIGHT/2 - CHAR_THICK /2 && row < LEVEL_HEIGHT/2 + CHAR_THICK/2)) {
                    setPixelAt(col,row,sp,WHITE);
                }
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'V'
               if ((col_temp >= row/4 && col_temp <=row/4 +CHAR_THICK) ||
                    (col_temp >= CHAR_WIDTH - row/4-CHAR_THICK && col_temp <= CHAR_WIDTH-row/4)) {
                setPixelAt(col,row,sp,WHITE);
               }

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                col_temp = col - letter4_col;
                if (col_temp < CHAR_THICK ||
                    row < CHAR_THICK ||
                    row > LEVEL_HEIGHT - CHAR_THICK ||
                    (row > LEVEL_HEIGHT/2 - CHAR_THICK /2 && row < LEVEL_HEIGHT/2 + CHAR_THICK/2)) {
                    setPixelAt(col,row,sp,WHITE);
                }
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                col_temp = col - letter5_col;
                if (col_temp < CHAR_THICK || 
                    row > LEVEL_HEIGHT - CHAR_THICK) {
                    setPixelAt(col,row,sp,WHITE);
                }
            }
        }
    }
    
}
/*
void makePress(Sprite* sp, size_t xPos, size_t yPos) {
    
    sp->width = LEVEL_WIDTH;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*LEVEL_WIDTH*sizeof(int));

    size_t letter1_col = 0;
    size_t letter2_col = CHAR_WIDTH + 10;
    size_t letter3_col = letter2_col + CHAR_WIDTH + 10;
    size_t letter4_col = letter3_col + CHAR_WIDTH + 10;
    size_t letter5_col = letter4_col + CHAR_WIDTH + 10;

    size_t row, col,col_temp;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'P'
                if (col < CHAR_THICK || 
                    row < CHAR_THICK || 
                    (col > sp->width - CHAR_THICK && row < sp->height/2) ||
                    (row > sp->height/2 - CHAR_THICK && row < sp->height/2)) {
                    setPixelAt(col,row,sp,WHITE);
                }

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'R'
                if (col_temp < CHAR_THICK || 
                    row < CHAR_THICK || 
                    (col_temp > sp->width - CHAR_THICK && row < sp->height/2) ||
                    (row > sp->height/2 - CHAR_THICK && row < sp->height/2)||
                    (row > sp->height/2 && col_temp >= row/2 && col_temp <= row/2 + CHAR_THICK) {
                    setPixelAt(col,row,sp,WHITE);
                }
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'E'
               if (col_temp < CHAR_THICK ||
                    row < CHAR_THICK ||
                    row > LEVEL_HEIGHT - CHAR_THICK ||
                    (row > LEVEL_HEIGHT/2 - CHAR_THICK /2 && row < LEVEL_HEIGHT/2 + CHAR_THICK/2)) {
                    setPixelAt(col,row,sp,WHITE);
                }

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                //Draw 'S'
                col_temp = col - letter4_col;
                if (row < CHAR_THICK ||
                    (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2) ||
                    (row > sp->height - CHAR_THICK)) {
                    setPixelAt(col,row,sp,WHITE);
                }
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                col_temp = col - letter5_col;
                if (col_temp < CHAR_THICK || 
                    row > LEVEL_HEIGHT - CHAR_THICK) {
                    setPixelAt(col,row,sp,WHITE);
                }
            }
        }
    }
    
}

*/
//Modifies sp to be a life bar of the correct width
void makeLifeBar(Sprite* sp, int lifepoints, size_t x_pos, size_t y_pos) {  
    sp->width = LIFE_BAR_MAX_WIDTH;
    sp->height = LIFE_BAR_HEIGHT;
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->pixel_arr = (int*) malloc(LIFE_BAR_MAX_WIDTH*LIFE_BAR_HEIGHT*sizeof(int));
    size_t max_col = lifepoints*LIFE_BAR_MAX_WIDTH/LIFE_POINTS; 
    size_t row,col;
    for (row = 0; row < LIFE_BAR_HEIGHT; row++) {
        for (col = 0; col < LIFE_BAR_MAX_WIDTH; col++) {
            if (row < 2 || col < 2 || row > LIFE_BAR_HEIGHT - 3 || col > LIFE_BAR_MAX_WIDTH - 3) {
                setPixelAt(col, row, sp, WHITE); 
            } else if (col < max_col) {
                setPixelAt(col,row, sp, RED);
            } else {
                setPixelAt(col,row,sp,BLACK);
            }
         }
    }
    
}


void makeRightArrow(Sprite* sp, size_t x_pos, size_t y_pos, int arrow_col){
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->width = ARROW_HEIGHT_WIDTH;
    sp->height = ARROW_HEIGHT_WIDTH;
    sp->pixel_arr = (int*) malloc(ARROW_HEIGHT_WIDTH*ARROW_HEIGHT_WIDTH*sizeof(int));
    
    size_t row,col;
    int color;

    for (row = 0; row < ARROW_HEIGHT_WIDTH; row++) {
        for (col = 0; col < ARROW_HEIGHT_WIDTH; col++) {

           if ((row < ARROW_HEIGHT_WIDTH/4 && col < ARROW_HEIGHT_WIDTH/2) ||
               (row > 3*ARROW_HEIGHT_WIDTH/4 && col <ARROW_HEIGHT_WIDTH/2) ||
               ((row < (col-ARROW_HEIGHT_WIDTH/2) || row > (3*ARROW_HEIGHT_WIDTH/2 - col)) && col > 50)) {
                
               color = BLACK;

           } 
             else {
               color = arrow_col; 
            }
              setPixelAt(col,row,sp,color);
         }
    }

}

void makeLeftArrow(Sprite* sp, size_t x_pos, size_t y_pos, int arrow_col){
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->width = ARROW_HEIGHT_WIDTH;
    sp->height = ARROW_HEIGHT_WIDTH;
    sp->pixel_arr = (int*) malloc(ARROW_HEIGHT_WIDTH*ARROW_HEIGHT_WIDTH*sizeof(int));
    
    size_t row,col;
    int color;

    for (row = 0; row < ARROW_HEIGHT_WIDTH; row++) {
        for (col = 0; col < ARROW_HEIGHT_WIDTH; col++) {

           if ((row < ARROW_HEIGHT_WIDTH/4 && col > ARROW_HEIGHT_WIDTH/2) ||
               (row > 3*ARROW_HEIGHT_WIDTH/4 && col > ARROW_HEIGHT_WIDTH/2) ||
               ((row < ARROW_HEIGHT_WIDTH/2 - col || row > ARROW_HEIGHT_WIDTH/2 + col ) && col < 50)) {
                
               color = BLACK;

           } 
             else {
               color = arrow_col; 
            }
              setPixelAt(col,row,sp,color);
         }
    }

}

void makeUpArrow(Sprite* sp, size_t x_pos, size_t y_pos, int arrow_col){
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->width = ARROW_HEIGHT_WIDTH;
    sp->height = ARROW_HEIGHT_WIDTH;
    sp->pixel_arr = (int*) malloc(ARROW_HEIGHT_WIDTH*ARROW_HEIGHT_WIDTH*sizeof(int));
    
    size_t row,col;
    int color;

    for (row = 0; row < ARROW_HEIGHT_WIDTH; row++) {
        for (col = 0; col < ARROW_HEIGHT_WIDTH; col++) {

           if ((col < ARROW_HEIGHT_WIDTH/4 && row > ARROW_HEIGHT_WIDTH/2) ||
               (col > 3*ARROW_HEIGHT_WIDTH/4 && row > ARROW_HEIGHT_WIDTH/2) ||
               ((col < ARROW_HEIGHT_WIDTH/2 - row || col > ARROW_HEIGHT_WIDTH/2 + row ) && row < 50)) {
                
               color = BLACK;

           } 
             else {
               color = arrow_col; 
            }
             setPixelAt(col,row,sp,color);
         }
    }

}

void makeDownArrow(Sprite* sp, size_t x_pos, size_t y_pos, int arrow_col){
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->width = ARROW_HEIGHT_WIDTH;
    sp->height = ARROW_HEIGHT_WIDTH;
    sp->pixel_arr = (int*) malloc(ARROW_HEIGHT_WIDTH*ARROW_HEIGHT_WIDTH*sizeof(int));
    
    size_t row,col;
    int color;

    for (row = 0; row < ARROW_HEIGHT_WIDTH; row++) {
        for (col = 0; col < ARROW_HEIGHT_WIDTH; col++) {

           if ((col < ARROW_HEIGHT_WIDTH/4 && row < ARROW_HEIGHT_WIDTH/2) ||
               (col > 3*ARROW_HEIGHT_WIDTH/4 && row <ARROW_HEIGHT_WIDTH/2) ||
               ((col < (row-ARROW_HEIGHT_WIDTH/2) || col > (3*ARROW_HEIGHT_WIDTH/2 - row)) && row > 50)) {
                
               color = BLACK;

           } 
             else {
               color = arrow_col; 
            }
              setPixelAt(col,row,sp,color);
         }
    }

}

void makeTimerBar(Sprite* sp, size_t x_pos, size_t y_pos) {
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->height = TIME_HEIGHT;
    sp->width = TIME_WIDTH;
    sp->pixel_arr = (int *) malloc(TIME_HEIGHT*TIME_WIDTH*sizeof(int));
    
    size_t row, col;
    for (row = 0; row < TIME_HEIGHT; row++) {
        for (col = 0; col < TIME_WIDTH; col++) {
            
            int color = (unsigned int)WHITE - (unsigned int)(0xd400)*col/(TIME_WIDTH-1);
            setPixelAt(col,row,sp,color);
        }
     } 
}

void makeTimerMark(Sprite* sp, size_t x_pos, size_t y_pos) {
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->height = TIME_HEIGHT;
    sp->width = TIME_WIDTH/37;

    sp->pixel_arr = (int *) malloc(sp->height*sp->width*sizeof(int));

    size_t row, col;
    int color;

    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
           setPixelAt(col,row,sp,BLUE);
        }
     } 
}


void setTimerPos(Sprite* timer_mark, size_t timer_data, size_t timer_max, size_t timer_bar_x_pos) { 
    timer_mark->x_pos = ((timer_data*1.0)/timer_max)*TIME_WIDTH + timer_bar_x_pos;
}

void changeArrowColor(Sprite*sp, int color) {
    size_t row, col;
    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
            if (getPixelAt(col,row,sp) != BLACK) {
                setPixelAt(col,row,sp,color);
             }
        }
    }
}

//////////////////////
/// Draw Char /////////
//////////////////////


void makeOne(Sprite*sp) {
    size_t row, col;
    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
            if (col > sp->width - CHAR_THICK) {
                setPixelAt(col,row,sp,WHITE);
            } else {
                setPixelAt(col,row,sp,BACKGROUND);
            }
        }
    }
}

void makeTwo(Sprite* sp) {
    size_t row, col;

    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col ++) {
            if (row < CHAR_THICK || 
                (row <= sp->height/2 + CHAR_THICK/2 && row >= sp->height/2 - CHAR_THICK/2) ||
                row > sp->height - CHAR_THICK)
            {
                setPixelAt(col,row,sp,WHITE);

            } else if ((row < sp->height/2 - CHAR_THICK/2 && col > sp->width-CHAR_THICK) ||
                       (row > sp->height/2 + CHAR_THICK/2 && col < CHAR_THICK))
              {
                setPixelAt(col,row,sp,WHITE);
              } else {
                 setPixelAt(col,row,sp,BACKGROUND);
              }
        }
   }
}

void makeThree(Sprite* sp) {
    size_t row, col;

    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col ++) {
            if (row < CHAR_THICK || 
                (row <= sp->height/2 + CHAR_THICK/2 && row >= sp->height/2 - CHAR_THICK/2) ||
                row > sp->height - CHAR_THICK || 
                col > sp->width - CHAR_THICK)
            {
                setPixelAt(col,row,sp,WHITE);
            } else {
                setPixelAt(col,row,sp,BACKGROUND);
            }
         }
     }
}

void makeFour(Sprite* sp) {
     size_t row, col;

    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col ++) {
            if (col > sp->width - CHAR_THICK ||
                (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2) ||
                (col < CHAR_THICK && row < sp->height/2))
            {
                setPixelAt(col,row,sp,WHITE);
            } else {
                setPixelAt(col,row,sp,BACKGROUND);
             }
        }
    }
}

void makeFive(Sprite* sp) {
    size_t row, col;

    for (row=0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
            if (row < CHAR_THICK ||
                (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2) ||
                row > sp->height - CHAR_THICK ||
                (row < sp->height/2 && col < CHAR_THICK) ||
                (row > sp->height/2 && col > sp->width - CHAR_THICK)) {

                setPixelAt(col,row,sp,WHITE);
             } else {
                setPixelAt(col,row,sp,BACKGROUND);
               }
         }
     }
}

void makeSix(Sprite* sp) {
    size_t row, col;
    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
            if (col < CHAR_THICK ||
                row < CHAR_THICK ||
                (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2) ||
                row > sp->height - CHAR_THICK ||
                (row > sp->height/2 && col > sp->width - CHAR_THICK)) {

                setPixelAt(col,row,sp,WHITE);
             } else {
                setPixelAt(col,row,sp,BACKGROUND);
            }
        }
    }
}

void makeSeven(Sprite* sp) {
    size_t row,col;
    for (row=0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
        if (col > sp->width - CHAR_THICK ||
            row < CHAR_THICK) {

            setPixelAt(col,row,sp,WHITE);
        } else {
            setPixelAt(col,row, sp, BACKGROUND);
         }
       }
    }
}

void makeEight(Sprite* sp) {
    size_t row, col;

    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col++) {
            if (col < CHAR_THICK ||
                col > sp->width - CHAR_THICK ||
                row < CHAR_THICK ||
                row > sp->height - CHAR_THICK ||
                (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2)) {

                setPixelAt(col,row,sp,WHITE);
           } else {
                setPixelAt(col,row,sp,BACKGROUND);
          }
        }
    }
}

void makeNine(Sprite* sp) {
    size_t row, col;
    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col ++) {
            if (row < CHAR_THICK ||
                row > sp->height - CHAR_THICK ||
                (row > sp->height/2 - CHAR_THICK/2 && row < sp->height/2 + CHAR_THICK/2) ||
                (row < sp->height/2 && col < CHAR_THICK) ||
                col > sp->width - CHAR_THICK) 
            {
                setPixelAt(col,row,sp,WHITE);
            } else {
                setPixelAt(col,row,sp,BACKGROUND);
            }
        }
    }
}

void makeZero(Sprite* sp) {
    size_t row, col;
    for (row = 0; row < sp->height; row ++) {
        for (col = 0; col < sp->width; col++) {
            if (col < CHAR_THICK ||
                col > sp->width - CHAR_THICK ||
                row < CHAR_THICK ||
                row > sp->height - CHAR_THICK) {

                setPixelAt(col,row,sp,WHITE);
             } else {
                setPixelAt(col,row,sp,BACKGROUND);
             }
        }
    }
}


void makeDigit(Sprite*sp, size_t num, size_t x_pos, size_t y_pos) {
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->height = CHAR_HEIGHT;
    sp->width = CHAR_WIDTH;

    sp->pixel_arr = (int*) malloc(sp->height*sp->width*sizeof(int));

    switch(num) {
        case 1:
            makeOne(sp);
            break;
        case 2:
            makeTwo(sp);
            break;
        case 3:
            makeThree(sp);
            break;
        case 4:
            makeFour(sp);
            break;
        case 5:
            makeFive(sp);
            break;
        case 6:
            makeSix(sp);
            break;
        case 7:
            makeSeven(sp);
            break;
        case 8:
            makeEight(sp);
            break;
        case 9:
            makeNine(sp);
            break;
        case 0:
            makeZero(sp);
            break;
        default:
            break;
    }

}

void makeNum(Sprite* num_1, Sprite* num_0, size_t level, size_t x_pos, size_t y_pos) {
    if (level/10 != 0) {
        makeDigit(num_1,level/10,x_pos,y_pos);
    }

    makeDigit(num_0,level%10,x_pos+CHAR_WIDTH+10,y_pos);

}

void makeA(Sprite*sp) {
    size_t row,col,row_inv;
    for (row = 0; row < sp-> height; row++) {
        for (col = 0; col < sp->width; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            row_inv = sp->height - row - 1;
            if ((col >= row_inv/5 && col <=row_inv/5 + CHAR_THICK) ||
                (col >= CHAR_WIDTH - row_inv/5 - CHAR_THICK && col <= CHAR_WIDTH-row_inv/5) ||
                (row < sp->height/2 + CHAR_THICK && row > sp->height/2 && col > CHAR_THICK + row_inv/5 && col < CHAR_WIDTH-row_inv/5)) {
                setPixelAt(col,row,sp,WHITE);
            }
        }
    }

}

void makeC(Sprite* sp) {
   size_t row, col;
   for (row=0; row < sp->height; row++) {
    for (col = 0; col < sp->width; col++) {
        setPixelAt(col,row,sp,BACKGROUND);

        if (col < CHAR_THICK ||
            row < CHAR_THICK ||
            row > sp->height - CHAR_THICK) {
            setPixelAt(col,row,sp,WHITE);
        }
    }
  }
}

void makeE(Sprite* sp) {
    size_t row,col;
    for (row=0; row < sp->height;row++){
        for (col = 0; col < sp->width; col++) {
        setPixelAt(col,row,sp,BACKGROUND);

        if (col < CHAR_THICK ||
            row < CHAR_THICK ||
            row > sp->height - CHAR_THICK ||
            (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2)) {
            setPixelAt(col,row,sp,WHITE);
        } 
       }
    }
}

void makeL(Sprite* sp) {
   //TODO:Write this
}

void makeP(Sprite*sp) {
    size_t row,col;
    for (row = 0; row < sp->height; row++) {
        for (col = 0; col < sp->width; col ++) {
            if (col < CHAR_THICK ||
                row < CHAR_THICK ||
                (col > sp->width - CHAR_THICK && row < sp->height/2) ||
                (row > sp->height/2 - CHAR_THICK && row < sp->height/2)) {
                    setPixelAt(col,row,sp,WHITE);
            } else {
                setPixelAt(col,row,sp,BACKGROUND);
            }
         }
     }
}

void makeR(Sprite*sp) {
    //TODO:write this
}
void makeY(Sprite *sp) {
    //TODO: write this
}

void makeChar(Sprite* sp,char c, size_t x_pos,size_t y_pos) {
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->height = CHAR_HEIGHT;
    sp->width = CHAR_WIDTH;
    sp->pixel_arr = (int*) malloc(CHAR_HEIGHT*CHAR_WIDTH*sizeof(int));

    switch(c) {
        case 'A':
            makeA(sp);
            break;
        case 'C':
            makeC(sp);
            break;
        case 'E':
            makeE(sp);
            break;
        case 'L':
            makeL(sp);
            break;
        case 'P':
            makeP(sp);
            break;
        case 'R':
            makeR(sp);
            break;
        case 'S':
            makeFive(sp);
            break;
        case 'Y':
            makeY(sp);
            break;
        default:
            break;
    }


}

///////////////////////////////
//// Updating Screen //////////
///////////////////////////////
void makeStartScreen(GameScreen* g) {
    Sprite sp;
    makeRightArrow(&sp,SCREEN_WIDTH/2-200,SCREEN_HEIGHT/2-120,BLUE);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeDigit(&sp,5,SCREEN_WIDTH/2-200 + ARROW_HEIGHT_WIDTH + 10, SCREEN_HEIGHT/2-110);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeChar(&sp,'P',SCREEN_WIDTH/2-80 + CHAR_WIDTH,SCREEN_HEIGHT/2-110);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeChar(&sp,'A',SCREEN_WIDTH/2-75 + 2*CHAR_WIDTH, SCREEN_HEIGHT/2-110);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeChar(&sp,'C',SCREEN_WIDTH/2-65 + 3*CHAR_WIDTH, SCREEN_HEIGHT/2-110);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeChar(&sp,'E',SCREEN_WIDTH/2-55 + 4*CHAR_WIDTH, SCREEN_HEIGHT/2-110);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeChar(&sp,'P',SCREEN_WIDTH/2, SCREEN_HEIGHT/2+60);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);


}

void makeDeathScreenOne(GameScreen * g) {
    

}

void addSpriteToGame(int sprite_name,GameScreen* g, int x_pos, int y_pos) {
    
    Sprite sp;
    Sprite* keys_1 = g->key_arr_1;
    Sprite* keys_2 = g->key_arr_2;

    size_t numKeys = g->size;

    switch(sprite_name) {
        case INVALID_SPRITE:
          break;
        case LIFE_BAR_1:
          makeLifeBar(&(g->life_bar_1),g->life_1,LIFE_BAR_X_POS,LIFE_BAR_Y_POS);
          break;
        case LIFE_BAR_2:
          makeLifeBar(&(g->life_bar_2),g->life_2, LIFE_BAR_X_POS + SCREEN_WIDTH/2,LIFE_BAR_Y_POS);
          break;
        case RIGHT_ARROW:
          makeRightArrow(&keys_1[numKeys],x_pos, y_pos,BLUE);
          makeRightArrow(&keys_2[numKeys],x_pos+1000, y_pos,BLUE);
          g->size++;
          break;
        case LEFT_ARROW:
          makeLeftArrow(&keys_1[numKeys],x_pos,y_pos,BLUE);
          makeLeftArrow(&keys_2[numKeys],x_pos+1000,y_pos,BLUE);
          g->size++;
          break;
        case DOWN_ARROW:
          makeDownArrow(&keys_1[numKeys],x_pos,y_pos,BLUE);
          makeDownArrow(&keys_2[numKeys],x_pos+1000,y_pos,BLUE);
          g->size++;
          break;
        case UP_ARROW:
          makeUpArrow(&keys_1[numKeys],x_pos,y_pos,BLUE);
          makeUpArrow(&keys_2[numKeys],x_pos+1000,y_pos,BLUE);
          g->size++;
          break;
        case TIMER_BAR_1:
          makeTimerBar(&(g->timer_bar_1),x_pos,y_pos);
          break;
        case TIMER_BAR_2:
          makeTimerBar(&(g->timer_bar_2),x_pos,y_pos);
          break;
        case TIMER_MARK_1:
          makeTimerMark(&(g->timer_mark_1),x_pos,y_pos);
          break;
        case TIMER_MARK_2:
          makeTimerMark(&(g->timer_mark_2),x_pos,y_pos);
          break;
        case LEVEL:
          makeNum(&(g->level_num_1),&(g->level_num_0),g->level,LEVEL_X_POS,LEVEL_Y_POS);
          makeLevel(&(g->level_sprite),LEVEL_X_POS-LEVEL_WIDTH-15,LEVEL_Y_POS);
          break;
    }
}

void initializeLevel(GameScreen * g) {

    addSpriteToGame(LIFE_BAR_1,g,0,0); 
    addSpriteToGame(LEVEL,g,0,0);
    addSpriteToGame(TIMER_BAR_1,g,TIME_1_X_POS,TIME_Y_POS);
    addSpriteToGame(TIMER_MARK_1,g,TIME_1_X_POS,TIME_Y_POS);
}

void initializeLevelTwo(GameScreen* g) {
    initializeLevel(g);
    addSpriteToGame(LIFE_BAR_2,g,0,0);
    addSpriteToGame(TIMER_BAR_2,g,TIME_2_X_POS,TIME_Y_POS);
    addSpriteToGame(TIMER_MARK_2,g,TIME_2_X_POS,TIME_Y_POS);
}

void updateGameScreenSinglePlayer(GameScreen * g) 
{
    addSpriteToGame(LIFE_BAR_1,g,0,0);

    placeSprite(g->pixel_arr,&(g->life_bar_1));
    placeSprite(g->pixel_arr,&(g->timer_bar_1));
    placeSprite(g->pixel_arr,&(g->timer_mark_1));
    if (g->level > 9) {
        placeSprite(g->pixel_arr,&(g->level_num_1));
    }
    placeSprite(g->pixel_arr,&(g->level_num_0));
    placeSprite(g->pixel_arr,&(g->level_sprite));
    size_t i;
    for (i = 0; i < g->size; i++) {
        placeSprite(g->pixel_arr,&(g->key_arr_1[i]));
    }
    

}

void updateGameScreenTwoPlayer(GameScreen* g)
{
    Sprite sp;
    sp.pixel_arr = g->pixel_arr;
    sp.height = SCREEN_HEIGHT;
    sp.width = SCREEN_WIDTH;
    size_t i;
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        setPixelAt(SCREEN_WIDTH/2,i,&sp,WHITE);    
    }
    updateGameScreenSinglePlayer(g);
    addSpriteToGame(LIFE_BAR_2,g,0,0);
    placeSprite(g->pixel_arr,&(g->life_bar_2));
    placeSprite(g->pixel_arr,&(g->timer_bar_2));
    placeSprite(g->pixel_arr,&(g->timer_mark_2));
    for (i = 0; i < g->size; i++) {
        placeSprite(g->pixel_arr,&(g->key_arr_2[i]));
    }
    
}
void resetKeys(GameScreen *g, int player) {
    size_t i;
    if (player==1) {
        for (i = 0; i < g->arrow_index_1; i++) {
            changeArrowColor(&(g->key_arr_1[i]),BLUE);
        }
        g->arrow_index_1 = 0;
    } else {
        for (i = 0; i < g->arrow_index_2; i++) {
            changeArrowColor(&(g->key_arr_2[i]),BLUE);
        }
        g->arrow_index_2 = 0;
   }
}

void clearSprites(GameScreen *g) {
    free(g->timer_mark_1.pixel_arr);
    free(g->timer_bar_1.pixel_arr);
    free(g->life_bar_1.pixel_arr);
    if (g->level > 9) {
        free(g->level_num_1.pixel_arr);
    }
    free(g->level_num_0.pixel_arr);
    free(g->level_sprite.pixel_arr);

    size_t i;
    for (i = 0; i < g->size; i++) {
        free(g->key_arr_1[i].pixel_arr);
        free(g->key_arr_2[i].pixel_arr);
    }

    initializeScreen(g);
}

void clearSpritesTwo(GameScreen * g) {
    clearSprites(g);

    free(g->timer_mark_2.pixel_arr);
    free(g->timer_bar_2.pixel_arr);
    free(g->life_bar_2.pixel_arr);
}

//////////////////////////////////////
//// Setup and Teardown Framebuffer///
//////////////////////////////////////

int setUpFrameBuffer(int **fbp, long int *screensize_in_int, int* fbfd) {
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screensize = 0;

  // Open the file for reading and writing
  *fbfd = open("/dev/fb0", O_RDWR);
  if (!*fbfd) {
    printf("Error: cannot open framebuffer device.\n");
    return(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(*fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
  }

  // Get variable screen information
  if (ioctl(*fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
  }
  printf("%dx%d, %d bpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );
  // map framebuffer to user memory 
  screensize = finfo.smem_len;
  *screensize_in_int = screensize/4;

  *fbp = (int*)mmap(0, 
                    screensize, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    *fbfd, 0);

  if ((int)*fbp == -1) {
    printf("Failed to mmap.\n");
    return(1);
  } else {
    return 0;
  }
}

void tearDownFrameBuffer(int* fbp, int fbfd,long int screensize_in_int) {

  munmap(fbp, screensize_in_int*4);
  close(fbfd);

}
