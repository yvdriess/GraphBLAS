//------------------------------------------------------------------------------
// GB_red:  hard-coded functions for reductions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2023, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If this file is in the Generated2/ folder, do not edit it
// (it is auto-generated from Generator/*).

#include "GB.h"
#ifndef GBCUDA_DEV
#include "GB_control.h" 
#include "GB_red__include.h"

// The reduction is defined by the following types and operators:

// Reduce to scalar:   GB (_red__plus_uint16)

// A type:   uint16_t
// Z type:   uint16_t

// Update:   z += y
// Add func: z = x + y
// Identity: 0
// Terminal: ;

#define GB_A_TYPENAME \
    uint16_t

#define GB_Z_TYPENAME \
    uint16_t

// declare a scalar and set it equal to the monoid identity value

    #define GB_DECLARE_MONOID_IDENTITY(z)           \
        uint16_t z = 0

// reduction operator:

    // declare aij as ztype (= atype since no typecasting is done here)
    #define GB_DECLAREA(aij)  \
        uint16_t aij

    // aij = Ax [pA]
    #define GB_GETA(aij,Ax,pA,A_iso)  \
        aij = Ax [pA]

    // z += y, update
    #define GB_UPDATE(z,y) \
        z += y

    // z = x+y, additive function
    #define GB_ADD(z,x,y) \
        z = x + y

    // s += (ztype) Ax [p], no typecast here however
    #define GB_GETA_AND_UPDATE(s,Ax,p)              \
        GB_UPDATE (s, Ax [p])

// break the loop if terminal condition reached

    #define GB_MONOID_IS_TERMINAL                   \
        0

    #define GB_TERMINAL_CONDITION(z,zterminal)      \
        (false)

    #define GB_IF_TERMINAL_BREAK(z,zterminal)       \
        ;

// panel size for built-in operators

    #define GB_PANEL                                \
        64

// special case for the ANY monoid

    #define GB_IS_ANY_MONOID                        \
        0

// disable this operator and use the generic case if these conditions hold
#define GB_DISABLE \
    (GxB_NO_PLUS || GxB_NO_UINT16 || GxB_NO_PLUS_UINT16)

//------------------------------------------------------------------------------
// reduce to a non-iso matrix to scalar, for monoids only
//------------------------------------------------------------------------------

GrB_Info GB (_red__plus_uint16)
(
    uint16_t *result,
    const GrB_Matrix A,
    GB_void *restrict W_space,
    bool *restrict F,
    int ntasks,
    int nthreads
)
{ 
    #if GB_DISABLE
    return (GrB_NO_VALUE) ;
    #else
    uint16_t z = (*result) ;
    uint16_t *restrict W = (uint16_t *) W_space ;
    if (A->nzombies > 0 || GB_IS_BITMAP (A))
    {
        #include "GB_reduce_to_scalar_template.c"
    }
    else
    {
        #include "GB_reduce_panel.c"
    }
    (*result) = z ;
    return (GrB_SUCCESS) ;
    #endif
}

#endif

