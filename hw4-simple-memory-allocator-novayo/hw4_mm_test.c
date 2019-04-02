#include "lib/hw_malloc.h"
#include "hw4_mm_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

int main(int argc, char *argv[])
{
    char input[20];
    while (!feof(stdin)) {
        if (fgets(input, 20, stdin) != NULL) {
            char *pch;
            pch = strtok(input, " ");
            if (!strcmp(pch, "alloc")) {
                pch = strtok(NULL, " ");
                size_t need = atoi(pch);
                void *ptr = hw_malloc(need);
                printf("%012p\n", ptr);
            } else if (!strcmp(pch, "free")) {
                pch = strtok(NULL, " ");
                //printf("pch = %s\n", pch);
                int i = 0;
                sscanf(pch, "%x", &i);
                //printf("i = %x\n", i);
                void *mem = (void *)(uintptr_t)i;
                printf("%s\n", hw_free(mem) == True ? "success" : "fail");
            } else if (!strcmp(pch, "print")) {
                pch = strtok(NULL, " ");
                if (pch[0] == 'b') {
                    pch = strtok(pch, "[");
                    pch = strtok(NULL, "[");
                    pch = strtok(pch, "]");
                    int i = atoi(pch);
                    show_bin(i);
                    //show_all_bin();
                }
            }
        }
    }
    return 0;
}
