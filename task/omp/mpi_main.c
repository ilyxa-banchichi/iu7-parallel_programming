#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define USE_MULTITHREAD
#define USE_MPI
// #define DEBUG_LOG

#include "lib/mpi_utils.h"
#include "lib/cat.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (rank == 0)
    {
        Article articles[ARTICLES_COUNT];
        init_articles(articles);
        ArticleResult *results = (ArticleResult *)malloc(ARTICLES_COUNT * sizeof(ArticleResult));

        printf("Начало раскроя MPI (%d процессов)...\n", nprocs);
        struct timeval t_start, t_end;
        gettimeofday(&t_start, NULL);

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

        gettimeofday(&t_end, NULL);
        double elapsed = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec) / 1e6;
        printf("Total time: %f sec\n", elapsed);

        write_results_to_file(elapsed, articles, results, ARTICLES_COUNT, "mpi_cutting_results.txt");
        printf("Результаты сохранены в файл cutting_results.txt\n");

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