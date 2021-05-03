#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int test_char(char* c){
if(c==NULL){
    printf("NULL\n");
    
}
else{
    printf("OK\n");
}
return 0;
}


int main(int argc, char* argv[])
{
    char *alpha = "alpha";
    test_char(NULL);
}




