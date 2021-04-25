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

void dec_to_oct(int dec, char* to_ret){
int oct = 0;
int i = 1;
while(dec !=0){
    oct = oct +(dec%8)*i;
    dec = dec/8;
    i = i*10;
}
sprintf(to_ret, "%d", oct);
}

int write_in_file(struct tar_t* to_write, char* content_of_file){
    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        // On peut lire et écrire dans le fichier
        fwrite(to_write, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content_of_file, strlen(content_of_file),1,fichier);//contenu du fichier
        int nbr_to_write = 512-(strlen(content_of_file)%512);
        char zero[2] = "0";
        fwrite(zero, sizeof(char), nbr_to_write, fichier); 
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

 int check_extractor(int *count_crash, int *count_other_msg, char* argument, char* name_file){
     char cmd[100];
    strcpy(cmd, argument);
    //cmd[30] = '\0';
    //test of the structure
    strcat(cmd, " archive.tar");

    char buf[33];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return 3;
    }

    if(fgets(buf, 33, fp) == NULL) {
        //printf("No output\n");
        char remove[120];
        strcpy(remove, "sudo rm -f ");
        strcat(remove, name_file);
        system(remove);
        if(pclose(fp) == -1) {
            printf("Command close not found\n");
            //rv = -1;
        }
        return 0;
    }
    if(strncmp(buf, "*** The program has crashed ***\n", 33)) {
        printf("Not the crash message\n");
        *count_other_msg= *count_other_msg+1;
        if(pclose(fp) == -1) {
            printf("Command close not found\n");
            //rv = -1;
        }
        return 2;
    } else {
        printf("Crash message\n");
        *count_crash = *count_crash+1;
        char dest[20]=" success_";
        char to_add[5];
        char cmd_cp[50] = "cp ";
        sprintf(to_add, "%d", *count_crash);
        strcat(dest, to_add);//on a le nom du fichier
        strcat(dest, ".tar");
        strcat(cmd_cp, "archive.tar");
        strcat(cmd_cp, dest);
        system(cmd_cp);
        if(pclose(fp) == -1) {
            printf("Command not found\n");
            //rv = -1;
        }
        return 1;
    }   

 }

void change_name(int *count_crash, int *count_other_msg, char* argument){
    int flag=0;
    printf("change name\n");
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->name[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<99 && flag!=1;index++){
        test_sacha1->name[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}



void change_mode(int *count_crash, int *count_other_msg, char* argument){
    printf("change mode\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "mode");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->mode[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){
        test_sacha1->mode[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    strcpy(test_sacha1->mode, "0000000");

    for(int index = 0;index<7 && flag!=1; index++){ //test all octal
        for(int new_uid=0;new_uid<8 && flag!=1; new_uid++){
            char nbr = new_uid+'0';
            test_sacha1->mode[index] = nbr;
            //test_sacha1->gid[8]='\0';
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }

    free(test_sacha1);    
}


void change_uid(int *count_crash, int *count_other_msg, char* argument){
    printf("change uid\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "uid");

    for(int index = 0;index<7 && flag!=1; index++){ //test all octal
        for(int new_uid=0;new_uid<8 && flag!=1; new_uid++){
            char nbr = new_uid+'0';
            test_sacha1->uid[index] = nbr;
            //test_sacha1->gid[8]='\0';
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    flag = 0;

    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->uid[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){
        test_sacha1->uid[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

void change_gid(int *count_crash, int *count_other_msg, char* argument){
    printf("change gid\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "gid");
    //strcpy(test_sacha1->gid, "0000000");
    for(int index = 0;index<7 && flag!=1; index++){ //test all octal
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->gid[index] = nbr;
            //test_sacha1->gid[8]='\0';
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    flag = 0;
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){ //test non octal
        test_sacha1->gid[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){
        test_sacha1->gid[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);
}





void change_size(int *count_crash, int *count_other_msg, char* argument){
    printf("change size\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "size");
    strcpy(test_sacha1->size, "00000000000");
    for(int index = 0;index<11 && flag!=1; index++){//test octal
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->size[index] = nbr;
            //test_sacha1->gid[8]='\0';
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            //strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }

    flag = 0;
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){ //test non octal
        test_sacha1->size[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<11 && flag!=1;index++){
        test_sacha1->size[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);
}

void change_mtime(int *count_crash, int *count_other_msg, char* argument){
    printf("change mtime\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "mtime");
    strcpy(test_sacha1->mtime, "00000000000");
    for(int index = 0;index<11 && flag!=1; index++){
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->mtime[index] = nbr;
            //test_sacha1->gid[8]='\0';
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }

    flag = 0;
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){ //test non octal
        test_sacha1->mtime[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<11 && flag!=1;index++){
        test_sacha1->mtime[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);
}



void change_chksum(int *count_crash, int *count_other_msg, char* argument){
    printf("change chksum\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "chksum");
    for(int index = 0;index<7 && flag!=1; index++){
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->chksum[index] = nbr;
            //test_sacha1->gid[8]='\0';
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    flag = 0;
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->chksum[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        //strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){
        test_sacha1->chksum[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        //strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

void change_typeflag(int *count_crash, int *count_other_msg, char* argument){
    printf("change typeflag\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "typeflag");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->typeflag = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    flag = 0;
    for(first = 0x30; first!=0x40 && flag!=1; first++){
        test_sacha1->typeflag = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);

    }
    free(test_sacha1);    
}

void change_linkname(int *count_crash, int *count_other_msg, char* argument){
    printf("change linkname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "linkname");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->linkname[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<99 && flag!=1;index++){
        test_sacha1->linkname[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}



void change_magic(int *count_crash, int *count_other_msg, char* argument){
    printf("change magic\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "magic");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->magic[0] = first;
        //strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<5 && flag!=1;index++){
        test_sacha1->magic[index]= first;
        //strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

void change_version(int *count_crash, int *count_other_msg, char* argument){
    printf("change version\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "version");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->version[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        //strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

void change_uname(int *count_crash, int *count_other_msg, char* argument){
    printf("change uname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "uname");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->uname[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<31 && flag!=1;index++){
        test_sacha1->uname[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

void change_gname(int *count_crash, int *count_other_msg, char* argument){
    printf("change gname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "gname");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        test_sacha1->gname[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<31 && flag!=1;index++){
        test_sacha1->gname[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");//pcq on met 5*A dans le fichier 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

int without_zero_at_the_end(int *count_crash, int *count_other_msg, char* argument){
    printf("without zero at the end\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "without_zero");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[6]="AAAAA";
    //char size_of_content = (char) sizeof(content);
    char *size_of_content = (char*) calloc(12, sizeof(char));
    dec_to_oct(strlen(content), size_of_content);
    strcpy(test_sacha1->size, size_of_content);//pcq on met 5*A dans le fichier 
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));
    if (fichier != NULL)
    {
        // On peut lire et écrire dans le fichier
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content, strlen(content),1,fichier);//contenu du fichier
        int nbr_to_write = 512-(strlen(content));
        char zero[2] = "0";
        fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        //fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//fin du fichier
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
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}


void change_content(int *count_crash, int *count_other_msg, char* argument){
    printf("change content\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "content");
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content=first;
        char* size_of_content = (char*) calloc(12, sizeof(char));
        dec_to_oct(sizeof(content), size_of_content);
        strcpy(test_sacha1->size, size_of_content);
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, &content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    char content[32];
    for(int index=1;index<31 && flag!=1;index++){
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        content[index]=first;
        char* size_of_content = (char*) calloc(12, sizeof(char));
        dec_to_oct(sizeof(content), size_of_content);
        strcpy(test_sacha1->size, size_of_content);
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    /*
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    printf("debug\n");
    
    char* large_content = (char *)calloc(8589934591, sizeof(char));
    
    printf("debug2\n");
    
    memset(large_content, 'a', 8589934590*sizeof(char));
    large_content[8589934590] = '\0';
    
    printf("debug3\n");
    strcpy(test_sacha1->size, "77777777776");
    printf("debug4\n");
    int check = calculate_checksum(test_sacha1);
    int ret = write_in_file(test_sacha1, large_content); 
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    
    free(large_content);
    */
    free(test_sacha1);    


}



int no_data(int *count_crash, int *count_other_msg, char* argument){
    printf("no_data\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "no_data");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    strcpy(test_sacha1->size, "00");//pcq on met 5*A dans le fichier 
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        // On peut lire et écrire dans le fichier
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header

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
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}


int two_header_no_data(int *count_crash, int *count_other_msg, char* argument){
    printf("two header no_data\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "no_data");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    strcpy(test_sacha1->size, "00");//pcq on met 5*A dans le fichier 
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        // On peut lire et écrire dans le fichier
        for(int i=0;i<100;i++){
            fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        }
        
        //fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
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
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}

int lot_of_header(int *count_crash, int *count_other_msg, char* argument){
    printf("lot of header\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "lot_of_header");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[7]="azerty";
    char* size_of_content = (char*) calloc(12, sizeof(char));
    dec_to_oct(strlen(content), size_of_content);
    //printf("sizeof content = %s\n", size_of_content);
    strcpy(test_sacha1->size, size_of_content);
    int check = calculate_checksum(test_sacha1);


    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        for(int rep=0; rep<1000; rep++){
            fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
            fwrite(content, strlen(content),1,fichier);//contenu du fichier
            int nbr_to_write = 512-(strlen(content));
            char zero[2] = "0";
            int test_fwrite = fwrite(zero, sizeof(char), nbr_to_write, fichier);       

        }
        // On peut lire et écrire dans le fichier
        
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
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}



int write_after_end(int *count_crash, int *count_other_msg, char* argument){
    printf("write_after_end\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "write_after_end");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[7]="azerty";
    char* size_of_content = (char*) calloc(12, sizeof(char));
    dec_to_oct(strlen(content), size_of_content);
    printf("sizeof content = %s\n", size_of_content);
    strcpy(test_sacha1->size, size_of_content);
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));
    /*
    char end_of_archive [1024];
    //memset(end_of_archive, NULL, 1024);
    for(int i=0; i<=1023; i++){
        end_of_archive[i]=NULL;
    }
    */
    if (fichier != NULL)
    {
        // On peut lire et écrire dans le fichier
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content, strlen(content),1,fichier);//contenu du fichier
        int nbr_to_write = 512-(strlen(content));
        char zero[2] = "0";
        fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//fin du fichier
        char write_end[10]="after end";
        fwrite(write_after_end, 9*sizeof(char), 1, fichier);
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
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}

int test(int *count_crash, int *count_other_msg, char* argument){
    printf("test\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "coucou.txt");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[20]="azerty20";
    char* size_of_content = (char*) calloc(12, sizeof(char));
    dec_to_oct(strlen(content), size_of_content);
    strcpy(test_sacha1->size, size_of_content);
    //strcpy(test_sacha1->size, "00000000000");
    int check = calculate_checksum(test_sacha1);
    

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1025, sizeof(char));
    //memset(end_of_archive, '\0', 1024);



    if (fichier != NULL)
    {
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content, strlen(content),1,fichier);//contenu du fichier
        int nbr_to_write = 512-strlen(content);
        char zero[2] = "0";
        int test_fwrite = fwrite(zero, sizeof(char), nbr_to_write, fichier);   
        //printf("test_fwrite = %d\n", test_fwrite);         

        
        // On peut lire et écrire dans le fichier
        
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
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
        return -1;
    int count_crash = 0;
    int count_other_msg = 0;
    
    change_mode(&count_crash, &count_other_msg, argv[1]);
    change_uid(&count_crash, &count_other_msg, argv[1]);
    change_gid(&count_crash, &count_other_msg, argv[1]);
    change_size(&count_crash, &count_other_msg, argv[1]);
    change_mtime(&count_crash, &count_other_msg, argv[1]);
    change_chksum(&count_crash, &count_other_msg, argv[1]);
    change_name(&count_crash, &count_other_msg, argv[1]);
    change_typeflag(&count_crash, &count_other_msg, argv[1]);
    change_linkname(&count_crash, &count_other_msg, argv[1]);
    change_magic(&count_crash, &count_other_msg, argv[1]);
    change_version(&count_crash, &count_other_msg, argv[1]);
    change_uname(&count_crash, &count_other_msg, argv[1]);
    change_gname(&count_crash, &count_other_msg, argv[1]);

    without_zero_at_the_end(&count_crash, &count_other_msg, argv[1]);
    change_content(&count_crash, &count_other_msg, argv[1]);
    no_data(&count_crash, &count_other_msg, argv[1]);
    two_header_no_data(&count_crash, &count_other_msg, argv[1]);
    lot_of_header(&count_crash, &count_other_msg, argv[1]);
    write_after_end(&count_crash, &count_other_msg, argv[1]);
    //test(&count_crash, &count_other_msg, argv[1]);

    printf ("number of crashes = %d\n", count_crash);
    printf ("number of other msg = %d\n", count_other_msg);
    return 0;
}




