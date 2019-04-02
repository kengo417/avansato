#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define DEFMAX  10
#define BUFLEN  30

typedef struct
{
  int  no;
  char *rec;
} ST_LINE;

int main(int argc,  char **argv)
{
    ST_LINE     *line;
    int         i;
    int         j;

    line = (ST_LINE*)calloc(sizeof(ST_LINE), DEFMAX);
    if(line == NULL)
    {
        fprintf(stderr, "alloc error.\n");
        return -1;
    }
    else
    {
        for(j = 0; j < (DEFMAX); j++)
        {
            (line + j)->rec = malloc(BUFLEN);
        }
    }

    for(j = 0; j < (DEFMAX);)
    {
        *((line + j)->rec + 0) = 0x00;

        (line + j)->no = (j + 1);
        sprintf((line + j)->rec, "line(%d) = %x", j, (line + j)->rec);

        j++;
        if(j >= 10) break;

        *((line + j)->rec + 0) = 0x00;
    }

    for(j = 0; j < (DEFMAX) && *((line + j)->rec + 0); j++)
    {
        printf("%03d : [%s]\n", (line + j)->no, (line + j)->rec);
    }

    for(j = 0; j < DEFMAX; j++)
    {
        free((line + j)->rec);
    }
    free(line);

    return 0;
}
