//------------------------------------------------------------------------------
// GB_concat_bitmap: concatenate an array of matrices into a bitmap matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_FREE_WORK                        \
    GB_WERK_POP (A_ek_slicing, int64_t) ;   \
    GB_phbix_free (T) ;

#define GB_FREE_ALL         \
    GB_FREE_WORK ;          \
    GB_phbix_free (C) ;

#include "GB_concat.h"

GrB_Info GB_concat_bitmap           // concatenate into a bitmap matrix
(
    GrB_Matrix C,                   // input/output matrix for results
    const int64_t cnz,              // # of entries in C
    const GrB_Matrix *Tiles,        // 2D row-major array of size m-by-n,
    const GrB_Index m,
    const GrB_Index n,
    const int64_t *GB_RESTRICT Tile_rows,  // size m+1
    const int64_t *GB_RESTRICT Tile_cols,  // size n+1
    GB_Context Context
)
{ 
GB_GOTCHA ; // concat_bitmap

    //--------------------------------------------------------------------------
    // allocate C as a bitmap matrix
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL ;
    GB_WERK_DECLARE (A_ek_slicing, int64_t) ;
    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = GB_clear_static_header (&T_header) ;

    GrB_Type ctype = C->type ;
    int64_t cvlen = C->vlen ;
    int64_t cvdim = C->vdim ;
    bool csc = C->is_csc ;
    size_t csize = ctype->size ;
    GB_Type_code ccode = ctype->code ;
    if (!GB_IS_BITMAP (C))
    { 
GB_GOTCHA ; // concat_bitmap
        GB_phbix_free (C) ;
        GB_OK (GB_bix_alloc (C, cvlen * cvdim, true, true, false, true,
            Context)) ;
        C->plen = -1 ;
        C->nvec = cvdim ;
        C->nvec_nonempty = (cvlen > 0) ? cvdim : 0 ;
    }
    ASSERT (GB_IS_BITMAP (C)) ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // concatenate all matrices into C
    //--------------------------------------------------------------------------

    for (int64_t outer = 0 ; outer < (csc ? n : m) ; outer++)
    {
        for (int64_t inner = 0 ; inner < (csc ? m : n) ; inner++)
        {

            //------------------------------------------------------------------
            // get the tile A; transpose and typecast, if needed
            //------------------------------------------------------------------

            A = csc ? GB_TILE (Tiles, inner, outer)
                    : GB_TILE (Tiles, outer, inner) ;
            if (csc != A->is_csc)
            { 
GB_GOTCHA ; // concat_bitmap
                // T = (ctype) A', not in-place
                GB_OK (GB_transpose (&T, ctype, csc, A,
                    NULL, NULL, NULL, false, Context)) ;
                A = T ;
                GB_MATRIX_WAIT (A) ;
            }
            ASSERT (C->is_csc == A->is_csc) ;
            ASSERT (!GB_ANY_PENDING_WORK (A)) ;
            GB_Type_code acode = A->type->code ;

            //------------------------------------------------------------------
            // determine where to place the tile in C
            //------------------------------------------------------------------

            // The tile A appears in vectors cvstart:cvend-1 of C, and indices
            // cistart:ciend-1.

            int64_t cvstart, cvend, cistart, ciend ;
            if (csc)
            { 
GB_GOTCHA ; // concat_bitmap
                // C and A are held by column
                // Tiles is row-major and accessed in column order
                cvstart = Tile_cols [outer] ;
                cvend   = Tile_cols [outer+1] ;
                cistart = Tile_rows [inner] ;
                ciend   = Tile_rows [inner+1] ;
            }
            else
            { 
GB_GOTCHA ; // concat_bitmap
                // C and A are held by row
                // Tiles is row-major and accessed in row order
                cvstart = Tile_rows [outer] ;
                cvend   = Tile_rows [outer+1] ;
                cistart = Tile_cols [inner] ;
                ciend   = Tile_cols [inner+1] ;
            }

            int64_t avdim = cvend - cvstart ;
            int64_t avlen = ciend - cistart ;
            ASSERT (avdim == A->vdim) ;
            ASSERT (avlen == A->vlen) ;
            int64_t anz = GB_NNZ_HELD (A) ;

            //------------------------------------------------------------------
            // copy the tile A into C
            //------------------------------------------------------------------

            if (ccode == acode)
            {
                // no typecasting needed
                switch (csize)
                {
                    #define GB_COPY(pC,pA) Cx [pC] = Ax [pA]

                    case 1 : // uint8, int8, bool, or 1-byte user-defined
GB_GOTCHA ; // concat_bitmap
                        #define GB_CTYPE uint8_t
                        #include "GB_concat_bitmap_template.c"
                        break ;

                    case 2 : // uint16, int16, or 2-byte user-defined
GB_GOTCHA ; // concat_bitmap
                        #define GB_CTYPE uint16_t
                        #include "GB_concat_bitmap_template.c"
                        break ;

                    case 4 : // uint32, int32, float, or 4-byte user-defined
GB_GOTCHA ; // concat_bitmap
                        #define GB_CTYPE uint32_t
                        #include "GB_concat_bitmap_template.c"
                        break ;

                    case 8 : // uint64, int64, double, float complex,
GB_GOTCHA ; // concat_bitmap
                             // or 8-byte user defined
                        #define GB_CTYPE uint64_t
                        #include "GB_concat_bitmap_template.c"
                        break ;

                    case 16 : // double complex or 16-byte user-defined
GB_GOTCHA ; // concat_bitmap
                        #define GB_CTYPE uint64_t
                        #undef  GB_COPY
                        #define GB_COPY(pC,pA)                      \
                            Cx [2*pC  ] = Ax [2*pA  ] ;             \
                            Cx [2*pC+1] = Ax [2*pA+1] ;
                        #include "GB_concat_bitmap_template.c"
                        break ;

                    default : // user-defined of a different size
GB_GOTCHA ; // concat_bitmap
                        #define GB_CTYPE GB_void
                        #undef  GB_COPY
                        #define GB_COPY(pC,pA)                      \
                            memcpy (Cx + pC, Ax + pA, csize) ;
                        #include "GB_concat_bitmap_template.c"
                        break ;
                }
            }
            else
            { 
GB_GOTCHA ; // concat_bitmap
                // with typecasting (not for user-defined types)
                GB_cast_function cast_A_to_C = GB_cast_factory (ccode, acode) ;
                size_t asize = A->type->size ;
                #define GB_CTYPE GB_void
                #undef  GB_COPY
                #define GB_COPY(pC,pA)  \
                    cast_A_to_C (Cx + (pC)*csize, Ax + (pA)*asize, asize) ;
                #include "GB_concat_bitmap_template.c"
            }

            GB_FREE_WORK ;
        }
    }

    C->magic = GB_MAGIC ;
    C->nvals = cnz ;
    return (GrB_SUCCESS) ;
}

