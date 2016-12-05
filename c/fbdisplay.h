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
    int num_keys;
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
    
    Sprite star;
};

typedef struct GameScreen GameScreen;


void initializeScreen(GameScreen *g) {
    g->size = 0;
    g->arrow_index_1 = 0;
    g->arrow_index_2 = 0;
    g->winner = 0;

    g->num_keys = 4 + g->level/4;    
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
            if (col > CHAR_WIDTH/2 - CHAR_THICK/2 && col < CHAR_WIDTH/2 + CHAR_THICK/2) {
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

            } else if ((row < sp->height/2 - CHAR_THICK/2 && col > CHAR_WIDTH-CHAR_THICK) ||
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

void makeColon(Sprite * sp, size_t x_pos, size_t y_pos) {
    sp->width = CHAR_THICK;
    sp->height = CHAR_HEIGHT;
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->pixel_arr = (int*) malloc(CHAR_THICK*CHAR_HEIGHT*sizeof(int));
    
    size_t row,col;
    for (row = 0; row < CHAR_HEIGHT; row++) {
        for (col = 0; col < CHAR_THICK; col++) {
           setPixelAt(col,row,sp,BACKGROUND);
           if (row < CHAR_THICK && row > CHAR_HEIGHT - CHAR_THICK) {
             setPixelAt(col,row,sp,WHITE);
           }
         }
    }}

void makeHyphen(Sprite* sp, size_t x_pos, size_t y_pos) {
    sp->width = CHAR_WIDTH;
    sp->height = CHAR_HEIGHT;
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->pixel_arr = (int*) malloc(CHAR_WIDTH*CHAR_HEIGHT*sizeof(int));
    
    size_t row,col;
    for (row = 0; row < CHAR_HEIGHT; row++) {
        for (col = 0; col < CHAR_WIDTH; col++) {
           setPixelAt(col,row,sp,BACKGROUND);
           if (row > CHAR_HEIGHT/2 - CHAR_THICK/2 && row < CHAR_HEIGHT/2 + CHAR_THICK && col < CHAR_HEIGHT-CHAR_THICK/2 && col > CHAR_THICK/2) {
             setPixelAt(col,row,sp,WHITE);
           }
         }
    }}

void makeStar(Sprite* sp, size_t x_pos, size_t y_pos) {
    sp->width = CHAR_HEIGHT/2;
    sp->height = CHAR_HEIGHT/2;
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->pixel_arr = (int*) malloc((sp->width)*(sp->height)*sizeof(int));

    size_t row, col;
    for (row = 0; row < sp->height; row++) {
        for (col=0; col < sp->width; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if ( ((row > sp->height/2 - CHAR_THICK/4 && row < sp->height/2 + CHAR_THICK/4) || ( col > sp->width/2 - CHAR_THICK/4 && col < sp->width/2 + CHAR_THICK/4)) ||
               ((col >= row && col <= row + CHAR_THICK/4) || (col >= sp->width - row-CHAR_THICK/4 && col <= sp->width - row)) ) {
                setPixelAt(col,row,sp,WHITE);
            }
        }
    }
}

void makeA(Sprite*sp, size_t row, size_t col, size_t col_temp, int color) {
     size_t row_inv = sp->height - row - 1;
     if ((col_temp >= row_inv/5 && col_temp <=row_inv/5 + CHAR_THICK) ||
       (col_temp >= CHAR_WIDTH - row_inv/5 - CHAR_THICK && col_temp <= CHAR_WIDTH-row_inv/5) ||
       (row < sp->height/2 + CHAR_THICK && row > sp->height/2 && col_temp > CHAR_THICK + row_inv/5 && col_temp < CHAR_WIDTH-row_inv/5)) {
           setPixelAt(col,row,sp,color);
    }

}

void makeB(Sprite* sp,size_t row, size_t col, size_t col_temp, int color) {
    if (col_temp < CHAR_THICK ||
      (row < CHAR_THICK && col < CHAR_WIDTH - CHAR_THICK/2) ||
      (row > sp->height - CHAR_THICK && col < CHAR_WIDTH - CHAR_THICK/2) || 
      (row < sp->height/2+CHAR_THICK/2 && row>sp->height/2 -CHAR_THICK/2 && col_temp < CHAR_WIDTH-CHAR_THICK/2) ||
      (col > CHAR_WIDTH - CHAR_THICK && ((row < CHAR_HEIGHT-CHAR_THICK/2 && row >=CHAR_HEIGHT/2+CHAR_THICK/2) ||(row > CHAR_THICK/2 && row <= CHAR_HEIGHT/2-CHAR_THICK/2)))) {
        setPixelAt(col,row,sp,color);
    }
}
void makeC(Sprite* sp,size_t row, size_t col, size_t col_temp, int color) {
    if (col_temp < CHAR_THICK ||
      row < CHAR_THICK ||
      row > sp->height - CHAR_THICK) {
        setPixelAt(col,row,sp,color);
    }
}

void makeE(Sprite* sp, size_t row, size_t col, size_t col_temp, int color) {
    if (col_temp < CHAR_THICK ||
        row < CHAR_THICK ||
        row > sp->height - CHAR_THICK ||
        (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2)) {
            setPixelAt(col,row,sp,color);
    }
}

void makeI(Sprite* sp, size_t row, size_t col, size_t col_temp, int color) {
    if ((col_temp < CHAR_WIDTH/2 + CHAR_THICK/2 && col_temp > CHAR_WIDTH/2 - CHAR_THICK/2) || 
        row < CHAR_THICK ||
        row > CHAR_HEIGHT - CHAR_THICK) {
        setPixelAt(col,row,sp,color);
   }
}

void makeL(Sprite* sp, size_t row, size_t col, size_t col_temp, int color) {
   if (col_temp < CHAR_THICK || 
       row > LEVEL_HEIGHT - CHAR_THICK) {
        setPixelAt(col,row,sp,color);
   }
}

void makeN(Sprite* sp, size_t row, size_t col, size_t col_temp, int color) {
    if(col_temp < CHAR_THICK || col_temp > CHAR_WIDTH - CHAR_THICK ||
       (col_temp >= row/2 && col_temp <= row/2 + CHAR_THICK)) {
       setPixelAt(col,row,sp,color);
    }
}

void makeO(Sprite* sp, size_t row, size_t col, size_t col_temp, int color) {
    if (col_temp < CHAR_THICK || row < CHAR_THICK ||
       col_temp > CHAR_WIDTH - CHAR_THICK ||
       row > CHAR_HEIGHT - CHAR_THICK) {

        setPixelAt(col,row,sp,color);
    }
}

void makeP(Sprite*sp, size_t row, size_t col, size_t col_temp, int color) {
    if (col_temp < CHAR_THICK ||
        row < CHAR_THICK ||
        (col_temp > CHAR_WIDTH - CHAR_THICK && row < sp->height/2) ||
        (row > sp->height/2 - CHAR_THICK && row < sp->height/2)) {
            setPixelAt(col,row,sp, color);
     }
}

void makeR(Sprite*sp, size_t row, size_t col, size_t col_temp, int color) {
    if (col_temp < CHAR_THICK || 
        row < CHAR_THICK || 
        (col_temp > CHAR_WIDTH - CHAR_THICK && row < sp->height/2) ||
        (row > sp->height/2 - CHAR_THICK && row < sp->height/2)||
        (row >= sp->height/2 && col_temp >= row/2 && col_temp <= row/2 + CHAR_THICK)) {
        setPixelAt(col,row,sp,color);
    }
}

void makeS(Sprite*sp, size_t row, size_t col, size_t col_temp, int color) {
    if (row < CHAR_THICK ||
       (row < sp->height/2 + CHAR_THICK/2 && row > sp->height/2 - CHAR_THICK/2) ||
       row > sp->height - CHAR_THICK ||
       (row < sp->height/2 && col_temp < CHAR_THICK) ||
       (row > sp->height/2 && col_temp > CHAR_WIDTH - CHAR_THICK)) {

      setPixelAt(col,row,sp,color);
    }

}

void makeT(Sprite *sp,size_t row, size_t col, size_t col_temp, int color) {
    if (row < CHAR_THICK || 
        (col_temp > CHAR_WIDTH/2 - CHAR_THICK/2 && col_temp < CHAR_WIDTH/2 + CHAR_THICK/2)) {
           setPixelAt(col,row,sp,color);
   }
}

void makeU(Sprite*sp,size_t row, size_t col, size_t col_temp, int color) {
   if (col_temp < CHAR_THICK || col_temp > CHAR_WIDTH - CHAR_THICK ||
      row > CHAR_HEIGHT - CHAR_THICK) {
        setPixelAt(col,row,sp,color);
   }
}

void makeV(Sprite*sp, size_t row, size_t col, size_t col_temp, int color) {
    if ((col_temp >= row/4 && col_temp <=row/4 +CHAR_THICK) ||
                    (col_temp >= CHAR_WIDTH - row/4-CHAR_THICK && col_temp <= CHAR_WIDTH-row/4)) {
                setPixelAt(col,row,sp,color);
    }
}

void makeW(Sprite*sp, size_t row, size_t col, size_t col_temp, int color) {
    if ((col_temp >= row/16 && col_temp <=row/16 +CHAR_THICK) ||
         (col_temp >= CHAR_WIDTH - row/16-CHAR_THICK && col_temp <= CHAR_WIDTH-row/16)) {
                setPixelAt(col,row,sp,color);
    }
}
void makeY(Sprite *sp,size_t row, size_t col, size_t col_temp, int color) {
   if ((col_temp >= row/2 && col_temp <=row/2 + CHAR_THICK & row < CHAR_HEIGHT/2) || 
       (col_temp >= CHAR_WIDTH-row/2-CHAR_THICK && col_temp <=CHAR_WIDTH-row/2 & row < CHAR_HEIGHT/2) ||
       (col_temp >= CHAR_WIDTH/2 - CHAR_THICK/2 && col_temp <= CHAR_WIDTH/2 + CHAR_THICK/2 && row >= CHAR_HEIGHT/2)) {
       setPixelAt(col,row,sp,color);
   }
}

void makeChar(Sprite* sp,char c,size_t row, size_t col,size_t col_temp, int color) {

    switch(c) {
        case 'A':
            makeA(sp,row,col,col_temp, color);
            break;
        case 'B':
            makeB(sp,row,col,col_temp,color);
            break;
        case 'C':
            makeC(sp,row,col,col_temp, color);
            break;
        case 'E':
            makeE(sp,row,col,col_temp, color);
            break;
        case 'I':
            makeI(sp,row,col,col_temp,color);
            break;
        case 'L':
            makeL(sp,row,col,col_temp, color);
            break;
        case 'N':
            makeN(sp,row,col,col_temp,color);
            break;
        case 'O':
            makeO(sp,row,col,col_temp,color);
            break;
        case 'P':
            makeP(sp,row,col,col_temp, color);
            break;
        case 'R':
            makeR(sp,row,col,col_temp, color);
            break;
        case 'S':
            makeS(sp,row,col,col_temp, color);
            break;
        case 'T':
            makeT(sp,row,col,col_temp,color);
            break;
        case 'U':
            makeU(sp,row,col,col_temp,color);
            break;
        case 'V':
            makeV(sp,row,col,col_temp,color);
            break;
        case 'W':
            makeW(sp,row,col,col_temp,color); 
            break;
        case 'Y':
            makeY(sp,row,col,col_temp, color);
            break;
        default:
            break;
    }


}

///////////////////////////////
///// Making Words ////////////
///////////////////////////////
void makeSpace(Sprite*sp, size_t xPos, size_t yPos) {
    
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
               //Draw 'S'
               makeChar(sp,'S',row,col,col,WHITE); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'P'
                makeChar(sp,'P',row,col,col_temp,WHITE); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'A'
               makeChar(sp,'A',row,col,col_temp,WHITE); 

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                col_temp = col - letter4_col;
                //Draw 'C'
                makeChar(sp,'C',row,col,col_temp,WHITE);
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                col_temp = col - letter5_col;
                makeChar(sp,'E',row,col,col_temp,WHITE); 
            }
        }
    }

}

void makeASprite(Sprite*sp,size_t xPos, size_t yPos) {
    sp->width = CHAR_WIDTH;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*sp->width*sizeof(int));
    
    size_t row, col;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < sp->width; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            makeChar(sp,'A',row,col,col,RED); 
        }
    }
}

void makeBSprite(Sprite*sp,size_t xPos, size_t yPos) {
    sp->width = CHAR_WIDTH;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*sp->width*sizeof(int));
    
    size_t row, col;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < sp->width; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            makeChar(sp,'B',row,col,col,RED); 
        }
    }
}
void makePlayer(Sprite*sp, size_t xPos, size_t yPos) {
    
    sp->width = LEVEL_WIDTH + CHAR_WIDTH + 10;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*sp->width*sizeof(int));
    
    size_t letter1_col = 0;
    size_t letter2_col = CHAR_WIDTH + 10;
    size_t letter3_col = letter2_col + CHAR_WIDTH + 10;
    size_t letter4_col = letter3_col + CHAR_WIDTH + 10;
    size_t letter5_col = letter4_col + CHAR_WIDTH + 10;
    size_t letter6_col = letter5_col + CHAR_WIDTH + 10;

    size_t row, col,col_temp;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < sp->width; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'S'
               makeChar(sp,'P',row,col,col,WHITE); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'P'
                makeChar(sp,'L',row,col,col_temp,WHITE); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'A'
               makeChar(sp,'A',row,col,col_temp,WHITE); 

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                col_temp = col - letter4_col;
                //Draw 'C'
                makeChar(sp,'Y',row,col,col_temp,WHITE);
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                col_temp = col - letter5_col;
                makeChar(sp,'E',row,col,col_temp,WHITE); 
            } else if (col < letter6_col+CHAR_WIDTH && col > letter6_col) {
                col_temp = col - letter6_col;
                makeChar(sp,'R',row,col,col_temp,WHITE);
            }
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
                
                makeChar(sp,'L',row,col,col,WHITE);
            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'E'
                makeChar(sp,'E',row,col,col_temp,WHITE);
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'V'
               makeChar(sp,'V',row, col,col_temp,WHITE);

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                col_temp = col - letter4_col;
                makeChar(sp,'E',row,col,col_temp,WHITE); 
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                col_temp = col - letter5_col;
                makeChar(sp,'L',row,col,col_temp,WHITE); 
            }
        }
    }
    
}

void makeScore(Sprite* sp, size_t xPos, size_t yPos) {
    
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
            
                makeChar(sp,'S',row,col,col,WHITE);

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
               
                col_temp = col - letter2_col;
                makeChar(sp,'C',row,col,col_temp,WHITE); 

            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               
               col_temp = col - letter3_col;
               makeChar(sp,'O',row,col,col_temp,WHITE); 

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                
                col_temp = col - letter4_col;
                makeChar(sp,'R',row,col,col_temp,WHITE);

            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                
                col_temp = col - letter5_col;
                makeChar(sp,'E',row,col,col_temp,WHITE);
 
            }
        }
    }
    

}
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
                makeChar(sp,'P',row,col,col,WHITE);


            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'R'
                makeChar(sp,'R',row,col,col_temp,WHITE); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'E'
               makeChar(sp,'E',row,col,col_temp,WHITE); 

            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                //Draw 'S'
                col_temp = col - letter4_col;
                makeChar(sp,'S',row,col,col_temp,WHITE);

            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                col_temp = col - letter5_col;
                makeChar(sp,'S',row,col,col_temp,WHITE);
 
            }
        }
    }
    
}


void makeStart(Sprite* sp, size_t xPos, size_t yPos, int color) {
    
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

    size_t row, col,col_temp,row_inv;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'S'
               makeChar(sp,'S',row,col,col,color); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'T'
                makeChar(sp,'T',row,col,col_temp,color); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'A'
               makeChar(sp,'A',row,col,col_temp,color);
            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                //Draw 'R'
                col_temp = col - letter4_col;
                makeChar(sp,'R',row,col,col_temp,color); 
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                //Draw 'T'
                col_temp = col - letter5_col;
                makeChar(sp,'T',row,col,col_temp,color);
            }
        }
    }
    
}

void makeToContinue(Sprite* sp, size_t xPos, size_t yPos) {
    sp->width = LEVEL_WIDTH*2 + 15;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*sp->width*sizeof(int));
    
    size_t letter1_col = 0;
    size_t letter2_col = CHAR_WIDTH + 10;
    size_t letter3_col = letter2_col + CHAR_WIDTH + 25;
    size_t letter4_col = letter3_col + CHAR_WIDTH + 10;
    size_t letter5_col = letter4_col + CHAR_WIDTH + 10;
    size_t letter6_col = letter5_col + CHAR_WIDTH + 10;
    size_t letter7_col = letter6_col + CHAR_WIDTH + 10;
    size_t letter8_col = letter7_col + CHAR_WIDTH + 10;
    size_t letter9_col = letter8_col + CHAR_WIDTH + 10;
    size_t letter10_col = letter9_col + CHAR_WIDTH + 10;
    
    size_t row, col,col_temp;

    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < sp->width; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'T'
               makeChar(sp,'T',row,col,col,WHITE); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'O'
               makeChar(sp,'O',row,col,col_temp,WHITE);                
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'C'
               makeChar(sp,'C',row,col,col_temp,WHITE); 
            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                //Draw 'O'
               col_temp = col - letter4_col;
               makeChar(sp,'O',row,col,col_temp,WHITE); 
            } else if (col < letter5_col+CHAR_WIDTH && col > letter5_col) {
                //Draw 'N'
               col_temp = col - letter5_col;
               makeChar(sp,'N',row,col,col_temp,WHITE); 
            } else if (col < letter6_col + CHAR_WIDTH && col > letter6_col) {
                col_temp = col - letter6_col;
                //Draw 'T'
                makeChar(sp,'T',row,col,col_temp,WHITE);
           } else if (col < letter7_col+CHAR_WIDTH && col > letter7_col) {
               col_temp = col - letter7_col;
               //Draw 'I'
               makeChar(sp,'I',row,col,col_temp,WHITE); 
            } else if (col < letter8_col+CHAR_WIDTH && col > letter8_col) {
                //Draw 'N'
                col_temp = col - letter8_col;
                makeChar(sp,'N',row,col,col_temp,WHITE);
            } else if (col < letter9_col+CHAR_WIDTH && col > letter9_col) {
                //Draw 'U'
                col_temp = col-letter9_col;
                makeChar(sp,'U',row,col,col_temp,WHITE);
            } else if (col < letter10_col+CHAR_WIDTH && col > letter10_col) {
                //Draw 'E'
                col_temp = col-letter10_col;
                makeChar(sp,'E',row,col,col_temp,WHITE);
            }
        }
    }
}

void makeYou(Sprite* sp, size_t xPos, size_t yPos, int color) {
    
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

    size_t row, col,col_temp,row_inv;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               makeChar(sp,'Y',row,col,col,color); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                makeChar(sp,'O',row,col,col_temp,color); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               makeChar(sp,'U',row,col,col_temp,color);
            }         
      }
    }
    
}


void makeWin(Sprite* sp, size_t xPos, size_t yPos, int color) {
    
    sp->width = LEVEL_WIDTH;
    sp->height = LEVEL_HEIGHT;
    sp->x_pos = xPos;
    sp->y_pos = yPos;
    sp->pixel_arr = (int*) malloc(LEVEL_HEIGHT*LEVEL_WIDTH*sizeof(int));

    size_t letter1_col = 0;
    size_t letter2_col = CHAR_WIDTH-CHAR_THICK;
    size_t letter3_col = letter2_col + CHAR_WIDTH + 10;
    size_t letter4_col = letter3_col + CHAR_WIDTH + 10;
    size_t letter5_col = letter4_col + CHAR_WIDTH + 10;

    size_t row, col,col_temp,row_inv;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               makeChar(sp,'V',row,col,col,color); 

            } if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                makeChar(sp,'V',row,col,col_temp,color); 
            }
            else if (col < letter3_col + CHAR_WIDTH && col > letter3_col) {
                col_temp = col - letter3_col;
                makeChar(sp,'I',row,col,col_temp,color); 
            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
               col_temp = col - letter4_col;
               makeChar(sp,'N',row,col,col_temp,color);
            }         
      }
    }
    
}

void makeLose(Sprite* sp, size_t xPos, size_t yPos, int color) {
    
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

    size_t row, col,col_temp,row_inv;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'S'
               makeChar(sp,'L',row,col,col,color); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'T'
                makeChar(sp,'O',row,col,col_temp,color); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'A'
               makeChar(sp,'S',row,col,col_temp,color);
            } else if (col < letter4_col+CHAR_WIDTH && col > letter4_col) {
                //Draw 'R'
                col_temp = col - letter4_col;
                makeChar(sp,'E',row,col,col_temp,color); 
            }         }
    }
    
}

void makeTie(Sprite* sp, size_t xPos, size_t yPos, int color) {
    
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

    size_t row, col,col_temp,row_inv;
    for (row = 0; row < LEVEL_HEIGHT; row ++) {
        for (col = 0; col < LEVEL_WIDTH; col++) {
            setPixelAt(col,row,sp,BACKGROUND);
            if (col < CHAR_WIDTH) {
               //Draw 'S'
               makeChar(sp,'T',row,col,col,color); 

            } else if (col < letter2_col + CHAR_WIDTH && col > letter2_col) {
                col_temp = col - letter2_col;
                //Draw 'T'
                makeChar(sp,'I',row,col,col_temp,color); 
            } else if (col < letter3_col+CHAR_WIDTH && col > letter3_col) {
               col_temp = col - letter3_col;
               //Draw 'A'
               makeChar(sp,'E',row,col,col_temp,color);
            }      
         }
    }
    
}
///////////////////////////////
//// Updating Screen //////////
///////////////////////////////
void makeStartScreen(GameScreen* g, int color) {
    size_t title_xPos, title_yPos, pick_xPos, pick1_yPos, pick2_yPos;

    title_xPos = SCREEN_WIDTH/2-200;
    title_yPos = SCREEN_HEIGHT/2 - 100;
    pick_xPos = SCREEN_WIDTH/2-250;
    pick1_yPos = SCREEN_HEIGHT/2 + 150;
    pick2_yPos = pick1_yPos + 50 + CHAR_HEIGHT + 25;

    Sprite sp;
    makeRightArrow(&sp,title_xPos,title_yPos,color);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);
    
    makeSpace(&sp,title_xPos+ARROW_HEIGHT_WIDTH+10,title_yPos + 20);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeASprite(&sp,pick_xPos,pick1_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeDigit(&sp,1,pick_xPos+CHAR_WIDTH+15, pick1_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeHyphen(&sp,pick_xPos+CHAR_WIDTH*2 +10,pick1_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makePlayer(&sp,pick_xPos+CHAR_WIDTH*3+20, pick1_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);
    
    makeBSprite(&sp,pick_xPos,pick2_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeDigit(&sp,2,pick_xPos+CHAR_WIDTH+15, pick2_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeHyphen(&sp,pick_xPos+CHAR_WIDTH*2 +10,pick2_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makePlayer(&sp,pick_xPos+CHAR_WIDTH*3+20, pick2_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

}

void makeDeathScreenOne(GameScreen * g) {
    size_t continue_xPos,continue_yPos, score_xPos,score_yPos;
    
    score_xPos = SCREEN_WIDTH/2 - LEVEL_WIDTH;
    score_yPos = 300;
    continue_xPos = 230;
    continue_yPos = SCREEN_HEIGHT/2 + 100;

    Sprite sp,sp1;
    
    makeScore(&sp,score_xPos, score_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeNum(&sp,&sp1,g->level,score_xPos+ LEVEL_WIDTH +25,score_yPos);
    if (g->level > 9) { placeSprite(g->pixel_arr,&sp); free(sp.pixel_arr);}
    placeSprite(g->pixel_arr,&sp1);
    free(sp1.pixel_arr);

    makePress(&sp,continue_xPos,continue_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeStart(&sp,continue_xPos + LEVEL_WIDTH+10,continue_yPos, RED);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeToContinue(&sp, continue_xPos + 2*(LEVEL_WIDTH+10),continue_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);
}

void makeDeathScreenTwo(GameScreen * g) {
    size_t xPos, yPos,lose_xPos,win_xPos, continue_xPos, continue_yPos;
    xPos = 200;
    yPos = 400;
    continue_xPos = xPos - 50;
    continue_yPos = 800;
    
    Sprite sp;
    sp.pixel_arr = g->pixel_arr;
    sp.height = SCREEN_HEIGHT;
    sp.width = SCREEN_WIDTH;
    size_t i;
    
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        setPixelAt(SCREEN_WIDTH/2,i,&sp,WHITE);    
    }

    if (g->winner != 0) {
        makeYou(&sp,xPos, yPos,WHITE);
        placeSprite(g->pixel_arr,&sp);
        free(sp.pixel_arr);

        makeYou(&sp,SCREEN_WIDTH/2 + xPos,yPos,WHITE);
        placeSprite(g->pixel_arr,&sp);
        free(sp.pixel_arr);

        if (g->winner == 1) {
            win_xPos = xPos;
            lose_xPos = xPos + SCREEN_WIDTH/2;
            
        } else {
            lose_xPos = xPos;
            win_xPos = xPos + SCREEN_WIDTH/2;
            lose_xPos = xPos; 
        }

        makeLose(&sp,lose_xPos,yPos+LEVEL_HEIGHT+20,RED);
        placeSprite(g->pixel_arr,&sp);
        free(sp.pixel_arr);
        
        makeWin(&sp,win_xPos,yPos+LEVEL_HEIGHT+20,GREEN);
        placeSprite(g->pixel_arr,&sp);
        free(sp.pixel_arr);
 
    } else {

        makeTie(&sp,xPos,yPos,BLUE);
        placeSprite(g->pixel_arr,&sp);
        free(sp.pixel_arr);
        
        makeTie(&sp,xPos+SCREEN_WIDTH/2,yPos,BLUE);
        placeSprite(g->pixel_arr,&sp);
        free(sp.pixel_arr);
    }

    makePress(&sp,continue_xPos,continue_yPos);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeStart(&sp,continue_xPos + LEVEL_WIDTH+10,continue_yPos, RED);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

    makeToContinue(&sp, continue_xPos,continue_yPos + LEVEL_HEIGHT + 20);
    placeSprite(g->pixel_arr,&sp);
    free(sp.pixel_arr);

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
