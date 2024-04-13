
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

void InnerKernel( int m, int n, int k, double *a, int lda, 
                                       double *b, int ldb,
                                       double *c, int ldc )
{
  int i, j;
  double 
    packedA[ m * k ];

  for ( j=0; j<n; j+=4 ){        /* Loop over the columns of C, unrolled by 4 */
    for ( i=0; i<m; i+=4 ){        /* Loop over the rows of C */
      /* Update C( i,j ), C( i,j+1 ), C( i,j+2 ), and C( i,j+3 ) in
	 one routine (four inner products) */
      PackMatrixA( k, &A( i, 0 ), lda, &packedA[ i*k ] );
      AddDot4x4( k, &packedA[ i*k ], 4, &B( 0,j ), ldb, &C( i,j ), ldc );
    }
  }
}


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
      InnerKernel( ib, n, pb, &A( i,p ), lda, &B(p, 0 ), ldb, &C( i,0 ), ldc );
    }
  }
}

void PackMatrixA( int k, double *a, int lda, double *a_to )
{
  int j;

  for( j=0; j<k; j++){  /* loop over columns of A */
    double 
      *a_ij_pntr = &A( 0, j );

    *a_to++ = *a_ij_pntr;
    *a_to++ = *(a_ij_pntr+1);
    *a_to++ = *(a_ij_pntr+2);
    *a_to++ = *(a_ij_pntr+3);
  }
}

#include <arm_neon.h>

typedef float64x2_t v2df_t;

void AddDot4x4( int k, double *a, int lda,  double *b, int ldb, double *c, int ldc )
{
  int p;
  v2df_t
    c_00_c_10_vreg,    c_01_c_11_vreg,    c_02_c_12_vreg,    c_03_c_13_vreg,
    c_20_c_30_vreg,    c_21_c_31_vreg,    c_22_c_32_vreg,    c_23_c_33_vreg,
    a_0p_a_1p_vreg,
    a_2p_a_3p_vreg,
    b_p0_vreg, b_p1_vreg, b_p2_vreg, b_p3_vreg;

  double 
    *b_p0_pntr, *b_p1_pntr, *b_p2_pntr, *b_p3_pntr;

  b_p0_pntr = &B( 0, 0 );
  b_p1_pntr = &B( 0, 1 );
  b_p2_pntr = &B( 0, 2 );
  b_p3_pntr = &B( 0, 3 );

  c_00_c_10_vreg = vdupq_n_f64(0);
  c_01_c_11_vreg = vdupq_n_f64(0);
  c_02_c_12_vreg = vdupq_n_f64(0);
  c_03_c_13_vreg = vdupq_n_f64(0);
  c_20_c_30_vreg = vdupq_n_f64(0);
  c_21_c_31_vreg = vdupq_n_f64(0);
  c_22_c_32_vreg = vdupq_n_f64(0);
  c_23_c_33_vreg = vdupq_n_f64(0);

  for ( p = 0; p < k; p++ ){
    a_0p_a_1p_vreg = vld1q_f64(&A( 0, p ));
    a_2p_a_3p_vreg = vld1q_f64(&A( 2, p ));

    b_p0_vreg = vdupq_n_f64(*b_p0_pntr++);
    b_p1_vreg = vdupq_n_f64(*b_p1_pntr++);
    b_p2_vreg = vdupq_n_f64(*b_p2_pntr++);
    b_p3_vreg = vdupq_n_f64(*b_p3_pntr++);

    c_00_c_10_vreg = vmlaq_f64(c_00_c_10_vreg, a_0p_a_1p_vreg, b_p0_vreg);
    c_01_c_11_vreg = vmlaq_f64(c_01_c_11_vreg, a_0p_a_1p_vreg, b_p1_vreg);
    c_02_c_12_vreg = vmlaq_f64(c_02_c_12_vreg, a_0p_a_1p_vreg, b_p2_vreg);
    c_03_c_13_vreg = vmlaq_f64(c_03_c_13_vreg, a_0p_a_1p_vreg, b_p3_vreg);

    c_20_c_30_vreg = vmlaq_f64(c_20_c_30_vreg, a_2p_a_3p_vreg, b_p0_vreg);
    c_21_c_31_vreg = vmlaq_f64(c_21_c_31_vreg, a_2p_a_3p_vreg, b_p1_vreg);
    c_22_c_32_vreg = vmlaq_f64(c_22_c_32_vreg, a_2p_a_3p_vreg, b_p2_vreg);
    c_23_c_33_vreg = vmlaq_f64(c_23_c_33_vreg, a_2p_a_3p_vreg, b_p3_vreg);
  }

  C( 0, 0 ) += vgetq_lane_f64(c_00_c_10_vreg, 0);
  C( 0, 1 ) += vgetq_lane_f64(c_01_c_11_vreg, 0);
  C( 0, 2 ) += vgetq_lane_f64(c_02_c_12_vreg, 0);
  C( 0, 3 ) += vgetq_lane_f64(c_03_c_13_vreg, 0);

  C( 1, 0 ) += vgetq_lane_f64(c_00_c_10_vreg, 1);
  C( 1, 1 ) += vgetq_lane_f64(c_01_c_11_vreg, 1);
  C( 1, 2 ) += vgetq_lane_f64(c_02_c_12_vreg, 1);
  C( 1, 3 ) += vgetq_lane_f64(c_03_c_13_vreg, 1);

  C( 2, 0 ) += vgetq_lane_f64(c_20_c_30_vreg, 0);
  C( 2, 1 ) += vgetq_lane_f64(c_21_c_31_vreg, 0);
  C( 2, 2 ) += vgetq_lane_f64(c_22_c_32_vreg, 0);
  C( 2, 3 ) += vgetq_lane_f64(c_23_c_33_vreg, 0);

  C( 3, 0 ) += vgetq_lane_f64(c_20_c_30_vreg, 1);
  C( 3, 1 ) += vgetq_lane_f64(c_21_c_31_vreg, 1);
  C( 3, 2 ) += vgetq_lane_f64(c_22_c_32_vreg, 1);
  C( 3, 3 ) += vgetq_lane_f64(c_23_c_33_vreg, 1);
}
