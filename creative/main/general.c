#include "general.h"

/****************************
 * General purpose functions.
 ****************************/


char *strconcat(char *a, char *b) {
    size_t sumlen = strlen(a)+strlen(b);
    char *concat = malloc(sumlen + 2);
    bzero(concat, sumlen + 2);
    snprintf(concat, sumlen + 1, "%s %s", a, b);
    return concat;
}



char *string_copy(char *str) {
    int len = strlen(str);

    char *new_str = (char *)malloc(len + 1);
    if (new_str == NULL) {
        printf("Malloc Error: string_copy");
        exit(1);
    }
    bzero(new_str, len + 1);

    for (int i = 0; str[i]; ++i)
        new_str[i] = str[i];
    return new_str;
}
