#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp;
    char *data;
    int key_seq;
    
        data = getenv("QUERY_STRING");
    if(data == NULL){
        printf(" Error passing data from form to script.");
        exit(0);
    }
    /*else if (sscanf(data,"key_seq=%i",&key_seq) != 1) {
        printf("<P> Invalid input. Please enter only numbers.</P>");
    } else {*/
    // Open file
    sscanf(data,"key_seq=%i",&key_seq);     
    fp = fopen("/home/pi/key_seq.dat", "a+");
    if (fp == NULL){
        printf("Error opening file.\n");
        exit(0);
    }
    // Write to file

    fprintf(fp,"%i\n",key_seq);
    // Close file
    fclose(fp);
    printf("%s%c%c\n","Content-Type:text/html;charset=iso-8859-1",13,10);
  
    return 0;
}
