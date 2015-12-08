/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 *
 * Tahia Khan
 * 998897216
 * tahia.khan@mail.utoronto.ca
 *
 * Miro Kuratczyk
 * 997575137
 * miro.kuratczyk@mail.utoronto.ca
 *
 * Report located at ./report
 *
 * In our optimized version of the GOL keep a cell's state and neighbour count in each cell:
 * 
 * Bits represent state of a cell. Examples:
 * 000000    dead and no neighbours
 * 100000    alive and no neighbours
 * 100001    alive and one neighbour
 * 000011    dead and 3 neighbours
 *
 * To do this we:
 * - first process the inboard to follow this paradigm
 *   - if a cell is alive increment the count of its neighbours
 * - keep computing each geneartion until gens_max is reached
 * - restore the board to the original format of 1s & 0s
 * - return this
 *
 * Now when we iterate through the board we can:
 * - check if cell is to spawn or die
 *   - if spawning increment neighbours
 *   - if dying decrement neighbours
 *   - if neither we do nothing and save ourselves performing needless 
 *     read/write cycles
 *
 * This is an optimization because when processing the board you can check the
 * cell's neighbour count to determine if it should be dead or alive instead
 * of looking at each neighbours value (1 or 0) individually. This reduces the
 * number of reads/writes required significantly.
 *
 * We also parallelized our implementation with NUM_THREADS (= 4) threads. To
 * do this we seperate the board into numrows/NUM_THREADS chunks and then
 * process these chunks individually in NUM_THREADS (= 4) threads. This is done 
 * the threaded_game_of_life function.
 *
 * To do this we first need to process the row boundaries seperately. This was
 * parallelized and computed with the rowwerkers function. Then we can process
 * the individual chunks and continue onto the next generation.
 *
 * Beyond these aforementioned optimizations we also used:
 * - function inlining
 * - LICM
 * - optimizing for cache/memory access performance
 *   
 * Performance bottle necks were analyzed with: 
 * > perf record -e cpu-cycles,cache-misses ./gol 1000 inputs/1k.pbm outputs/1k.pbm
 *
 * Although we achieved a ~20 times performance increase, the hashlife implementation
 * is still far superior to ours.
 *
 ****************************************************************************/

#include "life.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*****************************************************************************
 * Constants and Macros
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

#define BOARD( __board, __i, __j )  (__board[(__i) + nrows*(__j)])

/*****************************************************************************
 * Globals
 ****************************************************************************/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************
 * Types
 ****************************************************************************/ 
typedef struct t_args {
    char* inboard;
    char* outboard;
    int nrows;
    int ncols;
    int start;
    int end;
} t_args;

/*****************************************************************************
 * game_of_life
 * Processes board sequentially or in parallel depending on dimensions by
 * the rules of Conway's 'Game of Life'
 *
 * returns board in final state after gens_max generations are computed
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
    /* lab spec said boardsize would be multiples of 32 */
    if (nrows < 32)
        return sequential_game_of_life(outboard, inboard, nrows, ncols, gens_max);
    else
        return threaded_game_of_life_driver (outboard, inboard, nrows, ncols, gens_max);
}

/*****************************************************************************
 * threaded_game_of_life_driver
 * Split board up into NUM_THREADS sections and compute these in parallel
 * - first compute rows which divide sections in parallel (mutex lock was needed)
 * - then compute sections in parallel
 * - keep iterating until gens_max reached
 *
 * returns board in final state after gens_max generations are computed
 ****************************************************************************/
char*
threaded_game_of_life_driver (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    int i,j;
    int curgen = 0;
    t_args *args = malloc(sizeof(t_args)*NUM_THREADS);
    t_args *startwerkers = malloc(sizeof(t_args)*(NUM_THREADS)); // struct for upper row boundary
    t_args *endwerkers = malloc(sizeof(t_args)*(NUM_THREADS));   // struct for lower row boundary

    pthread_t thrd[NUM_THREADS];

    pthread_mutex_init(&mutex, NULL); // lock for rowwerker
    
    // Let us initialize our board
    for (i = 0; i < nrows; i++) {
        for (j = 0; j< ncols; j++) {
            // Update cell count of neighbours
            if (ALIVE(BOARD(inboard, i, j))) {
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
                INCREMENT(inboard, inorth, jwest);
                INCREMENT (inboard, inorth, j);
                INCREMENT (inboard, inorth, jeast);
                INCREMENT (inboard, i, jwest);
                INCREMENT (inboard, i, jeast);
                INCREMENT (inboard, isouth, jwest);
                INCREMENT (inboard, isouth, j);
                INCREMENT (inboard, isouth, jeast);
            }
        }
    }

    int start = 0;
    int blockend = nrows/NUM_THREADS;
    int end = blockend;
    for (i=0; i<NUM_THREADS; i++){
        // precompute start and end of each chunk
        args[i].start = start+1;
        args[i].end = end-1;
        args[i].nrows = nrows;
        args[i].ncols = ncols;

        // precompute boundaries of each chunk
        startwerkers[i].start = start;
        startwerkers[i].nrows = nrows;
        startwerkers[i].ncols = ncols;

        endwerkers[i].start = end-1;
        endwerkers[i].nrows = nrows;
        endwerkers[i].ncols = ncols;

        // step
        start = end;
        end = end + blockend;
    }

    for(curgen = 0; curgen < gens_max; curgen++) {

        memmove(outboard,inboard,nrows * ncols * sizeof(char));
       
        int threadcnt=0;

        /* first we process boundaries between chunks */

        // best to do NUM_THREADS threads at a time
        // so we do NUM_THREADS/2 start chunk rows
        // and NUM_THREADS/2 end chunk rows
        for (i = 0; threadcnt < NUM_THREADS;i++) {
            startwerkers[i].inboard = inboard;
            startwerkers[i].outboard = outboard;
            endwerkers[i].inboard = inboard;
            endwerkers[i].outboard = outboard;

            pthread_create(&thrd[threadcnt++], NULL, rowwerker, (void*) &startwerkers[i]);
            pthread_create(&thrd[threadcnt++], NULL, rowwerker, (void*) &endwerkers[i]);
        }

        for (i = 0; i < NUM_THREADS; i++) {
            pthread_join(thrd[i], NULL);
        }

        threadcnt=0;

        // best to do NUM_THREADS threads at a time
        // so we do NUM_THREADS/2 start chunk rows
        // and NUM_THREADS/2 end chunk rows
        for (i = NUM_THREADS/2; threadcnt < NUM_THREADS;i++) {
            startwerkers[i].inboard = inboard;
            startwerkers[i].outboard = outboard;
            endwerkers[i].inboard = inboard;
            endwerkers[i].outboard = outboard;

            pthread_create(&thrd[threadcnt++], NULL, rowwerker, (void*) &startwerkers[i]);
            pthread_create(&thrd[threadcnt++], NULL, rowwerker, (void*) &endwerkers[i]);
        }
        for (i = 0; i < NUM_THREADS; i++) {
            pthread_join(thrd[i], NULL);
        }

        /* now we process chunks */
        for (i = 0; i < NUM_THREADS; i++) {
            args[i].inboard = inboard;
            args[i].outboard = outboard;
            pthread_create(&thrd[i], NULL, threaded_game_of_life, (void*) &args[i]);
        }

        for (i = 0; i < NUM_THREADS; i++) {
            pthread_join(thrd[i], NULL);
        }

        SWAP_BOARDS( outboard, inboard );
    }

    /* revert board to state of 1s and 0s */
    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++) {
            BOARD(inboard,i,j) = BOARD(inboard,i,j) >> 5;
        }
    }

    return inboard;

}

/*****************************************************************************
 * threaded_game_of_life
 *
 * Function which will be run by NUM_THREADS threads per geneartion.
 * Operates on a chunk updating neighbour counts when cells die or spawn
 * This reduces the number of reades/writes needed per cell
 ****************************************************************************/
void*
threaded_game_of_life (void* vargp)
{
    t_args *targs = (t_args*) vargp;
    int i, j;
    int nrows = targs->nrows;
    int ncols = targs->ncols;
    char* inboard = targs->inboard;
    char* outboard = targs->outboard;

    int start = targs->start;
    int end = targs->end;

        for (j = 0; j < ncols; j++)
        {
            for (i = start; i < end; i++)
            {
                char cell = BOARD(inboard,i,j);

                if (!ALIVE(cell)) {
                    if (SPAWNING(cell)) { // see if cell will come alive

                    // spawn cell
                    SPAWN(BOARD(outboard,i,j));

                    //increment all neighbours
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);
                    INCREMENT (outboard, inorth, jwest);
                    INCREMENT (outboard, inorth, j);
                    INCREMENT (outboard, inorth, jeast);
                    INCREMENT (outboard, i, jwest);
                    INCREMENT (outboard, i, jeast);
                    INCREMENT (outboard, isouth, jwest);
                    INCREMENT (outboard, isouth, j);
                    INCREMENT (outboard, isouth, jeast);
                    }
                }
                else if (STARVATION(cell) || OVERPOPULATION(cell)) { // see if cell will die

                    //set cell to be dead
                    PERISH(BOARD(outboard,i,j));

                    //decrement all neighbours
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);
                    DECREMENT (outboard, inorth, jwest);
                    DECREMENT (outboard, inorth, j);
                    DECREMENT (outboard, inorth, jeast);
                    DECREMENT (outboard, i, jwest);
                    DECREMENT (outboard, i, jeast);
                    DECREMENT (outboard, isouth, jwest);
                    DECREMENT (outboard, isouth, j);
                    DECREMENT (outboard, isouth, jeast);
                }
            }

    }
    pthread_exit(NULL);

}

/*****************************************************************************
 * rowwerker
 *
 * Function which will be run by NUM_THREADS threads per geneartion.
 * Operates on row boundaries updating neighbour counts when cells die or spawn
 * This reduces the number of reades/writes needed per cell
 ****************************************************************************/
void* rowwerker (void* werkerp) {
    t_args *targs = (t_args*) werkerp;
    int j;
    int ncols = targs->ncols;
    int nrows = targs->nrows;
    char* inboard = targs->inboard;
    char* outboard = targs->outboard;
    int i = targs->start;

    for (j = 0; j < ncols; j++) {
        char cell = BOARD(inboard,i,j);
        if (!ALIVE(cell)) {
            if (SPAWNING(cell)) { // see if cell will come alive

                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
                pthread_mutex_lock(&mutex);
                // spawn cell
                SPAWN(BOARD(outboard,i,j));

                // increment all neighbours
                INCREMENT (outboard, inorth, jwest);
                INCREMENT (outboard, inorth, j);
                INCREMENT (outboard, inorth, jeast);
                INCREMENT (outboard, i, jwest);
                INCREMENT (outboard, i, jeast);
                INCREMENT (outboard, isouth, jwest);
                INCREMENT (outboard, isouth, j);
                INCREMENT (outboard, isouth, jeast);
                pthread_mutex_unlock(&mutex);
            }
        } else {
            if (STARVATION(cell) || OVERPOPULATION(cell)) { // see if cell will die

                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
                pthread_mutex_lock(&mutex);

                // set cell to be dead
                PERISH(BOARD(outboard,i,j));

                // decrement all neighbours
                DECREMENT (outboard, inorth, jwest);
                DECREMENT (outboard, inorth, j);
                DECREMENT (outboard, inorth, jeast);
                DECREMENT (outboard, i, jwest);
                DECREMENT (outboard, i, jeast);
                DECREMENT (outboard, isouth, jwest);
                DECREMENT (outboard, isouth, j);
                DECREMENT (outboard, isouth, jeast);
                pthread_mutex_unlock(&mutex);
            }
        }
    }

    pthread_exit(NULL);
}

