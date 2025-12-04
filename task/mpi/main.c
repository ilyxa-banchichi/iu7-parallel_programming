#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#define INITIAL_MAX_SHEETS 500
#define ARTICLES_COUNT 9

typedef struct
{
    int x, y;
    int width, height;
} Rectangle;

typedef struct
{
    int width, height;
    int count;
} Item;

typedef struct
{
    int index;
    int sheet_width, sheet_height;
    Item *items;
    int item_count;
} Article;

typedef struct
{
    Rectangle *placements;
    int placement_count;
    int used_area;
} CutSheet;

typedef struct
{
    int index;
    CutSheet *sheets;
    int sheet_count;
    int total_waste;
} ArticleResult;

Rectangle create_rectangle(int x, int y, int width, int height)
{
    Rectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    return rect;
}

int area(Rectangle rect)
{
    return rect.width * rect.height;
}

int can_place(Rectangle rect, int item_width, int item_height, int allow_rotation)
{
    if ((item_width <= rect.width && item_height <= rect.height) ||
        (allow_rotation && item_height <= rect.width && item_width <= rect.height))
    {
        return 1;
    }
    return 0;
}

Rectangle place_item(Rectangle rect, int item_width, int item_height, int allow_rotation,
                     int *actual_width, int *actual_height)
{
    // Пробуем разместить без поворота
    if (item_width <= rect.width && item_height <= rect.height)
    {
        *actual_width = item_width;
        *actual_height = item_height;
        return create_rectangle(rect.x, rect.y, item_width, item_height);
    }
    // Пробуем с поворотом
    if (allow_rotation && item_height <= rect.width && item_width <= rect.height)
    {
        *actual_width = item_height;
        *actual_height = item_width;
        return create_rectangle(rect.x, rect.y, item_height, item_width);
    }
    *actual_width = 0;
    *actual_height = 0;
    return create_rectangle(0, 0, 0, 0);
}

void split_rectangle(Rectangle free_rect, Rectangle placed_rect,
                     Rectangle *new_rects, int *new_count)
{
    *new_count = 0;

    // Горизонтальное разделение (остаток справа)
    if (free_rect.width > placed_rect.width)
    {
        new_rects[*new_count] = create_rectangle(
            free_rect.x + placed_rect.width,
            free_rect.y,
            free_rect.width - placed_rect.width,
            placed_rect.height);
        (*new_count)++;
    }

    // Вертикальное разделение (остаток сверху)
    if (free_rect.height > placed_rect.height)
    {
        new_rects[*new_count] = create_rectangle(
            free_rect.x,
            free_rect.y + placed_rect.height,
            free_rect.width,
            free_rect.height - placed_rect.height);
        (*new_count)++;
    }
}

int compare_items(const Item *a, const Item *b)
{
    int areaA = a->width * a->height;
    int areaB = b->width * b->height;
    return areaB - areaA;
}

ArticleResult cut_article(Article article)
{
    ArticleResult result;
    result.sheets = NULL;
    result.sheet_count = 0;
    result.total_waste = 0;

    int total_items = 0;
    for (int i = 0; i < article.item_count; i++)
        total_items += article.items[i].count;

    Rectangle *items_to_cut = (Rectangle *)malloc(total_items * sizeof(Rectangle));
    int item_index = 0;

    for (int i = 0; i < article.item_count; i++)
    {
        Item item = article.items[i];
        for (int j = 0; j < article.items[i].count; j++)
        {
            items_to_cut[item_index] = create_rectangle(0, 0, item.width, item.height);
            item_index++;
        }
    }

    for (int i = 0; i < total_items - 1; i++)
    {
        for (int j = i + 1; j < total_items; j++)
        {
            int area_i = area(items_to_cut[i]);
            int area_j = area(items_to_cut[j]);
            if (area_i < area_j)
            {
                Rectangle temp = items_to_cut[i];
                items_to_cut[i] = items_to_cut[j];
                items_to_cut[j] = temp;
            }
        }
    }

    int remaining_items = total_items;
    int max_sheets = INITIAL_MAX_SHEETS;
    result.sheets = (CutSheet *)malloc(max_sheets * sizeof(CutSheet));

    while (remaining_items > 0)
    {
        if (result.sheet_count >= max_sheets)
        {
            max_sheets *= 2;
            result.sheets = (CutSheet *)realloc(result.sheets, max_sheets * sizeof(CutSheet));
        }

        CutSheet *current_sheet = &result.sheets[result.sheet_count];
        current_sheet->placements = (Rectangle *)malloc(remaining_items * sizeof(Rectangle));
        current_sheet->placement_count = 0;
        current_sheet->used_area = 0;

        Rectangle *free_rects = (Rectangle *)malloc(remaining_items * sizeof(Rectangle));
        int free_count = 1;
        free_rects[0] = create_rectangle(0, 0, article.sheet_width, article.sheet_height);

        for (int i = 0; i < total_items; i++)
        {
            if (items_to_cut[i].width == 0) // метка, что заготовка уже была размещена
                continue;

            int best_free_index = -1;
            int best_waste = article.sheet_width * article.sheet_height + 1;
            Rectangle best_placement;
            int best_actual_width, best_actual_height;

            for (int j = 0; j < free_count; j++)
            {
                int actual_width, actual_height;
                Rectangle placement = place_item(free_rects[j],
                                                 items_to_cut[i].width, items_to_cut[i].height, 1,
                                                 &actual_width, &actual_height);

                if (actual_width > 0)
                {
                    int waste = area(free_rects[j]) - area(placement);
                    if (waste < best_waste)
                    {
                        best_waste = waste;
                        best_free_index = j;
                        best_placement = placement;
                        best_actual_width = actual_width;
                        best_actual_height = actual_height;
                    }
                }
            }

            if (best_free_index != -1)
            {
                current_sheet->placements[current_sheet->placement_count] = best_placement;
                current_sheet->placement_count++;
                current_sheet->used_area += area(best_placement);

                Rectangle new_rects[2];
                int new_count;
                split_rectangle(free_rects[best_free_index], best_placement, new_rects, &new_count);

                free_rects[best_free_index] = free_rects[free_count - 1];
                free_count--;

                for (int k = 0; k < new_count; k++)
                {
                    free_rects[free_count] = new_rects[k];
                    free_count++;
                }

                items_to_cut[i].width = 0;
                remaining_items--;
            }
        }

        int sheet_waste = article.sheet_width * article.sheet_height - current_sheet->used_area;
        result.total_waste += sheet_waste;

        free(free_rects);
        result.sheet_count++;
    }

    free(items_to_cut);
    return result;
}

void write_results_to_file(Article *articles, ArticleResult *results, int article_count, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Ошибка открытия файла для записи!\n");
        return;
    }

    for (int art_idx = 0; art_idx < article_count; art_idx++)
    {
        fprintf(file, "Артикул %d (лист %dx%d):\n",
                art_idx + 1, articles[art_idx].sheet_width, articles[art_idx].sheet_height);

        for (int sheet_idx = 0; sheet_idx < results[art_idx].sheet_count; sheet_idx++)
        {
            fprintf(file, "  Лист %d:\n", sheet_idx + 1);
            CutSheet sheet = results[art_idx].sheets[sheet_idx];

            for (int place_idx = 0; place_idx < sheet.placement_count; place_idx++)
            {
                Rectangle rect = sheet.placements[place_idx];
                fprintf(file, "    Заготовка: x=%d, y=%d, w=%d, h=%d\n",
                        rect.x, rect.y, rect.width, rect.height);
            }
            fprintf(file, "    Использовано площади: %d, Отходы: %d\n",
                    sheet.used_area,
                    articles[art_idx].sheet_width * articles[art_idx].sheet_height - sheet.used_area);
        }
        fprintf(file, "Всего листов: %d, Общие отходы: %d\n\n",
                results[art_idx].sheet_count, results[art_idx].total_waste);
    }

    fclose(file);
}

void free_results(ArticleResult *results, int article_count)
{
    for (int i = 0; i < article_count; i++)
    {
        for (int j = 0; j < results[i].sheet_count; j++)
            free(results[i].sheets[j].placements);
        free(results[i].sheets);
    }
}

void init_articles(Article *articles)
{
    articles[0].index = 0;
    articles[0].sheet_width = 2000;
    articles[0].sheet_height = 1000;
    articles[0].item_count = 2;
    articles[0].items = malloc(articles[0].item_count * sizeof(Item));
    articles[0].items[0] = (Item){500, 500, 10};
    articles[0].items[1] = (Item){300, 400, 5};

    articles[1].index = 1;
    articles[1].sheet_width = 1500;
    articles[1].sheet_height = 1500;
    articles[1].item_count = 3;
    articles[1].items = malloc(articles[1].item_count * sizeof(Item));
    articles[1].items[0] = (Item){200, 300, 20};
    articles[1].items[1] = (Item){100, 100, 30};
    articles[1].items[2] = (Item){350, 700, 15};

    articles[2].index = 2;
    articles[2].sheet_width = 3000;
    articles[2].sheet_height = 1500;
    articles[2].item_count = 4;
    articles[2].items = malloc(articles[2].item_count * sizeof(Item));
    articles[2].items[0] = (Item){600, 800, 8};
    articles[2].items[1] = (Item){500, 500, 12};
    articles[2].items[2] = (Item){1000, 400, 6};
    articles[2].items[3] = (Item){300, 300, 20};

    articles[3].index = 3;
    articles[3].sheet_width = 1200;
    articles[3].sheet_height = 2400;
    articles[3].item_count = 3;
    articles[3].items = malloc(articles[3].item_count * sizeof(Item));
    articles[3].items[0] = (Item){400, 600, 10};
    articles[3].items[1] = (Item){350, 900, 5};
    articles[3].items[2] = (Item){250, 250, 16};

    articles[4].index = 4;
    articles[4].sheet_width = 2500;
    articles[4].sheet_height = 1000;
    articles[4].item_count = 5;
    articles[4].items = malloc(articles[4].item_count * sizeof(Item));
    articles[4].items[0] = (Item){400, 400, 10};
    articles[4].items[1] = (Item){500, 300, 20};
    articles[4].items[2] = (Item){300, 700, 7};
    articles[4].items[3] = (Item){200, 200, 30};
    articles[4].items[4] = (Item){800, 500, 4};

    articles[5].index = 5;
    articles[5].sheet_width = 1800;
    articles[5].sheet_height = 900;
    articles[5].item_count = 4;
    articles[5].items = malloc(articles[5].item_count * sizeof(Item));
    articles[5].items[0] = (Item){300, 300, 15};
    articles[5].items[1] = (Item){450, 600, 10};
    articles[5].items[2] = (Item){600, 400, 8};
    articles[5].items[3] = (Item){200, 500, 12};

    articles[6].index = 6;
    articles[6].sheet_width = 2200;
    articles[6].sheet_height = 2200;
    articles[6].item_count = 6;
    articles[6].items = malloc(articles[6].item_count * sizeof(Item));
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
    articles[7].items = malloc(articles[7].item_count * sizeof(Item));
    articles[7].items[0] = (Item){200, 800, 12};
    articles[7].items[1] = (Item){400, 600, 7};
    articles[7].items[2] = (Item){300, 300, 10};
    articles[7].items[3] = (Item){500, 1000, 4};

    articles[8].index = 8;
    articles[8].sheet_width = 2600;
    articles[8].sheet_height = 1400;
    articles[8].item_count = 5;
    articles[8].items = malloc(articles[8].item_count * sizeof(Item));
    articles[8].items[0] = (Item){600, 600, 10};
    articles[8].items[1] = (Item){900, 300, 6};
    articles[8].items[2] = (Item){400, 800, 8};
    articles[8].items[3] = (Item){200, 500, 15};
    articles[8].items[4] = (Item){700, 700, 5};
}

void init_articles_big(Article *articles)
{
    articles[0].index = 0;
    articles[0].sheet_width = 2000;
    articles[0].sheet_height = 1000;
    articles[0].item_count = 2;
    articles[0].items = malloc(articles[0].item_count * sizeof(Item));
    articles[0].items[0] = (Item){500, 500, 1000};
    articles[0].items[1] = (Item){300, 400, 500};

    articles[1].index = 1;
    articles[1].sheet_width = 1500;
    articles[1].sheet_height = 1500;
    articles[1].item_count = 3;
    articles[1].items = malloc(articles[1].item_count * sizeof(Item));
    articles[1].items[0] = (Item){200, 300, 2000};
    articles[1].items[1] = (Item){100, 100, 3000};
    articles[1].items[2] = (Item){350, 700, 1500};

    articles[2].index = 2;
    articles[2].sheet_width = 3000;
    articles[2].sheet_height = 1500;
    articles[2].item_count = 4;
    articles[2].items = malloc(articles[2].item_count * sizeof(Item));
    articles[2].items[0] = (Item){600, 800, 800};
    articles[2].items[1] = (Item){500, 500, 1200};
    articles[2].items[2] = (Item){1000, 400, 600};
    articles[2].items[3] = (Item){300, 300, 2000};

    articles[3].index = 3;
    articles[3].sheet_width = 1200;
    articles[3].sheet_height = 2400;
    articles[3].item_count = 3;
    articles[3].items = malloc(articles[3].item_count * sizeof(Item));
    articles[3].items[0] = (Item){400, 600, 1000};
    articles[3].items[1] = (Item){350, 900, 500};
    articles[3].items[2] = (Item){250, 250, 1600};

    articles[4].index = 4;
    articles[4].sheet_width = 2500;
    articles[4].sheet_height = 1000;
    articles[4].item_count = 5;
    articles[4].items = malloc(articles[4].item_count * sizeof(Item));
    articles[4].items[0] = (Item){400, 400, 1000};
    articles[4].items[1] = (Item){500, 300, 2000};
    articles[4].items[2] = (Item){300, 700, 700};
    articles[4].items[3] = (Item){200, 200, 3000};
    articles[4].items[4] = (Item){800, 500, 400};

    articles[5].index = 5;
    articles[5].sheet_width = 2500;
    articles[5].sheet_height = 1000;
    articles[5].item_count = 5;
    articles[5].items = malloc(articles[5].item_count * sizeof(Item));
    articles[5].items[0] = (Item){400, 400, 1000};
    articles[5].items[1] = (Item){500, 300, 2000};
    articles[5].items[2] = (Item){300, 700, 700};
    articles[5].items[3] = (Item){200, 200, 3000};
    articles[5].items[4] = (Item){800, 500, 400};

    articles[6].index = 6;
    articles[6].sheet_width = 2200;
    articles[6].sheet_height = 2200;
    articles[6].item_count = 6;
    articles[6].items = malloc(articles[6].item_count * sizeof(Item));
    articles[6].items[0] = (Item){700, 700, 500};
    articles[6].items[1] = (Item){500, 500, 1000};
    articles[6].items[2] = (Item){300, 900, 600};
    articles[6].items[3] = (Item){400, 300, 2000};
    articles[6].items[4] = (Item){900, 900, 300};
    articles[6].items[5] = (Item){200, 200, 4000};

    articles[7].index = 7;
    articles[7].sheet_width = 1000;
    articles[7].sheet_height = 3000;
    articles[7].item_count = 4;
    articles[7].items = malloc(articles[7].item_count * sizeof(Item));
    articles[7].items[0] = (Item){200, 800, 1200};
    articles[7].items[1] = (Item){400, 600, 700};
    articles[7].items[2] = (Item){300, 300, 1000};
    articles[7].items[3] = (Item){500, 1000, 400};

    articles[8].index = 8;
    articles[8].sheet_width = 2600;
    articles[8].sheet_height = 1400;
    articles[8].item_count = 5;
    articles[8].items = malloc(articles[8].item_count * sizeof(Item));
    articles[8].items[0] = (Item){600, 600, 1000};
    articles[8].items[1] = (Item){900, 300, 600};
    articles[8].items[2] = (Item){400, 800, 800};
    articles[8].items[3] = (Item){200, 500, 1500};
    articles[8].items[4] = (Item){700, 700, 500};
}

typedef struct
{
    int index;
    int sheet_width;
    int sheet_height;
    int item_count;
} ArticleHeader;

void MPI_SendArticle(Article a, int p)
{
    ArticleHeader hdr;
    hdr.index = a.index;
    hdr.sheet_width = a.sheet_width;
    hdr.sheet_height = a.sheet_height;
    hdr.item_count = a.item_count;
    MPI_Send(&hdr, sizeof(ArticleHeader), MPI_BYTE, p, 0, MPI_COMM_WORLD);
    if (hdr.item_count > 0)
        MPI_Send(a.items, hdr.item_count * sizeof(Item), MPI_BYTE, p, 0, MPI_COMM_WORLD);
}

int MPI_ReceiveArticle(Article *task)
{
    ArticleHeader hdr;
    MPI_Status st;
    MPI_Recv(&hdr, sizeof(ArticleHeader), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &st);

    if (st.MPI_TAG == 1)
        return 1;

    task->index = hdr.index;
    task->sheet_width = hdr.sheet_width;
    task->sheet_height = hdr.sheet_height;
    task->item_count = hdr.item_count;
    task->items = NULL;

    if (task->item_count > 0)
    {
        task->items = malloc(task->item_count * sizeof(Item));
        MPI_Recv(task->items, task->item_count * sizeof(Item), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    return 0;
}

MPI_Status MPI_ReceiveArticleResult(ArticleResult *results)
{
    MPI_Status st;
    int idx, sheet_count;
    MPI_Recv(&idx, 1, MPI_INT, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, &st);
    MPI_Recv(&sheet_count, 1, MPI_INT, st.MPI_SOURCE, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    results[idx].index = idx;
    results[idx].sheet_count = sheet_count;
    results[idx].sheets = malloc(sheet_count * sizeof(CutSheet));

    for (int s = 0; s < sheet_count; ++s)
    {
        int placement_count, used_area;
        MPI_Recv(&placement_count, 1, MPI_INT, st.MPI_SOURCE, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&used_area, 1, MPI_INT, st.MPI_SOURCE, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        results[idx].sheets[s].placement_count = placement_count;
        results[idx].sheets[s].used_area = used_area;

        results[idx].sheets[s].placements = NULL;
        if (placement_count > 0)
        {
            results[idx].sheets[s].placements = malloc(placement_count * sizeof(Rectangle));
            MPI_Recv(results[idx].sheets[s].placements,
                     placement_count * sizeof(Rectangle),
                     MPI_BYTE, st.MPI_SOURCE, 12, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    MPI_Recv(&results[idx].total_waste, 1, MPI_INT, st.MPI_SOURCE, 13, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return st;
}

void MPI_SendArticleResult(ArticleResult res)
{
    MPI_Send(&res.index, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
    MPI_Send(&res.sheet_count, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);

    for (int s = 0; s < res.sheet_count; ++s)
    {
        int placement_count = res.sheets[s].placement_count;
        int used_area = res.sheets[s].used_area;

        MPI_Send(&placement_count, 1, MPI_INT, 0, 11, MPI_COMM_WORLD);
        MPI_Send(&used_area, 1, MPI_INT, 0, 11, MPI_COMM_WORLD);

        if (placement_count > 0)
        {
            MPI_Send(res.sheets[s].placements,
                     placement_count * sizeof(Rectangle),
                     MPI_BYTE, 0, 12, MPI_COMM_WORLD);
        }
    }

    MPI_Send(&res.total_waste, 1, MPI_INT, 0, 13, MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (rank == 0)
    {
        printf("Начало параллельного раскроя MPI (%d процессов)...\n", nprocs);

        Article articles[ARTICLES_COUNT];
        init_articles_big(articles);
        ArticleResult *results = (ArticleResult *)malloc(ARTICLES_COUNT * sizeof(ArticleResult));

        clock_t start = clock();

        int next_article = 0;
        for (int p = 1; p < nprocs && next_article < ARTICLES_COUNT; p++)
        {
            MPI_SendArticle(articles[next_article], p);
            next_article++;
        }

        int finished = 0;
        while (finished < ARTICLES_COUNT)
        {
            MPI_Status st = MPI_ReceiveArticleResult(results);
            int worker = st.MPI_SOURCE;
            finished++;

            if (next_article < ARTICLES_COUNT)
            {
                MPI_SendArticle(articles[next_article], worker);
                next_article++;
            }
            else
            {
                MPI_Send(NULL, 0, MPI_BYTE, worker, 1, MPI_COMM_WORLD);
            }
        }

        clock_t end = clock();

        double total_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("MPI total time: %f sec\n", total_time);

        write_results_to_file(articles, results, ARTICLES_COUNT, "cutting_results.txt");
        printf("Результаты сохранены в cutting_results.txt\n");

        free_results(results, ARTICLES_COUNT);
        free(results);

        for (int i = 0; i < ARTICLES_COUNT; i++)
            free(articles[i].items);
    }
    else
    {
        while (1)
        {
            Article task;
            int status = MPI_ReceiveArticle(&task);
            if (status == 1)
                break;

            printf("Процесс %d начал раскрой артилка %d\n", rank, task.index);

            ArticleResult res = cut_article(task);
            res.index = task.index;

            MPI_SendArticleResult(res);

            free(task.items);
            free_results(&res, 1);
        }
    }

    MPI_Finalize();
    return 0;
}