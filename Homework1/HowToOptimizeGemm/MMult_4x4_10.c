
/* Create macros so that the matrices are stored in column-major order */

#define A(i,j) a[ (j)*lda + (i) ]
#define B(i,j) b[ (j)*ldb + (i) ]
#define C(i,j) c[ (j)*ldc + (i) ]

/* Routine for computing C = A * B + C */

void AddDot4x4( int, double *, int, double *, int, double *, int );

void MY_MMult( int m, int n, int k, double *a, int lda, 
                                    double *b, int ldb,
                                    double *c, int ldc )
{
  int i, j;

  for ( j=0; j<n; j+=4 ){        /* Loop over the columns of C, unrolled by 4 */
    for ( i=0; i<m; i+=4 ){        /* Loop over the rows of C */
      /* Update C( i,j ), C( i,j+1 ), C( i,j+2 ), and C( i,j+3 ) in
	 one routine (four inner products) */

      AddDot4x4( k, &A( i,0 ), lda, &B( 0,j ), ldb, &C( i,j ), ldc );
    }
  }
}

#include <arm_neon.h>  // Include NEON intrinsics header for ARM architecture

typedef float32x2_t v2df_t;  // Define a vector type for NEON operations

void AddDot4x4(int k, double *a, int lda, double *b, int ldb, double *c, int ldc) {
    int p;

    v2df_t
        c_00_c_10_vreg, c_01_c_11_vreg, c_02_c_12_vreg, c_03_c_13_vreg,
        c_20_c_30_vreg, c_21_c_31_vreg, c_22_c_32_vreg, c_23_c_33_vreg,
        a_0p_a_1p_vreg, a_2p_a_3p_vreg,
        b_p0_vreg, b_p1_vreg, b_p2_vreg, b_p3_vreg;

    double *b_p0_pntr, *b_p1_pntr, *b_p2_pntr, *b_p3_pntr;
    b_p0_pntr = &B(0, 0);
    b_p1_pntr = &B(0, 1);
    b_p2_pntr = &B(0, 2);
    b_p3_pntr = &B(0, 3);

    c_00_c_10_vreg = vdup_n_f32(0.0f);
    c_01_c_11_vreg = vdup_n_f32(0.0f);
    c_02_c_12_vreg = vdup_n_f32(0.0f);
    c_03_c_13_vreg = vdup_n_f32(0.0f);
    c_20_c_30_vreg = vdup_n_f32(0.0f);
    c_21_c_31_vreg = vdup_n_f32(0.0f);
    c_22_c_32_vreg = vdup_n_f32(0.0f);
    c_23_c_33_vreg = vdup_n_f32(0.0f);

    for (p = 0; p < k; p++) {
        a_0p_a_1p_vreg = vld1_f32((float32_t *)&A(0, p));
        a_2p_a_3p_vreg = vld1_f32((float32_t *)&A(2, p));

        b_p0_vreg = vdup_n_f32((float32_t)*b_p0_pntr++);
        b_p1_vreg = vdup_n_f32((float32_t)*b_p1_pntr++);
        b_p2_vreg = vdup_n_f32((float32_t)*b_p2_pntr++);
        b_p3_vreg = vdup_n_f32((float32_t)*b_p3_pntr++);

        c_00_c_10_vreg = vfma_lane_f32(c_00_c_10_vreg, a_0p_a_1p_vreg, b_p0_vreg, 0);
        c_01_c_11_vreg = vfma_lane_f32(c_01_c_11_vreg, a_0p_a_1p_vreg, b_p1_vreg, 0);
        c_02_c_12_vreg = vfma_lane_f32(c_02_c_12_vreg, a_0p_a_1p_vreg, b_p2_vreg, 0);
        c_03_c_13_vreg = vfma_lane_f32(c_03_c_13_vreg, a_0p_a_1p_vreg, b_p3_vreg, 0);

        c_20_c_30_vreg = vfma_lane_f32(c_20_c_30_vreg, a_2p_a_3p_vreg, b_p0_vreg, 0);
        c_21_c_31_vreg = vfma_lane_f32(c_21_c_31_vreg, a_2p_a_3p_vreg, b_p1_vreg, 0);
        c_22_c_32_vreg = vfma_lane_f32(c_22_c_32_vreg, a_2p_a_3p_vreg, b_p2_vreg, 0);
        c_23_c_33_vreg = vfma_lane_f32(c_23_c_33_vreg, a_2p_a_3p_vreg, b_p3_vreg, 0);
    }

    C(0, 0) += c_00_c_10_vreg[0];
    C(0, 1) += c_01_c_11_vreg[0];
    C(0, 2) += c_02_c_12_vreg[0];
    C(0, 3) += c_03_c_13_vreg[0];

    C(1, 0) += c_00_c_10_vreg[1];
    C(1, 1) += c_01_c_11_vreg[1];
    C(1, 2) += c_02_c_12_vreg[1];
    C(1, 3) += c_03_c_13_vreg[1];

    C(2, 0) += c_20_c_30_vreg[0];
    C(2, 1) += c_21_c_31_vreg[0];
    C(2, 2) += c_22_c_32_vreg[0];
    C(2, 3) += c_23_c_33_vreg[0];

    C(3, 0) += c_20_c_30_vreg[1];
    C(3, 1) += c_21_c_31_vreg[1];
    C(3, 2) += c_22_c_32_vreg[1];
    C(3, 3) += c_23_c_33_vreg[1];
}
