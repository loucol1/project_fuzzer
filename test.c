#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void dec_to_oct(long dec, char* to_ret){
int oct = 0;
int i = 1;
while(dec !=0){
    oct = oct +(dec%8)*i;
    dec = dec/8;
    i = i*10;
}
printf("oct = %d\n", oct);
sprintf(to_ret, "%d", oct);
}


int main(int argc, char* argv[])
{
    char test [15] = "test";
    int i=0;
    while(test[i]!='\0'){
        i++;
    }
    printf("i = %d\n", i);
    return 0;


}




