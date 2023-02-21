//------------------------------------------------------------------------------
// GB_nnz_full_template.c: number of entries in a full matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifdef GB_CUDA_KERNEL
__device__ static inline
#endif
int64_t GB_nnz_full      // return nnz(A) or INT64_MAX if integer overflow
(
    GrB_Matrix A
)
{ 
    uint64_t nz ;
    bool ok = GB_uint64_multiply (&nz, (uint64_t) A->vlen, (uint64_t) A->vdim) ;
    return (ok ? ((int64_t) nz) : INT64_MAX) ;
}

