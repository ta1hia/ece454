/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "TeamName",              /* Team name */

    "student1",     /* First member full name */
    "student1@ecf.utoronto.ca",  /* First member email address */

    "Tahia Khan",                   /* Second member full name (leave blank if none) */
    "tahia.khan@mail.utoronto.ca"   /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    naive_rotate(dim, src, dst);
}


/* 
 * second attempt 
*/
char rotate_two_descr[] = "second attempt: blocking only";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i, j, i1, j1, T;
    T=16;

    if (dim >= 1024)
        T = 128;

    for (i = 0; i < dim; i+=T)
	    for (j = 0; j < dim; j+=T)
            for (i1 = i; i1 < i+T ; i1++)
                for (j1 = j; j1 < j+T ; j1++)  
	                dst[(dim-1-j1)*dim+i1] = src[i1*dim+j1];
}

/* 
 * third attempt 
*/
char rotate_three_descr[] = "third attempt: remove loop invariant";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    // intermediate values
    int intmd1;
    pixel *src_copy;

    for (i = 0; i < dim; i++) {
        intmd1 = dim*dim-dim + i;
        src_copy = src+(i*dim);
	    for (j = 0; j < dim; j++)
	        dst[intmd1-j*dim] = src_copy[j];
    }
}

/* 
 * fourth attempt
 */
char rotate_four_descr[]= "fourth attempt: function inline RIDX";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	    for (j = 0; j < dim; j++)
	        dst[(dim-1-j)*dim+i] = src[i*dim+j];
}

/* 
 * fifth attempt
 */
char rotate_five_descr[]= "fifth attempt: loop reorder i and j";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j++)
	    for (i = 0; i < dim; i++)
	        dst[(dim-1-j)*dim+i] = src[i*dim+j];
}

/* 
 * sixth attempt
 */
char rotate_six_descr[]= "sixth attempt: loop unrolling and function inlining";
void attempt_six(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j+=4)
	    for (i = 0; i < dim; i+=4) {
            //j += 0
	        dst[(dim - 1 - j) * dim + i] = src[i * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 1)] = src[(i + 1) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 2)] = src[(i + 2) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 3)] = src[(i + 3) * dim + j];

            //j += 1
	        dst[(dim - 1 - (j + 1)) * dim + i] = src[i * dim + (j + 1)];
	        dst[(dim - 1 - (j + 1)) * dim + (i + 1)] = src[(i + 1) * dim + (j + 1)];
	        dst[(dim - 1 - (j + 1)) * dim + (i + 2)] = src[(i + 2) * dim + (j + 1)];
	        dst[(dim - 1 - (j + 1)) * dim + (i + 3)] = src[(i + 3) * dim + (j + 1)];

            //j += 2
	        dst[(dim - 1 - (j + 2)) * dim + i] = src[i * dim + (j + 2)];
	        dst[(dim - 1 - (j + 2)) * dim + (i + 1)] = src[(i + 1) * dim + (j + 2)];
	        dst[(dim - 1 - (j + 2)) * dim + (i + 2)] = src[(i + 2) * dim + (j + 2)];
	        dst[(dim - 1 - (j + 2)) * dim + (i + 3)] = src[(i + 3) * dim + (j + 2)];

            //j += 3
	        dst[(dim - 1 - (j + 3)) * dim + i] = src[i * dim + (j + 3)];
	        dst[(dim - 1 - (j + 3)) * dim + (i + 1)] = src[(i + 1) * dim + (j + 3)];
	        dst[(dim - 1 - (j + 3)) * dim + (i + 2)] = src[(i + 2) * dim + (j + 3)];
	        dst[(dim - 1 - (j + 3)) * dim + (i + 3)] = src[(i + 3) * dim + (j + 3)];
        }
}

/* 
 * seventh attempt
 */
char rotate_seven_descr[]= "seventh attempt: bigger loop unrolling";
void attempt_seven(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j+=8)
	for (i = 0; i < dim; i+=8) {
        //j += 0
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
	    dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
	    dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];
	    dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];
	    dst[RIDX(dim-1-j, i+4, dim)] = src[RIDX(i+4, j, dim)];
	    dst[RIDX(dim-1-j, i+5, dim)] = src[RIDX(i+5, j, dim)];
	    dst[RIDX(dim-1-j, i+6, dim)] = src[RIDX(i+6, j, dim)];
	    dst[RIDX(dim-1-j, i+7, dim)] = src[RIDX(i+7, j, dim)];

        //j += 1
	    dst[RIDX(dim-1-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+1, dim)] = src[RIDX(i+1, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+2, dim)] = src[RIDX(i+2, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+3, dim)] = src[RIDX(i+3, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+4, dim)] = src[RIDX(i+4, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+5, dim)] = src[RIDX(i+5, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+6, dim)] = src[RIDX(i+6, j+1, dim)];
	    dst[RIDX(dim-1-(j+1), i+7, dim)] = src[RIDX(i+7, j+1, dim)];

        //j += 2
	    dst[RIDX(dim-1-(j+2), i, dim)] = src[RIDX(i, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+1, dim)] = src[RIDX(i+1, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+2, dim)] = src[RIDX(i+2, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+3, dim)] = src[RIDX(i+3, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+4, dim)] = src[RIDX(i+4, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+5, dim)] = src[RIDX(i+5, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+6, dim)] = src[RIDX(i+6, j+2, dim)];
	    dst[RIDX(dim-1-(j+2), i+7, dim)] = src[RIDX(i+7, j+2, dim)];

        //j += 3
	    dst[RIDX(dim-1-(j+3), i, dim)] = src[RIDX(i, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+1, dim)] = src[RIDX(i+1, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+2, dim)] = src[RIDX(i+2, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+3, dim)] = src[RIDX(i+3, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+4, dim)] = src[RIDX(i+4, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+5, dim)] = src[RIDX(i+5, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+6, dim)] = src[RIDX(i+6, j+3, dim)];
	    dst[RIDX(dim-1-(j+3), i+7, dim)] = src[RIDX(i+7, j+3, dim)];

        //j += 4
	    dst[RIDX(dim-1-(j+4), i, dim)] = src[RIDX(i, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+1, dim)] = src[RIDX(i+1, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+2, dim)] = src[RIDX(i+2, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+3, dim)] = src[RIDX(i+3, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+4, dim)] = src[RIDX(i+4, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+5, dim)] = src[RIDX(i+5, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+6, dim)] = src[RIDX(i+6, j+4, dim)];
	    dst[RIDX(dim-1-(j+4), i+7, dim)] = src[RIDX(i+7, j+4, dim)];

        //j += 5
	    dst[RIDX(dim-1-(j+5), i, dim)] = src[RIDX(i, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+1, dim)] = src[RIDX(i+1, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+2, dim)] = src[RIDX(i+2, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+3, dim)] = src[RIDX(i+3, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+4, dim)] = src[RIDX(i+4, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+5, dim)] = src[RIDX(i+5, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+6, dim)] = src[RIDX(i+6, j+5, dim)];
	    dst[RIDX(dim-1-(j+5), i+7, dim)] = src[RIDX(i+7, j+5, dim)];

        //j += 6
	    dst[RIDX(dim-1-(j+6), i, dim)] = src[RIDX(i, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+1, dim)] = src[RIDX(i+1, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+2, dim)] = src[RIDX(i+2, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+3, dim)] = src[RIDX(i+3, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+4, dim)] = src[RIDX(i+4, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+5, dim)] = src[RIDX(i+5, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+6, dim)] = src[RIDX(i+6, j+6, dim)];
	    dst[RIDX(dim-1-(j+6), i+7, dim)] = src[RIDX(i+7, j+6, dim)];

        //j += 7
	    dst[RIDX(dim-1-(j+7), i, dim)] = src[RIDX(i, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+1, dim)] = src[RIDX(i+1, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+2, dim)] = src[RIDX(i+2, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+3, dim)] = src[RIDX(i+3, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+4, dim)] = src[RIDX(i+4, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+5, dim)] = src[RIDX(i+5, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+6, dim)] = src[RIDX(i+6, j+7, dim)];
	    dst[RIDX(dim-1-(j+7), i+7, dim)] = src[RIDX(i+7, j+7, dim)];
    }
}


/* 
 * eighth attempt
 */
char rotate_eight_descr[]= "eighth attempt: loop unrolling, function inlining and blocking (incomplete)";
void attempt_eigth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (j = 0; j < dim; j+=4)
	    for (i = 0; i < dim; i+=4) {
            //j += 0
	        dst[(dim - 1 - j) * dim + i] = src[i * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 1)] = src[(i + 1) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 2)] = src[(i + 2) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 3)] = src[(i + 3) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 4)] = src[(i + 4) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 5)] = src[(i + 5) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 6)] = src[(i + 6) * dim + j];
	        dst[(dim - 1 - j) * dim + (i + 7)] = src[(i + 7) * dim + j];

            //j += 1
	        dst[(dim - 1 - (j + 1)) * dim + i] = src[i * dim + (j + 1)];
	        dst[(dim - 1 - (j + 1)) * dim + (i + 1)] = src[(i + 1) * dim + (j + 1)];
	        dst[(dim - 1 - (j + 1)) * dim + (i + 2)] = src[(i + 2) * dim + (j + 1)];
	        dst[(dim - 1 - (j + 1)) * dim + (i + 3)] = src[(i + 3) * dim + (j + 1)];

            //j += 2
	        dst[(dim - 1 - (j + 2)) * dim + i] = src[i * dim + (j + 2)];
	        dst[(dim - 1 - (j + 2)) * dim + (i + 1)] = src[(i + 1) * dim + (j + 2)];
	        dst[(dim - 1 - (j + 2)) * dim + (i + 2)] = src[(i + 2) * dim + (j + 2)];
	        dst[(dim - 1 - (j + 2)) * dim + (i + 3)] = src[(i + 3) * dim + (j + 2)];

            //j += 3
	        dst[(dim - 1 - (j + 3)) * dim + i] = src[i * dim + (j + 3)];
	        dst[(dim - 1 - (j + 3)) * dim + (i + 1)] = src[(i + 1) * dim + (j + 3)];
	        dst[(dim - 1 - (j + 3)) * dim + (i + 2)] = src[(i + 2) * dim + (j + 3)];
	        dst[(dim - 1 - (j + 3)) * dim + (i + 3)] = src[(i + 3) * dim + (j + 3)];
        }
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    //add_rotate_function(&rotate, rotate_descr);   

    add_rotate_function(&attempt_two, rotate_two_descr);   
    add_rotate_function(&attempt_three, rotate_three_descr);   
    add_rotate_function(&attempt_four, rotate_four_descr);   
    add_rotate_function(&attempt_five, rotate_five_descr);   
    add_rotate_function(&attempt_six, rotate_six_descr);   
    add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

