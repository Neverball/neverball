#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "common.h"
#include "demo.h"
#include "demo_dir.h"
#include "dir.h"

/*---------------------------------------------------------------------------*/

static int scan_item(struct dir_item *item)
{
    FILE *fp;
    struct demo *d;
    int keep = 0;

    if ((fp = fopen(item->path, FMODE_RB)))
    {
        d = malloc(sizeof (struct demo));

        if (demo_header_read(fp, d))
        {
            strncpy(d->filename, item->path, MAXSTR);
            strncpy(d->name, base_name(d->filename, REPLAY_EXT), PATHMAX);
            d->name[PATHMAX - 1] = '\0';

            item->data = d;
            keep = 1;
        }
        else free(d);

        fclose(fp);
    }

    return keep;
}

Array demo_dir_scan(const char *path)
{
    return dir_scan(path, scan_item);
}

void demo_dir_free(Array items)
{
    struct dir_item *item;
    int i;

    for (i = 0; i < array_len(items); i++)
    {
        item = array_get(items, i);

        free(item->data);
        item->data = NULL;
    }

    dir_free(items);
}

/*---------------------------------------------------------------------------*/
