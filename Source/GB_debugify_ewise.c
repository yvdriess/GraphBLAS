//------------------------------------------------------------------------------
// GB_debugify_ewise: dump definitions for ewise to /tmp/grb/GB_ewise_*.h file
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_debugify_ewise
(
    // C matrix:
    bool C_iso,             // if true, operator is ignored
    int C_sparsity,         // sparse, hyper, bitmap, or full
    GrB_Type ctype,         // C=((ctype) T) is the final typecast
    // M matrix:
    GrB_Matrix M,
    bool Mask_struct,
    bool Mask_comp,
    // operator:
    GrB_BinaryOp binaryop,  // may be NULL (used by GB_wait)
    bool flipxy,
    // A and B matrices:
    GrB_Matrix A,
    GrB_Matrix B
)
{

    uint64_t scode ;

    GrB_Type atype = A->type ;
    GrB_Type btype = B->type ;

    // enumify the ewise problem
    GB_enumify_ewise (&scode, C_iso, C_sparsity, ctype,
        M, Mask_struct, Mask_comp, binaryop, flipxy, A, B) ;

    // FIXME: use naming convention from GB_debugify_mxm
    // namify the ewise problem
    bool builtin = ((binaryop == NULL) || binaryop->header_size == 0) &&
        (atype->header_size == 0) &&
        (btype->header_size == 0) &&
        (ctype->header_size == 0) ;

    char ewise_name [256 + 8*GxB_MAX_NAME_LEN] ;
    GB_namify_problem (ewise_name, 12, scode, builtin,
        (binaryop == NULL) ? "none" : binaryop->name,
        NULL,
        (binaryop == NULL) ? "void" : binaryop->xtype->name,
        (binaryop == NULL) ? "void" : binaryop->ytype->name,
        (binaryop == NULL) ? "void" : binaryop->ztype->name,
        atype->name,
        btype->name,
        ctype->name) ;

    // construct the filename and create the file
    char filename [512 + 8*GxB_MAX_NAME_LEN] ;
    sprintf (filename, "/tmp/grb/GB_ewise_%s.h", ewise_name);
    FILE *fp = fopen (filename, "w") ;
    if (fp == NULL) return ;

    // FIXME: pass this to GB_macrofy_ewise
    fprintf (fp,
        "//--------------------------------------"
        "----------------------------------------\n") ;
    fprintf (fp, "// GB_ewise_%s.h\n", ewise_name) ;

    // macrofy the ewise problem
    GB_macrofy_ewise (fp, scode, binaryop, ctype, atype, btype) ;
    fclose (fp) ;
}

