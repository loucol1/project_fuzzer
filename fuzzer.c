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

/**
 * transform a decimal integer into an octal value
 * @param dec: the decimal value to transform
 * @param to_ret: the octal value stored into a char*
 */
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


/**
 * Create a tar file with a header and a content
 * @param to_write: The tar header
 * @param content_of_file: the content of the file to put on a tar
 */
int write_in_file(struct tar_t* to_write, char* content_of_file){
    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        fwrite(to_write, sizeof(struct tar_t), 1, fichier);//header
        if(content_of_file!=NULL){
            fwrite(content_of_file, strlen(content_of_file),1,fichier);//payload
            int nbr_to_write = 512-(strlen(content_of_file)%512);
            char zero[2] = "0";
            fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        }
        
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//end of file
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        // can not open the .tar file
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

/**
 * Launches another axecutable given as argument,
 * parses its output and check whether or not it matches "*** The program has crashed ***".
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 * @param name_file : the name fiels of the archive to extract
 * @return -1 if the executable cannot be launched,
 *          0 if it is launched but does not print "*** The program has crashed ***",
 *          1 if it is launched and prints "*** The program has crashed ***".
 *
 */

 int check_extractor(int *count_crash, int *count_other_msg, char* argument, char* name_file){
    char cmd[100];
    strcpy(cmd, argument);
    strcat(cmd, " archive.tar ");

    char buf[33];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return 3;
    }

    if(fgets(buf, 33, fp) == NULL) {
        /*
        char remove[120];
        strcpy(remove, "sudo rm -f ");
        strcat(remove, name_file);
        system(remove);
        */
        remove(name_file);
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
        }
        return 2;
    } else {
        printf("Crash message\n");
        *count_crash = *count_crash+1;
        char dest[20]=" success_";
        char to_add[5];
        char cmd_cp[50] = "cp ";
        sprintf(to_add, "%d", *count_crash);
        strcat(dest, to_add);
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


/**
 * fuzz the name field of a header with: - all non ascii character for name[0]
 *                                       - a non ascii character for all index of name[index]
 *                                       - all number and all upper and lower letters of name[0]
 *                                       - non null terminated string name
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_name(int *count_crash, int *count_other_msg, char* argument){
    int flag=0;
    printf("change name\n");
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    unsigned int first;
    for(first= 0x80; first!=0xFF && flag!=1; first++){ //test all non ascii character for name[0]
        test_sacha1->name[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        test_sacha1->typeflag = 'g';
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<100 && flag!=1;index++){ //test a non ascii character for all index of name[index] + non null terminated string name
        test_sacha1->name[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    test_sacha1->name[99] = '\0';
    for(first= 0x30; first!=0x3A && flag!=1; first++){ //test all number for name[0]
        test_sacha1->name[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        test_sacha1->typeflag = 'g';
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }

    for(first= 0x3F; first!=0x5B && flag!=1; first++){//test all upper case for name [0]
        test_sacha1->name[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        test_sacha1->typeflag = 'g';
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }


    for(first= 0x61; first!=0x7B && flag!=1; first++){ //test all lower case for name[0]
        test_sacha1->name[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        test_sacha1->typeflag = 'g';
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }

    free(test_sacha1);    
}


/**
 * fuzz the mode field of a header with: - all values (ascii and non ascii) for mode[0]
 *                                       - a non ascii character for all index of mode[index]
 *                                       - all number in all positions
 *                                       - non null terminated string mode
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_mode(int *count_crash, int *count_other_msg, char* argument){
    printf("change mode\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "mode");
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){// test all values (ascii and non ascii) for mode[0]
        test_sacha1->mode[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    
    first = 0x80;
    for(int index=1;index<8 && flag!=1;index++){// test a non ascii character for all index of mode[index] + non null terminated string name
        test_sacha1->mode[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    test_sacha1->mode[7] = '\0';
    strcpy(test_sacha1->mode, "0000000");

    for(int index = 0;index<7 && flag!=1; index++){ //test all number in all positions
        for(int new_uid=0;new_uid<10 && flag!=1; new_uid++){
            char nbr = new_uid+'0';
            test_sacha1->mode[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }



    free(test_sacha1);    
}


/**
 * fuzz the uid field of a header with: - all values (ascii and non ascii) for uid[0]
 *                                       - a non ascii character for all index of uid[index]
 *                                       - all octal values in all positions
*                                        - non null terminated string uid
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_uid(int *count_crash, int *count_other_msg, char* argument){
    printf("change uid\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "uid");

    for(int index = 0;index<8 && flag!=1; index++){ //test all octal + non null terminated string uid
        for(int new_uid=0;new_uid<8 && flag!=1; new_uid++){
            char nbr = new_uid+'0';
            test_sacha1->uid[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05"); 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    test_sacha1->uid[7] = '\0';
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){//test all values
        test_sacha1->uid[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){//test a non ascii character for all index of uid[index] 
        test_sacha1->uid[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    
    free(test_sacha1);    
}


/**
 * fuzz the the gid and uid field with: non octal values
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_uid_gid(int *count_crash, int *count_other_msg, char* argument){
    printf("change uid_gid\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "uid_gid");
    test_sacha1->gid[0] = '9';
    for(int index = 0;index<7 && flag!=1; index++){ //test all octal
        for(int new_uid=0;new_uid<9 && flag!=1; new_uid++){
            char nbr = new_uid+'0';
            test_sacha1->uid[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05"); 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    free(test_sacha1);    
}




/**
 * fuzz the gid field of a header with: - all values (ascii and non ascii) for gid[0]
 *                                      - a non ascii character for all index of gid[index]
 *                                      - all octal values in all positions
 *                                      - non null terminated string gid
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_gid(int *count_crash, int *count_other_msg, char* argument){
    printf("change gid\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "gid");
    for(int index = 0;index<8 && flag!=1; index++){ //test all octal values in all positions + non null terminated string gid
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->gid[index] = nbr;
            strcpy(test_sacha1->version, "00");
            strcpy(test_sacha1->mode, "02000");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    test_sacha1->gid[7] = '\0';
    //flag = 0;
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for gid[0]
        test_sacha1->gid[0] = first;
        strcpy(test_sacha1->version, "00");
        strcpy(test_sacha1->mode, "02000");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){ // test a non ascii character for all index of gid[index]
        test_sacha1->gid[index]= first;
        strcpy(test_sacha1->version, "00");
        strcpy(test_sacha1->mode, "02000");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);
}




/**
 * fuzz the size field of a header with: - all values (ascii and non ascii) for size[0]
 *                                      - a non ascii character for all index of size[index]
 *                                      - all octal values in all positions
 *                                       - non null terminated string size
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_size(int *count_crash, int *count_other_msg, char* argument){
    printf("change size\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "size");
    strcpy(test_sacha1->size, "00000000000");
    int new_gid;
    for(int index = 0;index<12 && flag!=1; index++){//test all octal values in all positions + non null terminated string size
        for(new_gid=0;new_gid<8 && flag!=1; new_gid++){
            test_sacha1->typeflag = '0';
            char nbr = new_gid+'0';
            test_sacha1->size[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    test_sacha1->size[11] = '\0';
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for size[0]
        test_sacha1->typeflag = '0';
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
    for(int index=1;index<11 && flag!=1;index++){ //test a non ascii character for all index of size[index]
        test_sacha1->typeflag = '0';
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




/**
 * fuzz the mtime field of a header with: - all values (ascii and non ascii) for mtime[0]
 *                                        - a non ascii character for all index of mtime[index]
 *                                        - all octal values in all positions
 *                                        - non null terminated string mtime
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_mtime(int *count_crash, int *count_other_msg, char* argument){
    printf("change mtime\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "mtime");
    strcpy(test_sacha1->mtime, "00000000000");
    for(int index = 0;index<12 && flag!=1; index++){// test all octal values in all positions + non null terminated string mtime
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->mtime[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05"); 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    test_sacha1->mtime[11] = '\0';
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for mtime[0]
        test_sacha1->mtime[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<11 && flag!=1;index++){ // test a non ascii character for all index of mtime[index]
        test_sacha1->mtime[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);
}


/**
 * fuzz the chksum field of a header with: - all values (ascii and non ascii) for chksum[0]
 *                                         - a non ascii character for all index of chksum[index]
 *                                         - all octal values in all positions
 *                                         - non null terminated string chksum
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_chksum(int *count_crash, int *count_other_msg, char* argument){
    printf("change chksum\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "chksum");
    for(int index = 0;index<8 && flag!=1; index++){//test all octal values in all positions + non null terminated string chksum
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){
            char nbr = new_gid+'0';
            test_sacha1->chksum[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            strcpy(test_sacha1->version, "00");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05"); 
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }
    test_sacha1->chksum[7] = '\0';
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){// test all values (ascii and non ascii) for mtime[0]
        test_sacha1->chksum[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<7 && flag!=1;index++){ //test a non ascii character for all index of chksum[index]
        test_sacha1->chksum[index]= first;
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


/**
 * fuzz the typeflag field of a header with all values (ascii and non ascii) 
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
void change_typeflag(int *count_crash, int *count_other_msg, char* argument){
    printf("change typeflag\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "typeflag");


    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){
        test_sacha1->typeflag = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05"); 
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    printf("typeflag = %x\n", first);
    free(test_sacha1);    
}


/**
 * fuzz the linkname field of a header with: - all values (ascii and non ascii) for linkname[0]
 *                                           - a non ascii character for all index of linkname[index]
 *                                           - non null terminated string linkname
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_linkname(int *count_crash, int *count_other_msg, char* argument){
    printf("change linkname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "linkname");

    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    //char content[6]="AAAAA";
    //test_sacha1->typeflag = '2';
    //char size_of_content = (char) sizeof(content);
    //strcpy(test_sacha1->size, "05");

    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for linkname[0]
        test_sacha1->linkname[0] = first;
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, NULL); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<100 && flag!=1;index++){ //test a non ascii character for all index of linkname[index] + non null terminated string linkname
        test_sacha1->linkname[index]= first;
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, NULL); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}


/**
 * fuzz the magic field of a header with:  - all values (ascii and non ascii) for magic[0]
 *                                         - a non ascii character for all index of magic[index]
 *                                         - non null terminated string linkname
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_magic(int *count_crash, int *count_other_msg, char* argument){
    printf("change magic\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "magic");
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for magic[0]
        test_sacha1->magic[0] = first;
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<6 && flag!=1;index++){ //test a non ascii character for all index of magic[index] + non null terminated string linkname
        test_sacha1->magic[index]= first;
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}



/**
 * fuzz the version field of a header with: - all values (ascii and non ascii) for version[0]
 *                                          - a non ascii character for all index of version[index]
 *                                          - all octal values in all positions
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_version(int *count_crash, int *count_other_msg, char* argument){
    printf("change version\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "version");
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for version[0]
        test_sacha1->version[0] = first; 
        strcpy(test_sacha1->magic, "ustar");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }


    for(int index = 0;index<2 && flag!=1; index++){
        for(int new_gid=0;new_gid<8 && flag!=1; new_gid++){ //test all octal values in all positions 
            char nbr = new_gid+'0';
            test_sacha1->version[index] = nbr;
            strcpy(test_sacha1->magic, "ustar");
            char content[6]="AAAAA";
            char size_of_content = (char) sizeof(content);
            strcpy(test_sacha1->size, "05");
            int check = calculate_checksum(test_sacha1);
            int ret = write_in_file(test_sacha1, content); 
            flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
        }
    }

    first = 0x80;
    for(int index=1;index<2 && flag!=1;index++){// test a non ascii character for all index of version[index]
        test_sacha1->version[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }

    free(test_sacha1);    
}

/**
 * fuzz the uname field of a header with: - all values (ascii and non ascii) for uname[0]
 *                                        - a non ascii character for all index of uname[index]
 *                                        - non null terminated string uname
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_uname(int *count_crash, int *count_other_msg, char* argument){
    printf("change uname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "uname");
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){//test all values (ascii and non ascii) for uname[0]
        test_sacha1->uname[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<32 && flag!=1;index++){ //test a non ascii character for all index of uname[index] + non null terminated string uname
        test_sacha1->uname[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}


/**
 * fuzz the gname field of a header with: - all values (ascii and non ascii) for gname[0]
 *                                        - a non ascii character for all index of gname[index]
 *                                        - non null terminated string gname
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_gname(int *count_crash, int *count_other_msg, char* argument){
    printf("change gname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "gname");
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for gname[0]
        test_sacha1->gname[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<32 && flag!=1;index++){ //test a non ascii character for all index of gname[index] + non null terminated string gname
        test_sacha1->gname[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}

/**
 * fuzz the gname field of a header (when uname has a non ascii value) with: - all values (ascii and non ascii) for gname[0]
 *                                                                           - a non ascii character for all index of gname[index]
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

void change_uname_gname(int *count_crash, int *count_other_msg, char* argument){
    printf("change uname gname\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "uname_gname");
    unsigned int first;
    test_sacha1->uname[0] = 0xFF;
    for(first= 0x00; first!=0xFF && flag!=1; first++){ //test all values (ascii and non ascii) for gname[0]
        test_sacha1->gname[0] = first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    first = 0x80;
    for(int index=1;index<31 && flag!=1;index++){ //test a non ascii character for all index of gname[index]
        test_sacha1->gname[index]= first;
        strcpy(test_sacha1->magic, "ustar");
        strcpy(test_sacha1->version, "00");
        char content[6]="AAAAA";
        char size_of_content = (char) sizeof(content);
        strcpy(test_sacha1->size, "05");
        int check = calculate_checksum(test_sacha1);
        int ret = write_in_file(test_sacha1, content); 
        flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    }
    free(test_sacha1);    
}



/**
 * test to create a tar file without the two 512-byte blocks filled with binary zeros as an end-of-file marker
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */
int without_zero_at_the_end(int *count_crash, int *count_other_msg, char* argument){
    printf("without zero at the end\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "without_zero");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[6]="AAAAA";
    char *size_of_content = (char*) calloc(12, sizeof(char));
    dec_to_oct(strlen(content), size_of_content);
    strcpy(test_sacha1->size, size_of_content); 
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));
    if (fichier != NULL)
    {
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content, strlen(content),1,fichier);//payload
        int nbr_to_write = 512-(strlen(content));
        char zero[2] = "0";
        fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}

/**
 * fuzz the content of the tar file
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */


void change_content(int *count_crash, int *count_other_msg, char* argument){
    printf("change content\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "content");
    unsigned int first;
    for(first= 0x00; first!=0xFF && flag!=1; first++){
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
    free(test_sacha1);    


}


/**
 * test the extractor when there is a header with no data
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

int no_data(int *count_crash, int *count_other_msg, char* argument){
    printf("no_data\n");
    int flag = 0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "no_data");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    strcpy(test_sacha1->size, "00");
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header

        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//end of file
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        // can not open the .tar file
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}


/**
 * Write two headers with two contents in a tar file
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 * @param test_sacha1 : the first tar header
 * @param content1 : the first content of the tar file
 * @param test_sacha2 : the second tar header
 * @param content2 : the second content of the tar file
 */

int write_two_header(struct tar_t* test_sacha1, char *content1, struct tar_t*test_sacha2, char* content2, int *count_crash, int *count_other_msg, char* argument){

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        char zero[2] = "0";
        
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        if(content1!=NULL){
            fwrite(content1, strlen(content1),1,fichier);//payload
            int nbr_to_write = 512-(strlen(content1)%512);        
            fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        }
        fwrite(test_sacha2, sizeof(struct tar_t), 1, fichier);
         if(content2!=NULL){
            fwrite(content2, strlen(content2),1,fichier);//payload
            int nbr_to_write = 512-(strlen(content2)%512);        
            fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        }        
       
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//end of file
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        // can not open the .tar file
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
    if(check_extractor(count_crash, count_other_msg, argument, test_sacha2->name)==1){
        return 1;
    }
    return check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);

}




/**
 * test the extractor when there are two headers with and without data
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

int two_header(int *count_crash, int *count_other_msg, char* argument){
    printf("two header no_data\n");
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "test");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    char content[6]="AAAAA";
    strcpy(test_sacha1->size, "05");

    struct tar_t* test_sacha2 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha2->name, "test2");
    strcpy(test_sacha2->magic, "ustar");
    strcpy(test_sacha2->version, "00");
    strcpy(test_sacha2->size, "05");

    struct tar_t* test_sacha3 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha3->name, "test3");
    strcpy(test_sacha3->magic, "ustar");
    strcpy(test_sacha3->version, "00");
    strcpy(test_sacha3->size, "00");


    struct tar_t* test_sacha4 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha4->name, "test4");
    strcpy(test_sacha4->magic, "ustar");
    strcpy(test_sacha4->version, "00");
    strcpy(test_sacha4->size, "00");



    int check = calculate_checksum(test_sacha1);
    int check2 = calculate_checksum(test_sacha2);
    int check3 = calculate_checksum(test_sacha3);
    int check4 = calculate_checksum(test_sacha4);

    int flag = 0;

    flag = write_two_header(test_sacha1, content, test_sacha2, content, count_crash, count_other_msg, argument);
    if (flag==1){
        return 1;
    }
    flag = write_two_header(test_sacha3, NULL, test_sacha4, NULL, count_crash, count_other_msg, argument);
    if (flag==1){
        return 1;
    }
    flag = write_two_header(test_sacha1, content, test_sacha4, NULL, count_crash, count_other_msg, argument);
    if (flag==1){
        return 1;
    }
    flag = write_two_header(test_sacha3, NULL, test_sacha1, content, count_crash, count_other_msg, argument);


    free(test_sacha1);
    free(test_sacha2);
    free(test_sacha3);
    free(test_sacha4);
    return flag;
}





/**
 * test the extractor when there are a lot of header with data
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

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
    strcpy(test_sacha1->size, size_of_content);
    int check = calculate_checksum(test_sacha1);


    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        for(int rep=0; rep<1000; rep++){
            fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
            fwrite(content, strlen(content),1,fichier);//payload
            int nbr_to_write = 512-(strlen(content));
            char zero[2] = "0";
            int test_fwrite = fwrite(zero, sizeof(char), nbr_to_write, fichier);       

        }
        
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//end of file
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        //can not open the .tar file
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}


/**
 * test the extractor when there informations after the two 512-byte blocks filled with binary zeros as an end-of-file marker
 * @param count_crash: pointer that counts the number of crash cases
 * @param count_other_msg : pointer that counts the non crashes message
 * @param arguement : the path to the executable
 */

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
    strcpy(test_sacha1->size, size_of_content);
    int check = calculate_checksum(test_sacha1);

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        fwrite(content, strlen(content),1,fichier);//payload
        int nbr_to_write = 512-(strlen(content));
        char zero[2] = "0";
        fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//end of file
        char write_end[10]="after end";
        fwrite(write_end, 9*sizeof(char), 1, fichier);
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        // can not open the .tar file
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);
    free(test_sacha1);
    return 0;
}

//only for testing purpose
int test(int *count_crash, int *count_other_msg, char* argument){
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "test/");
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    strcpy(test_sacha1->size, "00");
    test_sacha1->typeflag = '5';
    char *content1 = NULL;

    struct tar_t* test_sacha2 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha2->name, "symlink");
    strcpy(test_sacha2->magic, "ustar");
    strcpy(test_sacha2->version, "00");
    test_sacha2->typeflag = '2';
    char *content2 = NULL;
    strcpy(test_sacha2->size, "00");
    strcpy(test_sacha2->linkname, "a");
    //strcpy(test_sacha2->linkname, "test");

    int check = calculate_checksum(test_sacha1);
    int check2 = calculate_checksum(test_sacha2);
    int flag = 0;

    FILE* fichier = NULL;
    fichier = fopen("archive.tar", "w+");
    char* end_of_archive = (char *) calloc(1024, sizeof(char));

    if (fichier != NULL)
    {
        char zero[2] = "0";
        
        fwrite(test_sacha1, sizeof(struct tar_t), 1, fichier);//header
        if(content1!=NULL){
            fwrite(content1, strlen(content1),1,fichier);//payload
            int nbr_to_write = 512-(strlen(content1)%512);        
            fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        }
        fwrite(test_sacha2, sizeof(struct tar_t), 1, fichier);
         if(content2!=NULL){
            fwrite(content2, strlen(content2),1,fichier);//payload
            int nbr_to_write = 512-(strlen(content2)%512);        
            fwrite(zero, sizeof(char), nbr_to_write, fichier); 
        }        
       
        fwrite(end_of_archive, 1024*sizeof(char), 1, fichier);//end of file
        if(fwrite==0){
            printf("error writting file \n");
        }

        fclose(fichier);
    }
    else
    {
        // can not open the .tar file
        printf("Impossible d'ouvrir le fichier archive.tar \n");
    }
   // check_extractor(count_crash, count_other_msg, argument, test_sacha2->name);
        
    



    char cmd[100];
    strcpy(cmd, argument);
    strcat(cmd, " archive.tar ");
    strcat(cmd, test_sacha2->name);

    char buf[33];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return 3;
    }

    if(fgets(buf, 33, fp) == NULL) {
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
        }
        return 2;
    } else {
        printf("Crash message\n");
        *count_crash = *count_crash+1;
        char dest[20]=" success_";
        char to_add[5];
        char cmd_cp[50] = "cp ";
        sprintf(to_add, "%d", *count_crash);
        strcat(dest, to_add);
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

   free(test_sacha1);
   free(test_sacha2);
    return 0;
}

//only for testing purpose
int test2(int *count_crash, int *count_other_msg, char* argument){
    printf("test2\n");
    int flag=0;
    struct tar_t* test_sacha1 = (struct tar_t*) calloc(1,sizeof(struct tar_t));
    strcpy(test_sacha1->name, "directory/");
    unsigned int first;
    
    strcpy(test_sacha1->magic, "ustar");
    strcpy(test_sacha1->version, "00");
    //char content[6]="AAAAA";
    //char size_of_content = (char) sizeof(content);
    strcpy(test_sacha1->size, "01");
    test_sacha1->typeflag = '5';
    int check = calculate_checksum(test_sacha1);
    int ret = write_in_file(test_sacha1, NULL); 
    flag = check_extractor(count_crash, count_other_msg, argument, test_sacha1->name);

}


int main(int argc, char* argv[])
{
    if (argc < 2)
        return -1;
    int count_crash = 0;
    int count_other_msg = 0;





    change_uname(&count_crash, &count_other_msg, argv[1]);
    change_name(&count_crash, &count_other_msg, argv[1]);
    change_gname(&count_crash, &count_other_msg, argv[1]);
    change_uid_gid(&count_crash, &count_other_msg, argv[1]); //ok
    two_header(&count_crash, &count_other_msg, argv[1]); //ok
    change_typeflag(&count_crash, &count_other_msg, argv[1]);//ok
    change_uid(&count_crash, &count_other_msg, argv[1]);//ok
    change_linkname(&count_crash, &count_other_msg, argv[1]);
    change_mode(&count_crash, &count_other_msg, argv[1]);
    change_gid(&count_crash, &count_other_msg, argv[1]);
    change_size(&count_crash, &count_other_msg, argv[1]); //ok
    without_zero_at_the_end(&count_crash, &count_other_msg, argv[1]);//ok
    change_uname_gname(&count_crash, &count_other_msg, argv[1]);
    change_mtime(&count_crash, &count_other_msg, argv[1]);
    change_chksum(&count_crash, &count_other_msg, argv[1]);
    change_magic(&count_crash, &count_other_msg, argv[1]);
    change_version(&count_crash, &count_other_msg, argv[1]);
    change_content(&count_crash, &count_other_msg, argv[1]);
    no_data(&count_crash, &count_other_msg, argv[1]);
    lot_of_header(&count_crash, &count_other_msg, argv[1]);
    write_after_end(&count_crash, &count_other_msg, argv[1]);

    //test(&count_crash, &count_other_msg, argv[1]);
    //test2(&count_crash, &count_other_msg, argv[1]);

    printf ("number of crashes = %d\n", count_crash);
    printf ("number of other msg = %d\n", count_other_msg);
    return 0;
}




