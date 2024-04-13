/* Create macros so that the matrices are stored in column-major order */

#define A(i,j) a[ (j)*lda + (i) ]
#define B(i,j) b[ (j)*ldb + (i) ]
#define C(i,j) c[ (j)*ldc + (i) ]

/* Block sizes */
#define mc 256
#define kc 128

#define min( i, j ) ( (i)<(j) ? (i): (j) )

/* Routine for computing C = A * B + C */

void AddDot4x4( int, double *, int, double *, int, double *, int );
void PackMatrixA( int, double *, int, double * );
void PackMatrixB( int, double *, int, double * );
void InnerKernel( int, int, int, double *, int, double *, int, double *, int, int );

void MY_MMult( int m, int n, int k, double *a, int lda, 
                                    double *b, int ldb,
                                    double *c, int ldc )
{
  int i, p, pb, ib;

  /* This time, we compute a mc x n block of C by a call to the InnerKernel */

  for ( p=0; p<k; p+=kc ){
    pb = min( k-p, kc );
    for ( i=0; i<m; i+=mc ){
      ib = min( m-i, mc );
      InnerKernel( ib, n, pb, &A( i,p ), lda, &B(p, 0 ), ldb, &C( i,0 ), ldc, i==0 );
    }
  }
}

void InnerKernel( int m, int n, int k, double *a, int lda, 
                                       double *b, int ldb,
                                       double *c, int ldc, int first_time )
{
  int i, j;
  double 
    packedA[ m * k ], packedB[ k*n ];

  for ( j=0; j<n; j+=4 ){        /* Loop over the columns of C, unrolled by 4 */
    PackMatrixB( k, &B( 0, j ), ldb, &packedB[ j*k ] );
    for ( i=0; i<m; i+=4 ){        /* Loop over the rows of C */
      /* Update C( i,j ), C( i,j+1 ), C( i,j+2 ), and C( i,j+3 ) in
	 one routine (four inner products) */
      if ( j == 0 ) 
	PackMatrixA( k, &A( i, 0 ), lda, &packedA[ i*k ] );
      AddDot4x4( k, &packedA[ i*k ], 4, &packedB[ j*k ], k, &C( i,j ), ldc );
    }
  }
}

void PackMatrixA( int k, double *a, int lda, double *a_to )
{
  int j;

  for( j=0; j<k; j++){  /* loop over columns of A */
    double 
      *a_ij_pntr = &A( 0, j );

    *a_to     = *a_ij_pntr;
    *(a_to+1) = *(a_ij_pntr+1);
    *(a_to+2) = *(a_ij_pntr+2);
    *(a_to+3) = *(a_ij_pntr+3);

    a_to += 4;
  }
}

void PackMatrixB( int k, double *b, int ldb, double *b_to )
{
  int i;
  double 
    *b_i0_pntr = &B( 0, 0 ), *b_i1_pntr = &B( 0, 1 ),
    *b_i2_pntr = &B( 0, 2 ), *b_i3_pntr = &B( 0, 3 );

  for( i=0; i<k; i++){  /* loop over rows of B */
    *b_to++ = *b_i0_pntr++;
    *b_to++ = *b_i1_pntr++;
    *b_to++ = *b_i2_pntr++;
    *b_to++ = *b_i3_pntr++;
  }
}

#include <arm_neon.h>

typedef float32x2_t v2df_t;

void AddDot4x4(int k, double *a, int lda, double *b, int ldb, double *c, int ldc) {
    /* Define ARM NEON registers for the computation */
    float32x2_t c_00_c_10_vreg = vdup_n_f32(0);
    float32x2_t c_01_c_11_vreg = vdup_n_f32(0);
    float32x2_t c_02_c_12_vreg = vdup_n_f32(0);
    float32x2_t c_03_c_13_vreg = vdup_n_f32(0);
    float32x2_t c_20_c_30_vreg = vdup_n_f32(0);
    float32x2_t c_21_c_31_vreg = vdup_n_f32(0);
    float32x2_t c_22_c_32_vreg = vdup_n_f32(0);
    float32x2_t c_23_c_33_vreg = vdup_n_f32(0);
    
    /* Loop through each element of the matrices */
    for (int p = 0; p < k; p++) {
        float32x2_t a_0p_a_1p_vreg = vld1_f32((float32_t *)a);
        float32x2_t a_2p_a_3p_vreg = vld1_f32((float32_t *)(a + 2));
        float32x2_t b_p0_vreg = vdup_n_f32(*b);
        float32x2_t b_p1_vreg = vdup_n_f32(*(b + 1));
        float32x2_t b_p2_vreg = vdup_n_f32(*(b + 2));
        float32x2_t b_p3_vreg = vdup_n_f32(*(b + 3));
        
        /* Perform matrix multiplication using ARM NEON intrinsics */
        c_00_c_10_vreg = vfma_lane_f32(c_00_c_10_vreg, a_0p_a_1p_vreg, b_p0_vreg, 0);
        c_01_c_11_vreg = vfma_lane_f32(c_01_c_11_vreg, a_0p_a_1p_vreg, b_p1_vreg, 0);
        c_02_c_12_vreg = vfma_lane_f32(c_02_c_12_vreg, a_0p_a_1p_vreg, b_p2_vreg, 0);
        c_03_c_13_vreg = vfma_lane_f32(c_03_c_13_vreg, a_0p_a_1p_vreg, b_p3_vreg, 0);
        c_20_c_30_vreg = vfma_lane_f32(c_20_c_30_vreg, a_2p_a_3p_vreg, b_p0_vreg, 0);
        c_21_c_31_vreg = vfma_lane_f32(c_21_c_31_vreg, a_2p_a_3p_vreg, b_p1_vreg, 0);
        c_22_c_32_vreg = vfma_lane_f32(c_22_c_32_vreg, a_2p_a_3p_vreg, b_p2_vreg, 0);
        c_23_c_33_vreg = vfma_lane_f32(c_23_c_33_vreg, a_2p_a_3p_vreg, b_p3_vreg, 0);
        
        /* Move to the next column of matrix B */
        b += 4;
        /* Move to the next row of matrix A */
        a += 4;
    }
    
    /* Store the results back to matrix C */
    c[0] += c_00_c_10_vreg[0];
    c[1] += c_01_c_11_vreg[0];
    c[2] += c_02_c_12_vreg[0];
    c[3] += c_03_c_13_vreg[0];
    c[4] += c_00_c_10_vreg[1];
    c[5] += c_01_c_11_vreg[1];
    c[6] += c_02_c_12_vreg[1];
    c[7] += c_03_c_13_vreg[1];
    c[8] += c_20_c_30_vreg[0];
    c[9] += c_21_c_31_vreg[0];
    c[10] += c_22_c_32_vreg[0];
    c[11] += c_23_c_33_vreg[0];
    c[12] += c_20_c_30_vreg[1];
    c[13] += c_21_c_31_vreg[1];
    c[14] += c_22_c_32_vreg[1];
    c[15] += c_23_c_33_vreg[1];
}
