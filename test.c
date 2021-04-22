#include <stdio.h>
#include <string.h>
#include <stdlib.h>




int main(int argc, char* argv[])
{

    char c[8]="aaaaaaaa";
    for(int index = 0;index<8; index++){
        for(int new_gid=0;new_gid<8; new_gid++){
            char nbr = new_gid+'0';
            c[index] = nbr;
            printf("gid = %s\n", c);
        }
    }
    return 0;


}




