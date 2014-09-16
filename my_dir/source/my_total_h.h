#include <stdio.h>
#ifndef MY_TOTAL_H_FILE
#define MY_TOTAL_H_FILE

/* my_version_check.c */
int is_need_update_ver(const char* verFile);
int update_txt_file_ver( const char* gitBranch, \
        const char* baseVer, const char* writeFile);


#endif
