#ifndef CUTTING_PROGRAM_TYPES
#define CUTTING_PROGRAM_TYPES

typedef struct
{
    int x, y;
    int width, height;
} Rectangle;

static inline Rectangle create_rectangle(int x, int y, int width, int height)
{
    Rectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    return rect;
}

static inline int area(Rectangle rect)
{
    return rect.width * rect.height;
}

static void split_rectangle(Rectangle free_rect, Rectangle placed_rect,
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

static inline int can_place(Rectangle rect, int item_width, int item_height, int allow_rotation)
{
    if ((item_width <= rect.width && item_height <= rect.height) ||
        (allow_rotation && item_height <= rect.width && item_width <= rect.height))
    {
        return 1;
    }
    return 0;
}

typedef struct
{
    int width, height;
    int count;
} Item;

static inline int compare_items(const Item *a, const Item *b)
{
    int areaA = a->width * a->height;
    int areaB = b->width * b->height;
    return areaB - areaA;
}

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

static inline void free_results(ArticleResult *results, int article_count)
{
    for (int i = 0; i < article_count; i++)
    {
        for (int j = 0; j < results[i].sheet_count; j++)
            free(results[i].sheets[j].placements);
        free(results[i].sheets);
    }
}

static Rectangle place_item(Rectangle rect, int item_width, int item_height, int allow_rotation,
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

#endif // CUTTING_PROGRAM_TYPES