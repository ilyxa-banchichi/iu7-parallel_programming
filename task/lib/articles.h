#ifndef CUTTING_PROGRAM_ARTICLES
#define CUTTING_PROGRAM_ARTICLES

#include <stdlib.h>
#include "types.h"

#define ARTICLES_COUNT 9
#define ITEMS_COUNT_MULTIPLIER 500

static void init_articles(Article *articles)
{
    articles[0].index = 0;
    articles[0].sheet_width = 2000;
    articles[0].sheet_height = 1000;
    articles[0].item_count = 2;
    articles[0].items = (Item *)malloc(articles[0].item_count * sizeof(Item));
    articles[0].items[0] = (Item){500, 500, 10};
    articles[0].items[1] = (Item){300, 400, 5};

    articles[1].index = 1;
    articles[1].sheet_width = 1500;
    articles[1].sheet_height = 1500;
    articles[1].item_count = 3;
    articles[1].items = (Item *)malloc(articles[1].item_count * sizeof(Item));
    articles[1].items[0] = (Item){200, 300, 20};
    articles[1].items[1] = (Item){100, 100, 30};
    articles[1].items[2] = (Item){350, 700, 15};

    articles[2].index = 2;
    articles[2].sheet_width = 3000;
    articles[2].sheet_height = 1500;
    articles[2].item_count = 4;
    articles[2].items = (Item *)malloc(articles[2].item_count * sizeof(Item));
    articles[2].items[0] = (Item){600, 800, 8};
    articles[2].items[1] = (Item){500, 500, 12};
    articles[2].items[2] = (Item){1000, 400, 6};
    articles[2].items[3] = (Item){300, 300, 20};

    articles[3].index = 3;
    articles[3].sheet_width = 1200;
    articles[3].sheet_height = 2400;
    articles[3].item_count = 3;
    articles[3].items = (Item *)malloc(articles[3].item_count * sizeof(Item));
    articles[3].items[0] = (Item){400, 600, 10};
    articles[3].items[1] = (Item){350, 900, 5};
    articles[3].items[2] = (Item){250, 250, 16};

    articles[4].index = 4;
    articles[4].sheet_width = 2500;
    articles[4].sheet_height = 1000;
    articles[4].item_count = 5;
    articles[4].items = (Item *)malloc(articles[4].item_count * sizeof(Item));
    articles[4].items[0] = (Item){400, 400, 10};
    articles[4].items[1] = (Item){500, 300, 20};
    articles[4].items[2] = (Item){300, 700, 7};
    articles[4].items[3] = (Item){200, 200, 30};
    articles[4].items[4] = (Item){800, 500, 4};

    articles[5].index = 5;
    articles[5].sheet_width = 2500;
    articles[5].sheet_height = 1000;
    articles[5].item_count = 5;
    articles[5].items = (Item *)malloc(articles[5].item_count * sizeof(Item));
    articles[5].items[0] = (Item){400, 400, 10};
    articles[5].items[1] = (Item){500, 300, 20};
    articles[5].items[2] = (Item){300, 700, 7};
    articles[5].items[3] = (Item){200, 200, 30};
    articles[5].items[4] = (Item){800, 500, 4};

    articles[6].index = 6;
    articles[6].sheet_width = 2200;
    articles[6].sheet_height = 2200;
    articles[6].item_count = 6;
    articles[6].items = (Item *)malloc(articles[6].item_count * sizeof(Item));
    articles[6].items[0] = (Item){700, 700, 5};
    articles[6].items[1] = (Item){500, 500, 10};
    articles[6].items[2] = (Item){300, 900, 6};
    articles[6].items[3] = (Item){400, 300, 20};
    articles[6].items[4] = (Item){900, 900, 3};
    articles[6].items[5] = (Item){200, 200, 40};

    articles[7].index = 7;
    articles[7].sheet_width = 1000;
    articles[7].sheet_height = 3000;
    articles[7].item_count = 4;
    articles[7].items = (Item *)malloc(articles[7].item_count * sizeof(Item));
    articles[7].items[0] = (Item){200, 800, 12};
    articles[7].items[1] = (Item){400, 600, 7};
    articles[7].items[2] = (Item){300, 300, 10};
    articles[7].items[3] = (Item){500, 1000, 4};

    articles[8].index = 8;
    articles[8].sheet_width = 2600;
    articles[8].sheet_height = 1400;
    articles[8].item_count = 5;
    articles[8].items = (Item *)malloc(articles[8].item_count * sizeof(Item));
    articles[8].items[0] = (Item){600, 600, 10};
    articles[8].items[1] = (Item){900, 300, 6};
    articles[8].items[2] = (Item){400, 800, 8};
    articles[8].items[3] = (Item){200, 500, 15};
    articles[8].items[4] = (Item){700, 700, 5};

    for (int i = 0; i < ARTICLES_COUNT; i++)
        for (int j = 0; j < articles[i].item_count; j++)
            articles[i].items[j].count *= ITEMS_COUNT_MULTIPLIER;
}

#endif // CUTTING_PROGRAM_ARTICLES