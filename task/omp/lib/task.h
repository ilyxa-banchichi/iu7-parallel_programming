#ifndef CUTTING_PROGRAM_TASK
#define CUTTING_PROGRAM_TASK

#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>

typedef struct
{
    int id;
    int width, height;
    CutSheet *current_sheet;
    Rectangle *free_rects;
    int free_rects_count;
} SheetState;

typedef struct
{
    int id;
    int first_item_index;
    int total_items_count;
    int free_items_count;
    SheetState *sheet_state;
    atomic_int status; // 0 - wait_init, 1 - ready_to_run, 2 - in_progress, 3 - complete
} CutTask;

CutTask *init_tasks(int count, int total_items)
{
    CutTask *tasks = malloc(count * sizeof(CutTask));
    int base_count = total_items / count;
    int remainder = total_items % count;
    int current_index = 0;

    for (int i = 0; i < count; i++)
    {
        tasks[i].id = i + 1;
        tasks[i].first_item_index = current_index;

        tasks[i].total_items_count = base_count + (i < remainder ? 1 : 0);
        tasks[i].free_items_count = tasks[i].total_items_count;
        tasks[i].status = 0;

        current_index += tasks[i].total_items_count;
    }

    return tasks;
}

void print_cut_task(const CutTask *task)
{
    printf("Task %d:\n\tfirst_item_index %d, \n\ttotal_items_count %d, \n\tfree_items_count %d\n",
           task->id, task->first_item_index,
           task->total_items_count,
           task->free_items_count);
}

bool is_all_tasks_completed(CutTask *tasks, int task_count)
{
    for (int i = 0; i < task_count; i++)
    {
        if (tasks[i].status != 3)
            return false;
    }
    return true;
}

int find_next_wait_init_task(CutTask *tasks, int task_count, int start_idx)
{
    for (int i = start_idx; i < task_count; i++)
    {
        // ждем, пока текущее выполнение завершится
        while (tasks[i].status == 1 || tasks[i].status == 2)
        {
        }

        // если задача завершена, то пропускаем
        if (tasks[i].status == 3)
            continue;

        return i;
    }

    return -1;
}

#endif // CUTTING_PROGRAM_TASK