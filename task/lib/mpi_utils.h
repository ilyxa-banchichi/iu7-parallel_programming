#ifndef CUTTING_PROGRAM_MPI_UTILS
#define CUTTING_PROGRAM_MPI_UTILS

#ifdef USE_MPI
#include <mpi.h>
#include "types.h"

typedef struct
{
    int index;
    int sheet_width;
    int sheet_height;
    int item_count;
} ArticleHeader;

static void MPI_SendArticle(Article a, int p)
{
    ArticleHeader hdr;
    printf("%d", a.index);
    hdr.index = a.index;
    hdr.sheet_width = a.sheet_width;
    hdr.sheet_height = a.sheet_height;
    hdr.item_count = a.item_count;
    MPI_Send(&hdr, sizeof(ArticleHeader), MPI_BYTE, p, 0, MPI_COMM_WORLD);
    if (hdr.item_count > 0)
        MPI_Send(a.items, hdr.item_count * sizeof(Item), MPI_BYTE, p, 0, MPI_COMM_WORLD);
}

static int MPI_ReceiveArticle(Article *task)
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

static MPI_Status MPI_ReceiveArticleResult(ArticleResult *results)
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

static void MPI_SendArticleResult(ArticleResult res)
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

#endif // USE_MPI

#endif // CUTTING_PROGRAM_MPI_UTILS