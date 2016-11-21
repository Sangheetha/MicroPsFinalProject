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

#define LIFE_BAR_HEIGHT 75
#define LIFE_BAR_MAX_WIDTH 400
#define LIFE_POINTS 10
#define LIFE_BAR_X_POS 100
#define LIFE_BAR_Y_POS 50

#define ARROW_HEIGHT_WIDTH 100

#define TIME_HEIGHT 50
#define TIME_WIDTH 450

#define LIFE_BAR 0
#define RIGHT_ARROW 4
#define LEFT_ARROW 3
#define DOWN_ARROW 2
#define UP_ARROW 1
#define TIMER_BAR 5
#define TIMER_MARK 6
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
    Sprite key_arr[20];
    size_t level;
    size_t size;
    size_t arrow_index;
    int life_1;
    Sprite timer_bar_1;
    Sprite timer_mark_1;
    Sprite life_bar_1;
};

typedef struct GameScreen GameScreen;


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
void makeLifeBar(Sprite* sp, int lifepoints) {
    sp->width = LIFE_BAR_MAX_WIDTH;
    sp->height = LIFE_BAR_HEIGHT;
    sp->x_pos = LIFE_BAR_X_POS;
    sp->y_pos = LIFE_BAR_Y_POS;
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
           if ((col < sp->width/2 - row ||
                col > sp->width/2 + row ) && row < 6) {
                setPixelAt(col,row,sp,BLACK);
           } else if (!(row > sp->width/2 &&
                        (col < sp->width/4 || col > 3*sp->width/4))) {
                setPixelAt(col,row,sp,BLUE);
           }
        }
     } 

}

void moveSpriteRight(Sprite* sp) {
    sp->x_pos++;
    sp->x_pos++;
    sp->x_pos++;
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

void addSpriteToGame(int sprite_name,GameScreen* g, int x_pos, int y_pos) {
    
    Sprite sp;
    Sprite* keys = g->key_arr;
    size_t numKeys = g->size;
    switch(sprite_name) {
        case INVALID_SPRITE:
          break;
        case LIFE_BAR:
          makeLifeBar(&(g->life_bar_1),g->life_1);
          break;
        case RIGHT_ARROW:
          makeRightArrow(&keys[numKeys],x_pos, y_pos,BLUE);
          g->size++;
          break;
        case LEFT_ARROW:
          makeLeftArrow(&keys[numKeys],x_pos,y_pos,BLUE);
          g->size++;
          break;
        case DOWN_ARROW:
          makeDownArrow(&keys[numKeys],x_pos,y_pos,BLUE);
          g->size++;
          break;
        case UP_ARROW:
          makeUpArrow(&keys[numKeys],x_pos,y_pos,BLUE);
          g->size++;
          break;
        case TIMER_BAR:
          makeTimerBar(&(g->timer_bar_1),x_pos,y_pos);
          break;
        case TIMER_MARK:
          makeTimerMark(&(g->timer_mark_1),x_pos,y_pos);
          break;
    }
}

void updateGameScreenSinglePlayer(GameScreen * g) 
{
    placeSprite(g->pixel_arr,&(g->life_bar_1));
    placeSprite(g->pixel_arr,&(g->timer_bar_1));
    placeSprite(g->pixel_arr,&(g->timer_mark_1));
    size_t i;
    for (i = 0; i < g->size; i++) {
        placeSprite(g->pixel_arr,&(g->key_arr[i]));
    }
}

int levelOver(GameScreen * g)
{
    Sprite timer_mark = g->timer_mark_1;
    Sprite timer_bar = g->timer_bar_1;
    return ((timer_mark.x_pos + timer_mark.width) > timer_bar.x_pos + timer_bar.width); 
}

void resetKeys(GameScreen *g) {
    size_t i;
    for (i = 0; i <= g->arrow_index; i++) {
        changeArrowColor(&(g->key_arr[i]),BLUE);
    }

    g->arrow_index = 0;
}

void clearSprites(GameScreen *g) {
    free(g->timer_mark_1.pixel_arr);
    free(g->timer_bar_1.pixel_arr);
    free(g->life_bar_1.pixel_arr);

    size_t i;
    for (i = 0; i <= g->arrow_index; i++) {
        free(g->key_arr[i].pixel_arr);
    }

    g->arrow_index = 0;
    g->size = 0;
}

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
