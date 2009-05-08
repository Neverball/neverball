#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "common.h"
#include "demo.h"
#include "demo_dir.h"
#include "dir.h"

/*---------------------------------------------------------------------------*/

static Array items;

static void free_items(void)
{
    if (items)
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
        items = NULL;
    }
}

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

int demo_scan(void)
{
    free_items();

    items = dir_scan(config_user(""), scan_item);

    return array_len(items);
}

const char *demo_pick(void)
{
    struct dir_item *item;
    struct demo *d;

    demo_scan();

    return (item = array_rnd(items)) && (d = item->data) ? d->filename : NULL;
}

const struct demo *demo_get(int i)
{
    return DIR_ITEM_GET(items, i)->data;
}

/*---------------------------------------------------------------------------*/

