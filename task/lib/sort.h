#ifndef CUTTING_PROGRAM_SORT
#define CUTTING_PROGRAM_SORT

#include <omp.h>
#include <stdlib.h>
#include "types.h"

int rect_compare(const void *a, const void *b)
{
    const Rectangle *ra = a;
    const Rectangle *rb = b;
    return area(*rb) - area(*ra); // сортировка по убыванию
}

void basic_sort(Rectangle *arr, int n)
{
    qsort(arr, n, sizeof(Rectangle), rect_compare);
}

void parallel_sort(Rectangle *arr, int n)
{
    int num_threads;

#pragma omp parallel
    {
#pragma omp single
        num_threads = omp_get_num_threads();
    }

    int chunk = (n + num_threads - 1) / num_threads;

// 1) параллельно сортируем чанки
#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int start = tid * chunk;
        int end = start + chunk;
        if (end > n)
            end = n;

        if (start < n)
            qsort(arr + start, end - start, sizeof(Rectangle), rect_compare);
    }

    // 2) последовательный merge отсортированных списков
    Rectangle *buffer = malloc(n * sizeof(Rectangle));

    int current_size = chunk;
    while (current_size < n)
    {
        int left = 0;

        while (left < n)
        {
            int mid = left + current_size;
            int right = mid + current_size;

            if (mid > n)
                mid = n;
            if (right > n)
                right = n;

            // слияние двух отсортированных частей
            int i = left, j = mid, k = left;

            while (i < mid && j < right)
            {
                if (area(arr[i]) > area(arr[j]))
                    buffer[k++] = arr[i++];
                else
                    buffer[k++] = arr[j++];
            }

            while (i < mid)
                buffer[k++] = arr[i++];
            while (j < right)
                buffer[k++] = arr[j++];

            left += 2 * current_size;
        }

        // копируем результат
        memcpy(arr, buffer, n * sizeof(Rectangle));
        current_size *= 2;
    }

    free(buffer);
}

#endif // CUTTING_PROGRAM_SORT