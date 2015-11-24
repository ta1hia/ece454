/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

#define NUM_THREADS 4
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

typedef struct t_args {
    char* inboard;
    char* outboard;
    int nrows;
    int ncols;
    int gens_max;
    int quadrant;
} t_args;

pthread_cond_t  cond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int arrived = 0;

void
barrier()
{
    pthread_mutex_lock(&mutex);
    arrived++;
    if (arrived < NUM_THREADS)
        pthread_cond_wait(&cond, &mutex);
    else {
        pthread_cond_broadcast(&cond);
        arrived = 0;
    }
    pthread_mutex_unlock(&mutex);
    
}



void*
threaded_game_of_life (void* vargp)
{
    t_args *targs = (t_args*) vargp;
    int curgen, i, j;
    int nrows = targs->nrows;
    int ncols = targs->ncols;
    int rows = nrows/NUM_THREADS;
    char* inboard = targs->inboard;
    char*outboard = targs->outboard;
    const int LDA = nrows;

    int start = rows * targs->quadrant;
    int gens_max = targs->gens_max;

    for (curgen = 0; curgen < gens_max; curgen++)
    {
        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
        for (i = start; i < start+rows; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);

                const char neighbor_count = 
                    BOARD (inboard, inorth, jwest) + 
                    BOARD (inboard, inorth, j) + 
                    BOARD (inboard, inorth, jeast) + 
                    BOARD (inboard, i, jwest) +
                    BOARD (inboard, i, jeast) + 
                    BOARD (inboard, isouth, jwest) +
                    BOARD (inboard, isouth, j) + 
                    BOARD (inboard, isouth, jeast);

                BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));

            }
        }
        //INSERT BARRIER
        barrier();
        SWAP_BOARDS( outboard, inboard );

    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    //return inboard;
    
    pthread_exit(NULL);

}

char*
threaded_game_of_life_driver (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
    int i;
    t_args args[4];
    pthread_t* thrd[4];

    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < NUM_THREADS; i++) {
        args[i].inboard = inboard;
        args[i].outboard = outboard;
        args[i].nrows = nrows;
        args[i].ncols = ncols;
        args[i].gens_max = gens_max;
        args[i].quadrant = i;
        pthread_create(thrd[i], NULL, threaded_game_of_life, (void*) &args[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(*thrd[i], NULL);
    }

    return inboard;

}

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
  return threaded_game_of_life_driver (outboard, inboard, nrows, ncols, gens_max);
}
