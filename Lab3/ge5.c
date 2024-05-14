// requires additional changes to the code to make it work

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define BLKSIZE 8
#define INDEX(i, j, size) ((i) * (size) + (j))


static double gtod_ref_time_sec = 0.0;

/* Adapted from the bl2_clock() routine in the BLIS library */

double dclock()
{
    double the_time, norm_sec;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (gtod_ref_time_sec == 0.0)
        gtod_ref_time_sec = (double)tv.tv_sec;
    norm_sec = (double)tv.tv_sec - gtod_ref_time_sec;
    the_time = norm_sec + tv.tv_usec * 1.0e-6;
    return the_time;
}


int ge(register double *A, int SIZE)
{
    register int i, j, k;
    for (k = 0; k < SIZE; k++)
    {
        for (i = k + 1; i < SIZE; i++)
        {
            register double multiplier = (A[INDEX(i, k, SIZE)] / A[INDEX(k, k, SIZE)]);
            for (j = k + 1; j < SIZE;)
            {
                if (j < (max(SIZE - BLKSIZE, 0)))
                {
                    A[INDEX(i, j, SIZE)] = A[INDEX(i, j, SIZE)] - A[INDEX(k, j, SIZE)] * multiplier;
                    A[INDEX(i, j + 1, SIZE)] = A[INDEX(i, j + 1, SIZE)] - A[INDEX(k, j + 1, SIZE)] * multiplier;
                    A[INDEX(i, j + 2, SIZE)] = A[INDEX(i, j + 2, SIZE)] - A[INDEX(k, j + 2, SIZE)] * multiplier;
                    A[INDEX(i, j + 3, SIZE)] = A[INDEX(i, j + 3, SIZE)] - A[INDEX(k, j + 3, SIZE)] * multiplier;
                    A[INDEX(i, j + 4, SIZE)] = A[INDEX(i, j + 4, SIZE)] - A[INDEX(k, j + 4, SIZE)] * multiplier;
                    A[INDEX(i, j + 5, SIZE)] = A[INDEX(i, j + 5, SIZE)] - A[INDEX(k, j + 5, SIZE)] * multiplier;
                    A[INDEX(i, j + 6, SIZE)] = A[INDEX(i, j + 6, SIZE)] - A[INDEX(k, j + 6, SIZE)] * multiplier;
                    A[INDEX(i, j + 7, SIZE)] = A[INDEX(i, j + 7, SIZE)] - A[INDEX(k, j + 7, SIZE)] * multiplier;
                    j += BLKSIZE;
                }
                else
                {
                    A[INDEX(i, j, SIZE)] = A[INDEX(i, j, SIZE)] - A[INDEX(k, j, SIZE)] * multiplier;
                    j++;
                }
            }
        }
    }
    return 0;
}

int main(int argc, const char *argv[])
{
    int i, j, k, iret;
    double dtime;
    int SIZE = 1500;
    
    // double **matrix;
    // double *matrix_;
    // matrix_ = (double *)malloc(SIZE * SIZE * sizeof(double));
    // matrix = (double **)malloc(SIZE * sizeof(double *));

    // for (i = 0; i < SIZE; i++)
    // {
    //     matrix[i] = matrix_ + i * SIZE;
    // }

    // srand(1);
    // for (i = 0; i < SIZE; i++)
    // {
    //     for (j = 0; j < SIZE; j++)
    //     {
    //         matrix[i][j] = rand();
    //     }
    // }
    double * matrix = (double *)malloc(SIZE * SIZE * sizeof(double));
    srand(1);
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            matrix[INDEX(i, j, SIZE)] = rand();
        }
    }

    printf("call GE");
    dtime = dclock();
    iret = ge(matrix, SIZE);
    dtime = dclock() - dtime;
    printf("Time: %le \n", dtime);

    double check = 0.0;
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            check = check + matrix[INDEX(i, j, SIZE)];
        }
    }
    printf("Check: %le \n", check);
    fflush(stdout);

    return iret;
}
