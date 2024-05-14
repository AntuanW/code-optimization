// requires additional changes to the code to make it work

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define max(a, b) ((a) > (b) ? (a) : (b))
#define BLKSIZE 8
#define INDEX(i, j, size) ((i) * (size) + (j))

#include <arm_neon.h>
typedef float64x2_t __m128d;
#define _mm_loadu_pd(a) vld1q_f64(a)
#define _mm_mul_pd(a, b) vmulq_f64(a, b)
#define _mm_sub_pd(a, b) vsubq_f64(a, b)
#define _mm_storeu_pd(a, b) vst1q_f64(a, b)

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
    register unsigned int i, j, k;
    register double multiplier;
    register __m128d mm_multiplier;
    register __m128d tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    for (k = 0; k < SIZE; k++)
    {
        for (i = k + 1; i < SIZE; i++)
        {
            multiplier = A[INDEX(i, k, SIZE)] / A[INDEX(k, k, SIZE)];
            mm_multiplier[0] = multiplier;
            mm_multiplier[1] = multiplier;
            for (j = k + 1; j < SIZE;)
            {
                if (j < (max(SIZE - BLKSIZE, 0)))
                {
                    tmp0 = _mm_loadu_pd(A + INDEX(i, j, SIZE));
                    tmp2 = _mm_loadu_pd(A + INDEX(i, j + 2, SIZE));
                    tmp4 = _mm_loadu_pd(A + INDEX(i, j + 4, SIZE));
                    tmp6 = _mm_loadu_pd(A + INDEX(i, j + 6, SIZE));

                    tmp1 = _mm_loadu_pd(A + INDEX(k, j, SIZE));
                    tmp3 = _mm_loadu_pd(A + INDEX(k, j + 2, SIZE));
                    tmp5 = _mm_loadu_pd(A + INDEX(k, j + 4, SIZE));
                    tmp7 = _mm_loadu_pd(A + INDEX(k, j + 6, SIZE));

                    tmp1 = _mm_mul_pd(tmp1, mm_multiplier);
                    tmp3 = _mm_mul_pd(tmp3, mm_multiplier);
                    tmp5 = _mm_mul_pd(tmp5, mm_multiplier);
                    tmp7 = _mm_mul_pd(tmp7, mm_multiplier);

                    tmp0 = _mm_sub_pd(tmp0, tmp1);
                    tmp2 = _mm_sub_pd(tmp2, tmp3);
                    tmp4 = _mm_sub_pd(tmp4, tmp5);
                    tmp6 = _mm_sub_pd(tmp6, tmp7);

                    _mm_storeu_pd(A+INDEX(i, j, SIZE), tmp0);
                    _mm_storeu_pd(A+INDEX(i, j + 2, SIZE), tmp2);
                    _mm_storeu_pd(A+INDEX(i, j + 4, SIZE), tmp4);
                    _mm_storeu_pd(A+INDEX(i, j + 6, SIZE), tmp6);
                    j += BLKSIZE;
                }
                else
                {
                    A[INDEX(i, j, SIZE)] = A[INDEX(i, j, SIZE)] - multiplier * A[INDEX(k, j, SIZE)];
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
    double *matrix = (double *)malloc(SIZE * SIZE * sizeof(double));
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
