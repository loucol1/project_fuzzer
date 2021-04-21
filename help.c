#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct tar_t
{                              /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 pas */ 
    char devminor[8];             /* 337 pas */
    char prefix[155];             /* 345 pas */
    char padding[12];             /* 500 pas */
};

int write_in_file(struct tar_t* to_write, char* content_of_file){
    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char end_of_archive [1024];
    //memset(end_of_archive, NULL, 1024);
    for(int i=0; i<=1023; i++){
        end_of_archive[i]=NULL;
    }

    if (fichier != NULL)
    {
        // On peut lire et écrire dans le fichier
        fwrite(to_write, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content_of_file, 5*sizeof(char),1,fichier);//contenu du fichier
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//fin du fichier
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        // l'ouverture du fichier .tar ne s'est pas bie faite
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
    return 0;
}






/**
 * Computes the checksum for a tar header and encode it on the header
 * @param entry: The tar header
 * @return the value of the checksum
 */
unsigned int calculate_checksum(struct tar_t* entry){
    // use spaces for the checksum bytes while calculating the checksum
    memset(entry->chksum, ' ', 8);

    // sum of entire metadata
    unsigned int check = 0;
    unsigned char* raw = (unsigned char*) entry;
    for(int i = 0; i < 512; i++){
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return check;
}








/** BONUS (for fun, no additional points) without modifying this code,
 * compile it and use the executable to restart our computer.
 */

/**
 * Launches another axecutable given as argument,
 * parses its output and check whether or not it matches "*** The program has crashed ***".
 * @param the path to the executable
 * @return -1 if the executable cannot be launched,
 *          0 if it is launched but does not print "*** The program has crashed ***",
 *          1 if it is launched and prints "*** The program has crashed ***".
 *
 * BONUS (for fun, no additional marks) without modifying this code,
 * compile it and use the executable to restart our computer.
 */

 void check_extractor(int *count_crash, int *count_other_msg, char* argument, char* name_file){
     char cmd[100];
    strcpy(cmd, argument);
    //cmd[30] = '\0';
    //test of the structure
    strncat(cmd, " archive.tar", 12);

    char buf[33];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return;
    }

    if(fgets(buf, 33, fp) == NULL) {
        //printf("No output\n");
        char remove[120];
        strcpy(remove, "sudo rm ");
        strncat(remove, name_file, strlen(name_file));
        system(remove);
        goto finally;
    }
    if(strncmp(buf, "*** The program has crashed ***\n", 33)) {
        printf("Not the crash message\n");
        *count_other_msg++;
        goto finally;
    } else {
        printf("Crash message\n");
        *count_crash++;
        goto finally;
    }
    finally:
    if(pclose(fp) == -1) {
        printf("Command not found\n");
        //rv = -1;
    }

 }

void change_name(int *count_crash, int *count_other_msg, char* argument){
    struct tar_t* test_sacha1 = (struct tar_t*) malloc(sizeof(struct tar_t));
    unsigned int first;
    for(first= 0x80; first!=0xFF; first++){
        test_sacha1->name[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[5]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    printf("out of first for loop \n");
    first = 0x80;
    for(int index=1;index<99;index++){
        test_sacha1->name[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[5]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }




/*
    unsigned int i;
    for(int j=95;j<99;j++){
        printf("j = %d\n", j);
        for(i=0x7F;i!=0xFF; i++){
        test_sacha1->name[j] = i;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[5]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        
        }
    }
    */
    free(test_sacha1);
    




    
}


int main(int argc, char* argv[])
{
    if (argc < 2)
        return -1;
    int count_crash = 0;
    int count_other_msg = 0;
    change_name(&count_crash, &count_other_msg, argv[1]);
    printf ("number of crashes = %d\n", count_crash);
    printf ("number of other msg = %d\n", count_other_msg);
    return 0;
}




