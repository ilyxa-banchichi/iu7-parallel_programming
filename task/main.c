#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// #define USE_MULTITHREAD
// #define DEBUG_LOG

#include "lib/cat.h"

int main()
{
    omp_set_num_threads(8);
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
    double elapsed = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec) / 1e6;
    printf("Total time: %f sec\n", elapsed);

    write_results_to_file(elapsed, articles, results, ARTICLES_COUNT, "cutting_results.txt");
    printf("Результаты сохранены в файл cutting_results.txt\n");

    free_results(results, ARTICLES_COUNT);
    free(results);

    for (int i = 0; i < ARTICLES_COUNT; i++)
        free(articles[i].items);

    return 0;
}