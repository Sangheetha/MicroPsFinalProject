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
    printf("%s\n",getenv("HTTP_REFERER"));
    /*const char * redirect_page_format =
    "<html>\n"
    "<head>\n"
    "<meta http-equiv=\"REFRESH\"\n"
    "content=\"0;url=%s\">\n"
    "</head>\n"
    "</html>\n";
    printf (redirect_page_format, getenv("HTTP_REFERER")); */
    printf("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0;url=%s\">",getenv("HTTP_REFERER"));   
    return 0;
}
