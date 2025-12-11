#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define INITIAL_MAX_SHEETS 10
#define ARTICLES_COUNT 1
#define ITEMS_COUNT_MULTIPLIER 500

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
    articles[0].sheet_width = 2000;
    articles[0].sheet_height = 1000;
    articles[0].item_count = 4;
    articles[0].items = malloc(articles[0].item_count * sizeof(Item));
    articles[0].items[0] = (Item){500, 500, 50};
    articles[0].items[1] = (Item){300, 400, 50};
    articles[0].items[2] = (Item){100, 400, 10};
    articles[0].items[3] = (Item){300, 200, 10};

    for (int i = 0; i < ARTICLES_COUNT; i++)
        for (int j = 0; j < articles[i].item_count; j++)
            articles[i].items[j].count *= ITEMS_COUNT_MULTIPLIER;
}

int main()
{
    Article articles[ARTICLES_COUNT];
    init_articles(articles);

    ArticleResult *results = (ArticleResult *)malloc(ARTICLES_COUNT * sizeof(ArticleResult));

    printf("Начало раскроя...\n");

    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    for (int i = 0; i < ARTICLES_COUNT; i++)
    {
        printf("Обработка артикула %d...\n", i + 1);
        results[i] = cut_article(articles[i]);
    }

    gettimeofday(&t_end, NULL);
    double elapsed = (t_end.tv_sec - t_start.tv_sec) +
                     (t_end.tv_usec - t_start.tv_usec) / 1e6;
    printf("Total time: %f sec\n", elapsed);

    write_results_to_file(articles, results, ARTICLES_COUNT, "cutting_results.txt");

    printf("Результаты сохранены в файл cutting_results.txt\n");

    free_results(results, ARTICLES_COUNT);
    free(results);

    for (int i = 0; i < ARTICLES_COUNT; i++)
        free(articles[i].items);

    return 0;
}