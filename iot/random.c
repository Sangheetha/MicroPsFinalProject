#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp;
    
    printf("%s%c%c\n",
           "Content-Type:text/html;charset=iso-8859-1",13,10);
    // Open file
    fp = fopen("/home/pi/key_seq.dat", "w");
    if (fp == NULL){
        printf("Error opening file.\n");
        exit(0);
    }
    // Write to file
    //For random mode, just wipe the file
    fprintf(fp,"");

    // Close file
    fclose(fp);
    
    return 0;
}
