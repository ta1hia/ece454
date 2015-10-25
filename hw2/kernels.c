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

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
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
//char rotate_descr[] = "rotate: Current working version";
//void rotate(int dim, pixel *src, pixel *dst) 
//{
//    naive_rotate(dim, src, dst);
//}


/* 
 * second attempt (commented out for now)
 char rotate_two_descr[] = "second attempt";
 void attempt_two(int dim, pixel *src, pixel *dst) 
 {
 naive_rotate(dim, src, dst);
 }
 */

char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j, tmp, tmp2, k,c,a;
    k = dim*(dim-1);


    c = k;
    a = 0;
    for (i = 0; i < dim-1;) {

        for (j = 0; j < dim; j++) {
            tmp = c-j*dim;
            tmp2 = a+j;

            dst[tmp] = src[tmp2];

            dst[tmp+1] = src[tmp2 + dim];

            dst[tmp+2] = src[tmp2 + 2*dim];

            dst[tmp+3] = src[tmp2 + 3*dim];

            dst[tmp+4] = src[tmp2 + 4*dim];

            dst[tmp+5] = src[tmp2 + 5*dim];

            dst[tmp+6] = src[tmp2 + 6*dim];

            dst[tmp+7] = src[tmp2 + 7*dim];

            dst[tmp+8] = src[tmp2 + 8*dim];

            dst[tmp+9] = src[tmp2 + 9*dim];

            dst[tmp+10] = src[tmp2 + 10*dim];

            dst[tmp+11] = src[tmp2 + 11*dim];

            dst[tmp+12] = src[tmp2 + 12*dim];

            dst[tmp+13] = src[tmp2 + 13*dim];

            dst[tmp+14] = src[tmp2 + 14*dim];

            dst[tmp+15] = src[tmp2 + 15*dim];

        }
        i+=16;
        c=k+i;
        a=i*dim;
    }

}


/* 
 * second attempt (commented out for now)
 */
char rotate_two_descr[] = "second attempt";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim/2;) {
        for (j = 0; j < dim; j++) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];

            dst[RIDX(dim-1-j, dim-i-1, dim)] = src[RIDX(dim-i-1, j, dim)];

            dst[RIDX(dim-1-j, dim-i-1-1, dim)] = src[RIDX(dim-i-1-1, j, dim)];
        }
        i+=2;
    }
}

/*
 * third attempt
 */
char rotate_three_descr[] = "third attempt";

void attempt_three(int dim, pixel *src, pixel *dst) {
    int i, j;

    for (i = 0; i < dim;) {
        for (j = 0; j < dim; j++) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];

            dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];

            dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];

        }
        i+=4;
    }
}

/*
 * fourth attempt
 */
char rotate_four_descr[] = "fourth attempt";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim;) {
        for (j = 0; j < dim; j++) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];

            dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];

            dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];

            dst[RIDX(dim-1-j, i+4, dim)] = src[RIDX(i+4, j, dim)];

            dst[RIDX(dim-1-j, i+5, dim)] = src[RIDX(i+5, j, dim)];

            dst[RIDX(dim-1-j, i+6, dim)] = src[RIDX(i+6, j, dim)];

            dst[RIDX(dim-1-j, i+7, dim)] = src[RIDX(i+7, j, dim)];

        }
        i+=8;
    }
}

/*
 * fifth attempt
 */
char rotate_five_descr[] = "fifth attempt";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim/2;) {
        for (j = 0; j < dim; j++) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];

            dst[RIDX(dim-1-j, i+2, dim)] = src[RIDX(i+2, j, dim)];

            dst[RIDX(dim-1-j, i+3, dim)] = src[RIDX(i+3, j, dim)];


            // sep

            dst[RIDX(dim-1-j, dim-i-1, dim)] = src[RIDX(dim-i-1, j, dim)];

            dst[RIDX(dim-1-j, dim-i-1-1, dim)] = src[RIDX(dim-i-1-1, j, dim)];

            dst[RIDX(dim-1-j, dim-i-1-2, dim)] = src[RIDX(dim-i-1-2, j, dim)];

            dst[RIDX(dim-1-j, dim-i-1-3, dim)] = src[RIDX(dim-i-1-3, j, dim)];
        }
        i+=4;
    }
}

/*
 * sixth attempt
 */
char rotate_six_descr[] = "sixth attempt";
void attempt_six(int dim, pixel *src, pixel *dst) {
    int i, j;

    for (i = 0; i < dim/2;) {
        for (j = 0; j < dim;) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];

            dst[RIDX(dim-1-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
            dst[RIDX(dim-1-(j+1), i+1, dim)] = src[RIDX(i+1, j+1, dim)];

            dst[RIDX(dim-1-(j+2), i, dim)] = src[RIDX(i, j+2, dim)];
            dst[RIDX(dim-1-(j+2), i+1, dim)] = src[RIDX(i+1, j+2, dim)];

            dst[RIDX(dim-1-(j+3), i, dim)] = src[RIDX(i, j+3, dim)];
            dst[RIDX(dim-1-(j+3), i+1, dim)] = src[RIDX(i+1, j+3, dim)];
            //sep
            dst[RIDX(dim-1-j, dim-i-1, dim)] = src[RIDX(dim-i-1, j, dim)];
            dst[RIDX(dim-1-j, dim-i-2, dim)] = src[RIDX(dim-i-2, j, dim)];

            dst[RIDX(dim-1-(j+1), dim-i-1, dim)] = src[RIDX(dim-i-1, j+1, dim)];
            dst[RIDX(dim-1-(j+1), dim-i-2, dim)] = src[RIDX(dim-i-2, j+1, dim)];

            dst[RIDX(dim-1-(j+2), dim-i-1, dim)] = src[RIDX(dim-i-1, j+2, dim)];
            dst[RIDX(dim-1-(j+2), dim-i-2, dim)] = src[RIDX(dim-i-2, j+2, dim)];

            dst[RIDX(dim-1-(j+3), dim-i-1, dim)] = src[RIDX(dim-i-1, j+3, dim)];
            dst[RIDX(dim-1-(j+3), dim-i-2, dim)] = src[RIDX(dim-i-2, j+3, dim)];

            j+=4;
        }
        i+=2;
    }
}

/*
 * seventh attempt
 */
char rotate_seven_descr[] = "seventh attempt";
void attempt_seven(int dim, pixel *src, pixel *dst) {

    int i, j, tmp, tmp2, k,c,a;
    k = dim*(dim-1);

    for (i = 0; i < dim;) {
        c = k+i;
        a = i*dim;
        dst[c] = src[a];
        for (j = 0; j < dim; j++) {
            tmp = c-j*dim;
            tmp2 = a+j;

            dst[tmp] = src[tmp2];

            dst[tmp+1] = src[tmp2 + dim];

            dst[tmp+2] = src[tmp2 + 2*dim];

            dst[tmp+3] = src[tmp2 + 3*dim];

            dst[tmp+4] = src[tmp2 + 4*dim];

            dst[tmp+5] = src[tmp2 + 5*dim];

            dst[tmp+6] = src[tmp2 + 6*dim];

            dst[tmp+7] = src[tmp2 + 7*dim];

            dst[tmp+8] = src[tmp2 + 8*dim];

            dst[tmp+9] = src[tmp2 + 9*dim];

            dst[tmp+10] = src[tmp2 + 10*dim];

            dst[tmp+11] = src[tmp2 + 11*dim];

            dst[tmp+12] = src[tmp2 + 12*dim];

            dst[tmp+13] = src[tmp2 + 13*dim];

            dst[tmp+14] = src[tmp2 + 14*dim];

            dst[tmp+15] = src[tmp2 + 15*dim];

            dst[tmp+16] = src[tmp2 + 16*dim];

            dst[tmp+17] = src[tmp2 + 17*dim];

            dst[tmp+18] = src[tmp2 + 18*dim];

            dst[tmp+19] = src[tmp2 + 19*dim];

            dst[tmp+20] = src[tmp2 + 20*dim];

            dst[tmp+21] = src[tmp2 + 21*dim];

            dst[tmp+22] = src[tmp2 + 22*dim];

            dst[tmp+23] = src[tmp2 + 23*dim];

            dst[tmp+24] = src[tmp2 + 24*dim];

            dst[tmp+25] = src[tmp2 + 25*dim];

            dst[tmp+26] = src[tmp2 + 26*dim];

            dst[tmp+27] = src[tmp2 + 27*dim];

            dst[tmp+28] = src[tmp2 + 28*dim];

            dst[tmp+29] = src[tmp2 + 29*dim];

            dst[tmp+30] = src[tmp2 + 30*dim];

            dst[tmp+31] = src[tmp2 + 31*dim];



        }
        i+=32;
    }
}

char rotate_eight_descr[] = "eigth attempt";
void attempt_eight(int dim, pixel *src, pixel *dst) 
{

    int i, j, tmp, tmp2, k,c,a;
    k = dim*(dim-1);


    c = k;
    a = 0;
    for (i = 0; i < dim-1;) {

        for (j = 0; j < dim; j++) {
            tmp = c-j*dim;
            tmp2 = a+j;

            dst[tmp] = src[tmp2];

            dst[tmp+1] = src[tmp2 + dim];

            dst[tmp+2] = src[tmp2 + 2*dim];

            dst[tmp+3] = src[tmp2 + 3*dim];

            dst[tmp+4] = src[tmp2 + 4*dim];

            dst[tmp+5] = src[tmp2 + 5*dim];

            dst[tmp+6] = src[tmp2 + 6*dim];

            dst[tmp+7] = src[tmp2 + 7*dim];

            dst[tmp+8] = src[tmp2 + 8*dim];

            dst[tmp+9] = src[tmp2 + 9*dim];

            dst[tmp+10] = src[tmp2 + 10*dim];

            dst[tmp+11] = src[tmp2 + 11*dim];

            dst[tmp+12] = src[tmp2 + 12*dim];

            dst[tmp+13] = src[tmp2 + 13*dim];

            dst[tmp+14] = src[tmp2 + 14*dim];

            dst[tmp+15] = src[tmp2 + 15*dim];

        }
        i+=16;
        c=k+i;
        a=i*dim;
    }

}

char rotate_nine_descr[] = "ninth attempt";
void attempt_nine(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i+=2) {
        for (j = 0; j < dim; j+=2) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
            dst[RIDX(dim-1-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
            dst[RIDX(dim-1-(j+1), i+1, dim)] = src[RIDX(i+1, j+1, dim)];

        }
    }   
}

char rotate_ten_descr[] = "ten attempt";
void attempt_ten(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i+=2) {
        for (j = 0; j < dim-1; j+=8) {
            dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
            dst[RIDX(dim-1-j, i+1, dim)] = src[RIDX(i+1, j, dim)];
            dst[RIDX(dim-1-(j+1), i, dim)] = src[RIDX(i, j+1, dim)];
            dst[RIDX(dim-1-(j+1), i+1, dim)] = src[RIDX(i+1, j+1, dim)];
            dst[RIDX(dim-1-(j+2), i, dim)] = src[RIDX(i, j+2, dim)];
            dst[RIDX(dim-1-(j+2), i+1, dim)] = src[RIDX(i+1, j+2, dim)];
            dst[RIDX(dim-1-(j+3), i, dim)] = src[RIDX(i, j+3, dim)];
            dst[RIDX(dim-1-(j+3), i+1, dim)] = src[RIDX(i+1, j+3, dim)];
            dst[RIDX(dim-1-(j+4), i, dim)] = src[RIDX(i, j+4, dim)];
            dst[RIDX(dim-1-(j+4), i+1, dim)] = src[RIDX(i+1, j+4, dim)];
            dst[RIDX(dim-1-(j+5), i, dim)] = src[RIDX(i, j+5, dim)];
            dst[RIDX(dim-1-(j+5), i+1, dim)] = src[RIDX(i+1, j+5, dim)];
            dst[RIDX(dim-1-(j+6), i, dim)] = src[RIDX(i, j+6, dim)];
            dst[RIDX(dim-1-(j+6), i+1, dim)] = src[RIDX(i+1, j+6, dim)];
            dst[RIDX(dim-1-(j+7), i, dim)] = src[RIDX(i, j+7, dim)];
            dst[RIDX(dim-1-(j+7), i+1, dim)] = src[RIDX(i+1, j+7, dim)];


        }
    }   
}

char rotate_eleven_descr[] = "eleven attempt";
void attempt_eleven(int dim, pixel *src, pixel *dst) 
{

    int i, j, tmp, tmp2, k,c,a;
    k = dim*(dim-1);

    for (i = 0; i < dim-20;) {
        c = k+i;
        a = i*dim;

        for (j = 0; j < dim; j++) {
            tmp = c-j*dim;
            tmp2 = a+j;

            dst[tmp] = src[tmp2];

            dst[tmp+1] = src[tmp2 + dim];

            dst[tmp+2] = src[tmp2 + 2*dim];

            dst[tmp+3] = src[tmp2 + 3*dim];

            dst[tmp+4] = src[tmp2 + 4*dim];

            dst[tmp+5] = src[tmp2 + 5*dim];

            dst[tmp+6] = src[tmp2 + 6*dim];

            dst[tmp+7] = src[tmp2 + 7*dim];

            dst[tmp+8] = src[tmp2 + 8*dim];

            dst[tmp+9] = src[tmp2 + 9*dim];

            dst[tmp+10] = src[tmp2 + 10*dim];

            dst[tmp+11] = src[tmp2 + 11*dim];

            dst[tmp+12] = src[tmp2 + 12*dim];

            dst[tmp+13] = src[tmp2 + 13*dim];

            dst[tmp+14] = src[tmp2 + 14*dim];

            dst[tmp+15] = src[tmp2 + 15*dim];

            dst[tmp+16] = src[tmp2 + 16*dim];

            dst[tmp+17] = src[tmp2 + 17*dim];

            dst[tmp+18] = src[tmp2 + 18*dim];

            dst[tmp+19] = src[tmp2 + 19*dim];

            dst[tmp+20] = src[tmp2 + 20*dim];

            dst[tmp+21] = src[tmp2 + 21*dim];

        }
        i+=22;
    }
    for (;i<dim-1;i+=2) {
        c = k+i;
        a = i*dim;
        for (j=0;j<dim;j++) {
            tmp = c-j*dim;
            tmp2 = a+j;
            dst[tmp++]=src[tmp2];

            dst[tmp]=src[tmp2+=dim];
        }
    }

}

char rotate_twelve_descr[] = "attempt twelve:";
void attempt_twelve(int dim, pixel *src, pixel *dst) 
{

//#define RIDX(i,j,n) ((i)*(n)+(j))
//dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
    int i, j, tmp, tmp2, k,c,a, i1, j1, T;
    k = dim*(dim-1);

    T = 16;
    c = k;
    a = 0;

    for (i1 = 0; i1 < dim; i1+=T) {
        for (j1 = 0; j1 < dim; j1+=T) {
            for (i = i1-T; i < i1+T;) {

                for (j =j1; j < j1+T; j++) {
                    tmp = c-j*dim;
                    tmp2 = a+j;

                    dst[tmp] = src[tmp2];

                    dst[tmp+1] = src[tmp2 + dim];

                    dst[tmp+2] = src[tmp2 + 2*dim];

                    dst[tmp+3] = src[tmp2 + 3*dim];

                    dst[tmp+4] = src[tmp2 + 4*dim];

                    dst[tmp+5] = src[tmp2 + 5*dim];

                    dst[tmp+6] = src[tmp2 + 6*dim];

                    dst[tmp+7] = src[tmp2 + 7*dim];

                    dst[tmp+8] = src[tmp2 + 8*dim];

                    dst[tmp+9] = src[tmp2 + 9*dim];

                    dst[tmp+10] = src[tmp2 + 10*dim];

                    dst[tmp+11] = src[tmp2 + 11*dim];

                    dst[tmp+12] = src[tmp2 + 12*dim];

                    dst[tmp+13] = src[tmp2 + 13*dim];

                    dst[tmp+14] = src[tmp2 + 14*dim];

                    dst[tmp+15] = src[tmp2 + 15*dim];

                }
                i+=16;
                c=k+i;
                a=i*dim;
            }
        }
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
    add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_two_descr);   
    //add_rotate_function(&attempt_three, rotate_three_descr);   
    //add_rotate_function(&attempt_four, rotate_four_descr);   
    //add_rotate_function(&attempt_five, rotate_five_descr);   
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   
    add_rotate_function(&attempt_twelve, rotate_twelve_descr);   

    /* ... Register additional rotate functions here */
}

