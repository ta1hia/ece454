/*
 * The heap is initialized to 6 blocks with the following structure:
 * [0|PR HDR|PR PREV_PTR|PR NEXT_PTR|PR FTR|EPI]
 *
 * The free list is initialized to point at the prologue's PREV_PTR and
 * free blocks are inserted at the head of the free list. The prologue
 * will always be the tail of the list (so that we can check for the
 * allocated_bit == 1 when iterating through the free list, since all other
 * blocks will have allocated_bit == 0).
 *
 * Newly freed blocks are added to the free list at the end of coalescing. 
 * Freed blocks are removed from the free list during allocation in first fit
 * order.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Farfisa",
    /* First member's full name */
    "Tahia Khan",
    /* First member's email address */
    "tahia.khan@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Miro Kuratczyk",
    /* Second member's email address (leave blank if none) */
    "miro.kuratczyk@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Added macros */
#define PREV_FREE_BLKP(bp)  ((void *)(bp))
#define NEXT_FREE_BLKP(bp)  ((void *)(bp + WSIZE))
#define PUT_PREV_PTR(bp)  (*(void **)(bp))
#define PUT_NEXT_PTR(bp)  (*(void **)(bp + WSIZE))

int mm_check(void);

void* heap_listp = NULL;
void* free_listp = NULL;

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 *
 * This sets up a heap of 6 blocks with the following 
 * structure:
 * [0|PR HDR|PR PREV_PTR|PR NEXT_PTR|PR FTR|EPI]
 **********************************************************/
 int mm_init(void)
 {
   if ((heap_listp = mem_sbrk(6*WSIZE)) == (void *)-1)
         return -1;
     PUT(heap_listp, 0);                         // alignment padding
     PUT(heap_listp + (1 * WSIZE), PACK(2*DSIZE, 1));   // prologue header
     PUT(heap_listp + (2 * WSIZE), 0);   // prologue PREV_PTR
     PUT(heap_listp + (3 * WSIZE), 0);   // prologue NEXT_PTR
     PUT(heap_listp + (4 * WSIZE), PACK(2*DSIZE, 1));   // prologue footer
     PUT(heap_listp + (5 * WSIZE), PACK(0, 1));    // epilogue header
     heap_listp += DSIZE;
     free_listp= heap_listp; /* Initialize free list to point at the prologue */

     mm_check();
     return 0;
 }

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        //return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        //return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        //return (PREV_BLKP(bp));
        bp = (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        //return (PREV_BLKP(bp));
        bp = (PREV_BLKP(bp));
    }

    /* adjust free list */
    PUT_PREV_PTR(bp) = NULL;
    PUT_NEXT_PTR(bp) = free_listp;
    PUT_PREV_PTR(free_listp) = bp;
    free_listp = bp;
    
    return bp;
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    /*void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
    return NULL;*/

    void *bp;

    /* Iterate through explicit free list */
    for (bp = free_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
    return NULL;

}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
  /* Get the current block size */
  size_t bsize = GET_SIZE(HDRP(bp));

  /* splice out this block from the free list*/
  PUT_NEXT_PTR(PREV_FREE_BLKP(bp)) = NEXT_FREE_BLKP(bp);
  PUT_PREV_PTR(NEXT_FREE_BLKP(bp)) = PREV_FREE_BLKP(bp);

  PUT(HDRP(bp), PACK(bsize, 1));
  PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    if(bp == NULL){
      return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    coalesce(bp);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){
    void * bp;
    
    /* validate prologue */
    if (GET_SIZE(HDRP(heap_listp)) != 2*DSIZE && !GET_ALLOC(HDRP(heap_listp)) && HDRP(heap_listp) != FTRP(heap_listp)) {
        printf("prologue invalid");
        return 0;
    }

    /* free_list (initial check, remove later) */
    if (GET_SIZE(HDRP(free_listp)) != 2*DSIZE && !GET_ALLOC(HDRP(free_listp)) && HDRP(free_listp) != FTRP(free_listp)) {
        printf("initial free list invalid");
        return 0;
    }

    /* check every block*/
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
    }

    /* check free list */
    int num_free=0;
    for (bp = free_listp; !GET_ALLOC(HDRP(bp)); bp = NEXT_FREE_BLKP(bp)) {
        //print hdr[size:allocated] [nxt] [prev] ft[size:allocated]
        num_free += 1;

    }
    if (num_free > 0) {
        printf("free blocks: %d\n", num_free);
    }

    /* validate epilogue */
    return 1;
}



