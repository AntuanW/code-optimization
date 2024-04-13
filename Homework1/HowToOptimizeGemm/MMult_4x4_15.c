#include <arm_neon.h>

/* Define macros for column-major order */
#define A(i,j) a[ (j)*lda + (i) ]
#define B(i,j) b[ (j)*ldb + (i) ]
#define C(i,j) c[ (j)*ldc + (i) ]

/* Block sizes */
#define mc 256
#define kc 128
#define nb 1000

#define min( i, j ) ( (i)<(j) ? (i): (j) )

/* Function prototypes */
void AddDot4x4(int k, double *a, int lda, double *b, int ldb, double *c, int ldc);
void PackMatrixA(int k, double *a, int lda, double *a_to);
void PackMatrixB(int k, double *b, int ldb, double *b_to);
void InnerKernel(int m, int n, int k, double *a, int lda, double *b, int ldb, double *c, int ldc, int first_time);

/* Main matrix multiplication function */
void MY_MMult(int m, int n, int k, double *a, int lda, double *b, int ldb, double *c, int ldc) {
    int i, p, pb, ib;

    /* Compute a mc x n block of C by a call to InnerKernel */
    for (p = 0; p < k; p += kc) {
        pb = min(k - p, kc);
        for (i = 0; i < m; i += mc) {
            ib = min(m - i, mc);
            InnerKernel(ib, n, pb, &A(i, p), lda, &B(p, 0), ldb, &C(i, 0), ldc, i == 0);
        }
    }
}

/* Inner kernel function */
void InnerKernel(int m, int n, int k, double *a, int lda, double *b, int ldb, double *c, int ldc, int first_time) {
    int i, j;
    double packedA[m * k];
    static double packedB[kc * nb];  // Note: using a static buffer is not thread safe...

    for (j = 0; j < n; j += 4) {  // Loop over the columns of C, unrolled by 4
        if (first_time)
            PackMatrixB(k, &B(0, j), ldb, &packedB[j * k]);
        for (i = 0; i < m; i += 4) {  // Loop over the rows of C
            if (j == 0)
                PackMatrixA(k, &A(i, 0), lda, &packedA[i * k]);
            AddDot4x4(k, &packedA[i * k], 4, &packedB[j * k], k, &C(i, j), ldc);
        }
    }
}

/* Function to pack matrix A */
void PackMatrixA(int k, double *a, int lda, double *a_to) {
    for (int j = 0; j < k; j++) {  // Loop over columns of A
        double *a_ij_pntr = &A(0, j);
        *a_to++ = *a_ij_pntr;
        *(a_to++) = *(a_ij_pntr + 1);
        *(a_to++) = *(a_ij_pntr + 2);
        *(a_to++) = *(a_ij_pntr + 3);
    }
}

/* Function to pack matrix B */
void PackMatrixB(int k, double *b, int ldb, double *b_to) {
    for (int i = 0; i < k; i++) {  // Loop over rows of B
        *b_to++ = *b++;
        *b_to++ = *b++;
        *b_to++ = *b++;
        *b_to++ = *b++;
    }
}

/* Function to perform dot product */
void AddDot4x4(int k, double *a, int lda, double *b, int ldb, double *c, int ldc) {
    /* ARM NEON registers for computation */
    float64x2_t c_00_c_10_vreg = vdupq_n_f64(0);
    float64x2_t c_01_c_11_vreg = vdupq_n_f64(0);
    float64x2_t c_02_c_12_vreg = vdupq_n_f64(0);
    float64x2_t c_03_c_13_vreg = vdupq_n_f64(0);
    float64x2_t c_20_c_30_vreg = vdupq_n_f64(0);
    float64x2_t c_21_c_31_vreg = vdupq_n_f64(0);
    float64x2_t c_22_c_32_vreg = vdupq_n_f64(0);
    float64x2_t c_23_c_33_vreg = vdupq_n_f64(0);
    float64x2_t a_0p_a_1p_vreg, a_2p_a_3p_vreg;
    float64x2_t b_p0_vreg, b_p1_vreg, b_p2_vreg, b_p3_vreg;

    for (int p = 0; p < k; p++) {
        a_0p_a_1p_vreg = vld1q_f64(a);
        a_2p_a_3p_vreg = vld1q_f64(a + 2);
        a += 4;

        b_p0_vreg = vld1q_dup_f64(b++);
        b_p1_vreg = vld1q_dup_f64(b++);
        b_p2_vreg = vld1q_dup_f64(b++);
        b_p3_vreg = vld1q_dup_f64(b++);

        c_00_c_10_vreg = vfmaq_laneq_f64(c_00_c_10_vreg, a_0p_a_1p_vreg, b_p0_vreg, 0);
        c_01_c_11_vreg = vfmaq_laneq_f64(c_01_c_11_vreg, a_0p_a_1p_vreg, b_p1_vreg, 0);
        c_02_c_12_vreg = vfmaq_laneq_f64(c_02_c_12_vreg, a_0p_a_1p_vreg, b_p2_vreg, 0);
        c_03_c_13_vreg = vfmaq_laneq_f64(c_03_c_13_vreg, a_0p_a_1p_vreg, b_p3_vreg, 0);
        c_20_c_30_vreg = vfmaq_laneq_f64(c_20_c_30_vreg, a_2p_a_3p_vreg, b_p0_vreg, 0);
        c_21_c_31_vreg = vfmaq_laneq_f64(c_21_c_31_vreg, a_2p_a_3p_vreg, b_p1_vreg, 0);
        c_22_c_32_vreg = vfmaq_laneq_f64(c_22_c_32_vreg, a_2p_a_3p_vreg, b_p2_vreg, 0);
        c_23_c_33_vreg = vfmaq_laneq_f64(c_23_c_33_vreg, a_2p_a_3p_vreg, b_p3_vreg, 0);
    }

    /* Store results back to matrix C */
    vst1q_f64(c, c_00_c_10_vreg);
    vst1q_f64(c + ldc, c_01_c_11_vreg);
    vst1q_f64(c + 2 * ldc, c_02_c_12_vreg);
    vst1q_f64(c + 3 * ldc, c_03_c_13_vreg);
    vst1q_f64(c + 4 * ldc, c_20_c_30_vreg);
    vst1q_f64(c + (4 * ldc) + 1, c_21_c_31_vreg);
    vst1q_f64(c + (4 * ldc) + 2, c_22_c_32_vreg);
    vst1q_f64(c + (4 * ldc) + 3, c_23_c_33_vreg);
}
