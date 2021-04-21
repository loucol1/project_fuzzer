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
        if(fwrite!=0){
            printf("content written successfully\n");
        }
        else{
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
int main(int argc, char* argv[])
{
    if (argc < 2)
        return -1;
    int rv = 0;
    char cmd[49];
    strcpy(cmd, argv[1]);
    //cmd[30] = '\0';

    //creation of the structure to test 
    struct tar_t* test_sacha1 = (struct tar_t*) malloc(sizeof(struct tar_t));
    strcpy(test_sacha1->name, "Sacha.txt");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[8]="AAAAAA";
    char size_of_content = (char) sizeof(content);
    strcpy(test_sacha1->size, "10");//pcq on met 5*A dans le fichier 
    int check = calculate_checksum(test_sacha1);
    int ret = write_in_file(test_sacha1, content);

    //test of the structure
    strncat(cmd, " archive.tar", 12);
    printf("cmd = %s \n",cmd);
    char buf[33];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    if(fgets(buf, 33, fp) == NULL) {
        printf("No output\n");
        goto finally;
    }
    if(strncmp(buf, "*** The program has crashed ***\n", 33)) {
        printf("Not the crash message\n");
        goto finally;
    } else {
        printf("Crash message\n");
        rv = 1;
        goto finally;
    }
    finally:
    if(pclose(fp) == -1) {
        printf("Command not found\n");
        rv = -1;
    }
    return rv;
}




