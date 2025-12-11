#ifndef CUTTING_PROGRAM_CUT
#define CUTTING_PROGRAM_CUT

#include <omp.h>

#define INITIAL_MAX_SHEETS 1000000

#include "types.h"
#include "task.h"
#include "sort.h"
#include "utils.h"
#include "articles.h"

static int cut_stage(CutTask *task, Rectangle *items_to_cut)
{
#ifdef DEBUG_LOG
    printf("[%d] Выполняем задачу %d для листа %d\n", omp_get_thread_num(), task->id, task->sheet_state->id);
#endif // DEBUG_LOG

    int start_placement_count = task->sheet_state->current_sheet->placement_count;
    for (int i = task->first_item_index; i < task->total_items_count + task->first_item_index; i++)
    {
        if (task->sheet_state->free_rects_count == 0)
            break;

        if (items_to_cut[i].width == 0) // метка, что заготовка уже была размещена
            continue;

        int best_free_index = -1;
        int best_waste = task->sheet_state->width * task->sheet_state->height + 1;
        Rectangle best_placement;
        int best_actual_width, best_actual_height;

        for (int j = 0; j < task->sheet_state->free_rects_count; j++)
        {
            int actual_width, actual_height;
            Rectangle placement = place_item(task->sheet_state->free_rects[j],
                                             items_to_cut[i].width,
                                             items_to_cut[i].height, 1,
                                             &actual_width, &actual_height);

            if (actual_width > 0)
            {
                int waste = area(task->sheet_state->free_rects[j]) - area(placement);
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
            task->sheet_state->current_sheet->placements[task->sheet_state->current_sheet->placement_count] = best_placement;
            task->sheet_state->current_sheet->placement_count++;
            task->sheet_state->current_sheet->used_area += area(best_placement);

            Rectangle new_rects[2];
            int new_count;
            split_rectangle(task->sheet_state->free_rects[best_free_index], best_placement, new_rects, &new_count);

            task->sheet_state->free_rects[best_free_index] = task->sheet_state->free_rects[task->sheet_state->free_rects_count - 1];
            task->sheet_state->free_rects_count--;

            for (int k = 0; k < new_count; k++)
            {
                task->sheet_state->free_rects[task->sheet_state->free_rects_count] = new_rects[k];
                task->sheet_state->free_rects_count++;
            }

            items_to_cut[i].width = 0;
            task->free_items_count--;
        }
    }

    return task->sheet_state->current_sheet->placement_count - start_placement_count;
}

static ArticleResult cut_article(Article article)
{
    ArticleResult result;
    result.sheets = NULL;
    result.sheet_count = -1;
    result.total_waste = 0;

    atomic_int total_items = 0;
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

#ifdef USE_MULTITHREAD
    parallel_sort(items_to_cut, total_items);
#else
    basic_sort(items_to_cut, total_items);
#endif // USE_MULTITHREAD

    int max_sheets = INITIAL_MAX_SHEETS;
    result.sheets = (CutSheet *)malloc(max_sheets * sizeof(CutSheet));

    printf("Всего заготовок для раскроя: %d \n", total_items);

    int tasks_count = 1;
#ifdef USE_MULTITHREAD
    tasks_count = omp_get_max_threads();
#endif // USE_MULTITHREAD
    CutTask *tasks = init_tasks(tasks_count, total_items);

#ifdef USE_MULTITHREAD
#pragma omp parallel
#endif // USE_MULTITHREAD
    {
        int thread_idx = omp_get_thread_num();
#ifdef DEBUG_LOG
        printf("[%d] Поток готов\n", thread_idx);
#endif // DEBUG_LOG

        while (!is_all_tasks_completed(tasks, tasks_count))
        {
            if (thread_idx == 0)
            {
                // ищем первую не до конца выполненую задачу
                int next_task_idx = find_next_wait_init_task(tasks, tasks_count, 0);
                if (next_task_idx == -1)
                    continue;

                result.sheet_count++;
                if (result.sheet_count >= max_sheets)
                {
                    max_sheets *= 2;
                    result.sheets = (CutSheet *)realloc(result.sheets, max_sheets * sizeof(CutSheet));
                }

                CutSheet *current_sheet = &result.sheets[result.sheet_count];
                current_sheet->placements = (Rectangle *)malloc(total_items * sizeof(Rectangle));
                current_sheet->placement_count = 0;
                current_sheet->used_area = 0;

                tasks[next_task_idx].sheet_state = (SheetState *)malloc(sizeof(SheetState));
                tasks[next_task_idx].sheet_state->id = result.sheet_count + 1;
                tasks[next_task_idx].sheet_state->width = article.sheet_width;
                tasks[next_task_idx].sheet_state->height = article.sheet_height;
                tasks[next_task_idx].sheet_state->current_sheet = current_sheet;

                int capacity = total_items < 2 ? 2 : total_items;
                tasks[next_task_idx].sheet_state->free_rects = (Rectangle *)malloc(capacity * sizeof(Rectangle));
                tasks[next_task_idx].sheet_state->free_rects_count = 1;
                tasks[next_task_idx].sheet_state->free_rects[0] = create_rectangle(0, 0, article.sheet_width, article.sheet_height);

#ifdef DEBUG_LOG
                printf("Новый лист %d, стартовая задача %d\n", tasks[next_task_idx].sheet_state->id, tasks[next_task_idx].id);
#endif // DEBUG_LOG
                atomic_store(&tasks[next_task_idx].status, 1);
            }

            int task_idx = thread_idx;
            if (task_idx >= tasks_count)
                break;

            CutTask *task = &tasks[task_idx];

            if (task->status == 3)
            {
                if (thread_idx != 0)
                    break;
            }

            if (task->status == 1)
            {
                while (atomic_exchange(&task->status, 2) == 2)
                {
                }
            }

            if (task->status == 2)
            {
                int new_placement_count = cut_stage(task, items_to_cut);
                atomic_fetch_sub(&total_items, new_placement_count);

                // работа не завершена
                if (task->sheet_state->free_rects_count != 0)
                {
                    // передаем работу следующей незаконченой задаче
                    int next_task_idx = find_next_wait_init_task(tasks, tasks_count, task_idx + 1);
                    if (next_task_idx != -1)
                    {
                        tasks[next_task_idx].sheet_state = task->sheet_state;
                        atomic_store(&tasks[next_task_idx].status, 1);
                    }
                    else
                    {
                        // освобождаем лист
                        free(task->sheet_state->free_rects);
                    }

                    task->sheet_state = NULL;
                }

                if (task->free_items_count != 0)
                    atomic_store(&task->status, 0);
                else
                    atomic_store(&task->status, 3);
            }
        }

#ifdef DEBUG_LOG
        printf("[%d] Поток завершен\n", thread_idx);
#endif // DEBUG_LOG
    }

    free(tasks);
    free(items_to_cut);
    return result;
}

#endif // CUTTING_PROGRAM_CUT