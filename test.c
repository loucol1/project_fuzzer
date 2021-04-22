#include <stdio.h>
#include <string.h>
#include <stdlib.h>




int main(int argc, char* argv[])
{
    int nbr = 8;
    char dest[20]=" success_";
    char to_add[5];
    char cmd_cp[50] = "cp ";
    sprintf(to_add, "%d", nbr);
    strcat(dest, to_add);//on a le nom du fichier
    strcat(dest, ".tar");
    strcat(cmd_cp, "archive.tar");
    strcat(cmd_cp, dest);        

    system(cmd_cp);
    return 0;
}




