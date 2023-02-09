//------------------------------------------------------------------------------
// GB_AxB_dot4_meta:  C+=A'*B via dot products, where C is full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C+=A'*B where C is a dense matrix and computed in-place.  The monoid of the
// semiring matches the accum operator, and the type of C matches the ztype of
// accum.  That is, no typecasting can be done with C.

// This method is not used for the generic case with memcpy's and function
// pointers.  It is only used for pre-generated and JIT kernels.

// The matrix C is the user input matrix.  C is not iso on output, but might
// iso on input, in which case the input iso scalar is cinput, and C->x has
// been expanded to non-iso.  If A and/or B are hypersparse, the iso value of C
// has been expanded, so that C->x is initialized.  Otherwise, C->x is not
// initialized.  Instead, each entry is initialized by the iso value in
// the GB_GET4C(cij,p) macro.  A and/or B can be iso.

#define GB_DOT4

// cij += A(k,i) * B(k,j)
#undef  GB_DOT
#define GB_DOT(k,pA,pB)                                                     \
{                                                                           \
    GB_IF_TERMINAL_BREAK (cij, zterminal) ; /* break if cij == zterminal */ \
    GB_DECLAREA (aki) ;                                                     \
    GB_GETA (aki, Ax, pA, A_iso) ;          /* aki = A(k,i) */              \
    GB_DECLAREB (bkj) ;                                                     \
    GB_GETB (bkj, Bx, pB, B_iso) ;          /* bkj = B(k,j) */              \
    GB_MULTADD (cij, aki, bkj, i, k, j) ;   /* cij += aki * bkj */          \
}

{ 

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    const int64_t cvlen = C->vlen ;

    const int64_t  *restrict Bp = B->p ;
    const int8_t   *restrict Bb = B->b ;
    const int64_t  *restrict Bh = B->h ;
    const int64_t  *restrict Bi = B->i ;
    const bool B_iso = B->iso ;
    const int64_t vlen = B->vlen ;
    const int64_t bvdim = B->vdim ;
    const bool B_is_hyper = GB_IS_HYPERSPARSE (B) ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    const bool B_is_sparse = GB_IS_SPARSE (B) ;

    const int64_t  *restrict Ap = A->p ;
    const int8_t   *restrict Ab = A->b ;
    const int64_t  *restrict Ah = A->h ;
    const int64_t  *restrict Ai = A->i ;
    const bool A_iso = A->iso ;
    const int64_t avdim = A->vdim ;
    ASSERT (A->vlen == B->vlen) ;
    ASSERT (A->vdim == C->vlen) ;
    const bool A_is_hyper = GB_IS_HYPERSPARSE (A) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_sparse = GB_IS_SPARSE (A) ;

    #if GB_IS_ANY_MONOID
    #error "dot4 not supported for ANY monoids"
    #endif

    GB_DECLARE_TERMINAL_CONST (zterminal) ;

    #if !GB_A_IS_PATTERN
    const GB_A_TYPE *restrict Ax = (GB_A_TYPE *) A->x ;
    #endif
    #if !GB_B_IS_PATTERN
    const GB_B_TYPE *restrict Bx = (GB_B_TYPE *) B->x ;
    #endif
          GB_C_TYPE *restrict Cx = (GB_C_TYPE *) C->x ;

    int ntasks = naslice * nbslice ;

    //--------------------------------------------------------------------------
    // if C is iso on input: get the iso scalar and convert C to non-iso
    //--------------------------------------------------------------------------

    const bool C_in_iso = C->iso ;
    GB_DECLARE_IDENTITY_CONST (zidentity) ;
    const GB_C_TYPE cinput = (C_in_iso) ? Cx [0] : zidentity ;
    if (C_in_iso)
    { 
        // allocate but do not initialize C->x unless A or B are hypersparse
        GrB_Info info = GB_convert_any_to_non_iso (C, A_is_hyper || B_is_hyper);
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            return (GrB_OUT_OF_MEMORY) ;
        }
        ASSERT (!C->iso) ;
        Cx = (GB_C_TYPE *) C->x ;
    }

    //--------------------------------------------------------------------------
    // C += A'*B
    //--------------------------------------------------------------------------

    #include "GB_meta16_factory.c"
}

#undef GB_DOT
#undef GB_DOT4

