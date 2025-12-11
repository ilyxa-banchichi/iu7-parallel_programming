#ifndef CUTTING_PROGRAM_UTILS
#define CUTTING_PROGRAM_UTILS

#include <stdio.h>
#include "types.h"

static void write_results_to_file(double time, Article *articles, ArticleResult *results, int article_count, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Ошибка открытия файла для записи!\n");
        return;
    }

    fprintf(file, "Total time: %f sec\n", time);

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

#endif // CUTTING_PROGRAM_UTILS