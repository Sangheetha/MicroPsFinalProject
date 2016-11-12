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
#define BRIGHT_BLUE 0xf0
#define RED 0xff3300

#define LIFE_BAR_HEIGHT 75
#define LIFE_BAR_MAX_WIDTH 400
#define LIFE_POINTS 10
#define LIFE_BAR_X_POS 100
#define LIFE_BAR_Y_POS 50

#define ARROW_HEIGHT_WIDTH 100

struct Sprite {
    int * pixel_arr;
    int width;
    int height;
    int x_pos;
    int y_pos;
 };

typedef struct Sprite Sprite;

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

void makeRightArrow(Sprite* sp, size_t x_pos, size_t y_pos){
    sp->x_pos = x_pos;
    sp->y_pos = y_pos;
    sp->width = ARROW_HEIGHT_WIDTH;
    sp->height = ARROW_HEIGHT_WIDTH;
    sp->pixel_arr = (int*) malloc(ARROW_HEIGHT_WIDTH*ARROW_HEIGHT_WIDTH*sizeof(int));
    size_t row,col;
    for (row = 0; row < ARROW_HEIGHT_WIDTH; row++) {
        for (col = 0; col < ARROW_HEIGHT_WIDTH; col++) {
           setPixelAt(col,row,sp,BRIGHT_BLUE); 
         }
    }

}

int main(int argc, char* argv[])
{
  int fbfd = 0;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screensize = 0;
  int *fbp = 0;

  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    printf("Error: cannot open framebuffer device.\n");
    return(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
  }

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
  }
  printf("%dx%d, %d bpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );
  // map framebuffer to user memory 
  screensize = finfo.smem_len;
  long int screensize_in_int = screensize/4;

  fbp = (int*)mmap(0, 
                    screensize, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    fbfd, 0);

  if ((int)fbp == -1) {
    printf("Failed to mmap.\n");
  }
  else {
    // draw...
    // just fill upper half of the screen with something
    while(1) {
        int screen[1920*1080]; //Temp array to hold next state of screen
        
        Sprite life;
        makeLifeBar(&life,9);

        Sprite arrow;
        makeRightArrow(&arrow,300,300);
        
        clearContents(screen,screensize_in_int);
        placeSprite(screen, &life);
        placeSprite(screen,&arrow);
        updateScreen(fbp,screen,screensize_in_int);
    }
    /*int* ip;
    for (ip = fbp; ip < fbp + screensize_in_int; ip++) {
        *ip = RED; 
    }
    int* lp;
    for (lp = ip; lp < ip + screensize_in_int; lp++) {
        *lp = 0xf0;
    }*/
    //printf("screensize is %d\n" , screensize);
    // and lower half with something else
    
  }

  // cleanup
  munmap(fbp, screensize);
  close(fbfd);
  return 0;
}
