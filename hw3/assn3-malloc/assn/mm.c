/*
 *  
 * For our implementation we started by implementing the buddy allocation system.
 * Next we tried to optimize upon this by implementing improvements such as:
 *  - lazy splitting
 *  - unaggressive merging
 * As detailed in http://www.cs.au.dk/~gerth/papers/actainformatica05.pdf
 *
 * After this we made further improvements and moved towards having segregated lists
 * based ranges of 2^k->2^(k+1)-1 staying in theme with buddy allocation. This is what 
 * our final implementation is based on because we found it most memory efficient.
 *
 * The heap is initialized to 4 blocks with the following structure:
 * [0|PR HDR|PR FTR|EPI]
 *
 * The segrated lists ranges 2^k->2^(k+1)-1 exist for k={2,3,4,5,6,7,8,9,10,11,12,13,14,15}
 * where the final list is for freed blocks of size 2^15 and above. They are pointed to by
 * global pointer array void** buddyp
 * 
 * When a block is freed we: (see mm_free) 
 * - first attempt to coalesce it with adjacent blocks (see coalesce_buddy)
 *   -> if it can be coalesced we remove the adjacent blocks from their
 *		corresponding free list
 *	 -> we continue to coalesce until the largest possible free block is obtained
 * - take the resulting block and add it to the appropriate segregated list
 *
 * When a malloc request comes in for size X we: (see mm_malloc)
 * - adjust the request size to include header and footer
 * - check segregated list of coresponding size [list for range 2^k<=X<=2^(k+1)-1]
 *	 -> check the first two entries of the list (this was an optimization)
 *		-> if it is an entry of size > X then return this
 *		-> otherwise the next range up for a match
 *	 -> if no appropriate block is found then extend the heap and return this
 * 
 * When a realloc request comes in for size X we: (see mm_realloc)
 * - if the pointer's current block size already is > X, return pointer
 * - if there is an adjacent free block of size Y and if current pointer's 
 *   block size + Y > X then
 *   -> remove adjacent block from corresponding segregated list
 *   -> coalesce them and return this
 *   -> memcpy data over to new block if the starting address of
 *      the memory block has changed
 * - otherwise allocated new block, memcpy data over and free old block
 *
 * Please see functions below for a more verbose description of each component.
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
 * Constants and Macros
 *************************************************************************/
#define WSIZE       sizeof(void *)          /* word size (bytes) */
#define DSIZE       (2 * WSIZE)				/* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)					/* initial heap size (bytes) */

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
#define PREV_FREE_BLKP(bp)  (*(void **)(bp))
#define NEXT_FREE_BLKP(bp)  (*(void **)(bp + WSIZE))
#define HEAD_OF_SEG(bp)     (*(void **)(bp))

/* Read and write a pointer at address p */
#define GETP(p)          (*(uintptr_t **)(p))
#define PUTP(p,val)      (*(uintptr_t **)(p) = (uintptr_t *)val)

#define NUM_SEG_LISTS       (14)

int    mm_check(void);
int    mm_check_init(void);
void   print_free_list(void);
void   print_implicit_list(void);

void*  heap_listp = NULL;
void*  buddyp[NUM_SEG_LISTS]; /* list of pointers to head of each segregated list of sizes by ranges of 2^k->2^k-1 */

/* buddy functions added for our implementation */
void   splice_free_block(void * bp);
void   splice_buddy(void * bp);
void * find_fit_buddy(size_t asize, size_t targsize, int isparent);
int    get_buddy_index(size_t size);
size_t next_power_of_two(size_t v);
size_t prev_power_of_two(size_t v);
void * power_coalesce(void *bp);

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 *
 * This sets up a heap of 6 blocks with the following 
 * structure:
 * [0|PR HDR|PR FTR|EPI]
 **********************************************************/
int mm_init(void)
{
	int i;
	if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
		return -1;
	PUT(heap_listp, 0);                              // alignment padding
	PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
	PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1));       // epilogue header
	heap_listp+=DSIZE;

	/* Initialize list of segregated list pointers */
	for (i = 0; i < NUM_SEG_LISTS; i++) {
		buddyp[i]=NULL;
	}

	/* Initialize free list to point at the prologue */
	//mm_check_init();

	return 0;

}

/**********************************************************
 * coalesce_buddy
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 * Always called by power_coalesce which will add the new
 * block after coalescing is complete.
 * Returns pointer to new block
 **********************************************************/
void *coalesce_buddy(void *bp) // I think we need to remove coalesced buddy from appropriate buddylist
{
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));
	size_t size_p = GET_SIZE(HDRP(PREV_BLKP(bp)));
	size_t size_n = GET_SIZE(HDRP(NEXT_BLKP(bp)));

	if (prev_alloc && next_alloc) { /* Case 1 afa*/
		/* Do nothing */
	}
	else if (!prev_alloc && !next_alloc){ /* Case 4 fff */
		size += size_p + size_n;
		splice_buddy(PREV_BLKP(bp));
		splice_buddy(NEXT_BLKP(bp));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
		PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
		bp = (PREV_BLKP(bp));
	}

	else if (prev_alloc && !next_alloc){ /* Case 2 aff*/
		size += size_n;
		splice_buddy(NEXT_BLKP(bp));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}

	else if (!prev_alloc && next_alloc){ /* Case 3 ffa*/
		size += size_p;
		splice_buddy(PREV_BLKP(bp));
		PUT(FTRP(bp), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
		bp = (PREV_BLKP(bp));
	}

	/* buddy added in power_coalesce */
	return bp;
}

/**********************************************************
 * power_coalesce
 * Continue coalescing until largest free block is formed
 * Return pointer to new superblock
 **********************************************************/
void *power_coalesce(void *bp) {
	size_t size = GET_SIZE(HDRP(bp));
	void* newp = coalesce_buddy(bp);
	size_t newsize = GET_SIZE(HDRP(newp));
	int i =0;	

	while (newsize>size) { /* should never encounter the scenario when i>1 */
		size = newsize;
		newp = coalesce_buddy(newp);
		newsize = GET_SIZE(HDRP(newp));
		i++;
	}
	add_buddy(newp);

	return newp;
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

	//printf("extend_heap: Extending heap by %d\n",size);
	if ( (bp = mem_sbrk(size)) == (void *)-1 ){
		mm_check();
		return NULL;
	}

	/* Initialize free block header/footer and the epilogue header */
	PUT(HDRP(bp), PACK(size, 0));                // free block header
	PUT(FTRP(bp), PACK(size, 0));                // free block footer
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

	return power_coalesce(bp);
}

/**********************************************************
 * split_buddy
 * Split free block into two chunks where the first chunk
 * is of size bytes
 * Updates headers of each block and attemps to coalesce
 * the second block
 * Returns pointer to first chunk
 **********************************************************/
void * split_buddy(void* bp, size_t size) {
	/* remember find_fit_buddy has already spliced block */
	void *np;
	int bpsize = GET_SIZE(HDRP(bp));	

	if (bpsize-size<=32) {
		return bp;	
	}

	PUT(HDRP(bp), PACK(size,1));
	PUT(FTRP(bp), PACK(size,1));

	np=NEXT_BLKP(bp);

	PUT(HDRP(np), PACK(bpsize-size,0));
	PUT(FTRP(np), PACK(bpsize-size,0));
	power_coalesce(np);

	return bp;
}

/**********************************************************
 * find_fit
 * Traverse the free lists to find a block of of size
 * bytes or greater
 * $optimization$ only check first two entries of first list
 * If no block found in current list, recurse on next list up
 * Return NULL if no free blocks exist of size or greater
 **********************************************************/
void * find_fit_buddy(size_t asize,size_t targsize, int isparent)
{
	void *bp;
	int index = get_buddy_index(targsize);
	size_t bestfit;
	void *np;
	size_t npfit;

	/* check if we are traversing the last list - last set of free blocks */
	if (index==13 &&(buddyp[index]==NULL || GET_SIZE(HDRP(buddyp[13])) < asize) ){ /* no suitable free blocks in any seg list -> should extend heap */
		bp=buddyp[13];
		while (bp!=NULL) {
			bestfit = GET_SIZE(HDRP(bp));
			if (bestfit>=asize) {
				np = NEXT_FREE_BLKP(bp);

				splice_buddy(bp);
				return bp;

			}
			bp=NEXT_FREE_BLKP(bp);
		}
		return NULL; /* no suitable free block found */
	}
	if (buddyp[index]==NULL) /* no free blocks in this seg list -> try next size */
		bp = find_fit_buddy(asize,next_power_of_two(targsize+1),0); /* recurse */
	else { 
		bp=buddyp[index];
		while(bp!=NULL) { /* check buddyp[0] and buddyp[1] for a fit, otherwise try next list */

			bestfit = GET_SIZE(HDRP(bp));
			if (GET_SIZE(HDRP(bp))>=asize) {
				np = NEXT_FREE_BLKP(bp); /* check if block next to head is a better fit - a cheap optimization which had a 4 percent observable improvement*/
				if (np) {
					npfit = GET_SIZE(HDRP(np));

					if (npfit>=asize && npfit < bestfit) {
						splice_buddy(np);
						return np;
					}
				}

				splice_buddy(bp);
				return bp;
			}
			bp=NEXT_FREE_BLKP(bp);
			break;
		}
		bp = find_fit_buddy(asize,next_power_of_two(targsize+1),0); /* recurse */
	}
	if (bp != NULL && GET_SIZE(HDRP(bp)) != asize){/* should split into buddies if we found a block of larger size - use &&isparent for lazy splitting*/
		split_buddy(bp, asize); 
	}

	return bp;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
	/* Get the current block size */
	size_t bsize = GET_SIZE(HDRP(bp));

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
	if (!GET_ALLOC(HDRP(bp))) {
		return;
	}
	size_t size = GET_SIZE(HDRP(bp));
	PUT(HDRP(bp), PACK(size,0));
	PUT(FTRP(bp), PACK(size,0));
	power_coalesce(bp);
}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * First look for a block in seglists
 * If no valid block exists in the free list, extend the heap
 * by size and return a pointer to this new memory block
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
	else {
		asize  = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);
	}

	/* Search the free list for a fit */
	if ((bp = find_fit_buddy(asize,prev_power_of_two(asize),1)) != NULL) {
		place(bp, asize);
		return bp;
	}

	/* No fit found. Get more memory and place the block */
	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
		return NULL;

	/* Remove new block of memory from seglist */
	splice_buddy(bp);
	/* Mark new block as allocated */
	place(bp, asize);

	return bp;

}

/**********************************************************
 * mm_realloc
 * Will find a new block of memory for ptr with a minimum 
 * of 'size' bytes.
 * Adjacent blocks are coalesced if they can be combined 
 * with ptr's block to create a block of 'size' bytes.
 * Returns pointer to new block of memory
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

	/* if blocksize already big enough just return ptr */
	if (GET_SIZE(HDRP(ptr))>size+16){
		return ptr;
	}

	void *oldptr = ptr;
	void *newptr = NULL;
	size_t copySize;

	/* check if we can coalesce! */

	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	size_t size_c = GET_SIZE(HDRP(ptr));
	size_t size_p = GET_SIZE(HDRP(PREV_BLKP(ptr)));
	size_t size_n = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
	size_t newsize;

	/* see if we can grab an adjacent free block and save time/memory */
	if (prev_alloc && !next_alloc){ /* Case 2 aff*/ // should check all use most efficient
		newsize = size_c+size_n;

		if (size_n>=size) { /* can just splice and use next block and free current */
			newptr = NEXT_BLKP(ptr);
			splice_buddy(NEXT_BLKP(newptr));

			copySize = GET_SIZE(HDRP(oldptr));
			if (size < copySize)
				copySize = size;
			memcpy(newptr, oldptr, copySize);

			mm_free(oldptr);
			return newptr;
		} else if (newsize >= size) { /* don't need to copy over in this scenario */
			splice_buddy(NEXT_BLKP(ptr));
			PUT(HDRP(ptr), PACK(newsize, 1));
			PUT(FTRP(ptr), PACK(newsize, 1));
			return ptr;
		}
	} else if (!prev_alloc && next_alloc) { /* Case ffa must copy data over */
		newsize=size_c + size_p;
		if (size_p>=size) { /* can't do size_p+size_c because would need temp buffer for memcpy */
			splice_buddy(PREV_BLKP(ptr));
			PUT(FTRP(ptr), PACK(newsize,1));
			PUT(HDRP(PREV_BLKP(ptr)), PACK(newsize,1));
			newptr=(PREV_BLKP(ptr));

			copySize=GET_SIZE(HDRP(ptr));
			if (size < copySize) {
				copySize=size;
			}
			memcpy(newptr,ptr,copySize);

			return newptr;
		}
	} 

	/* end coalesce check! */
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
 * splice_buddy
 * Remove free block based on address from corresponding seglist
 *********************************************************/
void splice_buddy(void * bp) {
	void* head;
	int index = get_buddy_index(prev_power_of_two(GET_SIZE(HDRP(bp))));

	if (buddyp[index]==NULL) { /* splice should not have been called if this seg list is empty */
		printf("ERROR splice_buddy: corresponding seglist is empty\n");
		return;
	}

	if (buddyp[index] == bp) { /* bp is the head of this seg list */
		if (NEXT_FREE_BLKP(bp)) {  /* set the next free block as head of seg list */
			buddyp[index] = NEXT_FREE_BLKP(bp);
			PREV_FREE_BLKP(NEXT_FREE_BLKP(bp)) = NULL;
		} else {               /* bp is the only block in this seg list so empty it */
			buddyp[index]=NULL;
			PREV_FREE_BLKP(bp) = NULL;
			NEXT_FREE_BLKP(bp) = NULL;
		}
	} else {                   /* bp is somewhere in seg list */

		NEXT_FREE_BLKP(PREV_FREE_BLKP(bp)) = NEXT_FREE_BLKP(bp);

		if (NEXT_FREE_BLKP(bp)!=NULL) {
			PREV_FREE_BLKP(NEXT_FREE_BLKP(bp)) = PREV_FREE_BLKP(bp);
		}
	}
}

/**********************************************************
 * add_buddy
 * Add block to head of corresponding free list
 *********************************************************/
void add_buddy(void *bp) {
	void *head;
	void *head_fp;

	int index =  get_buddy_index(prev_power_of_two(GET_SIZE(HDRP(bp))));

	if (buddyp[index]==NULL) { /* this seg list is empty */
		buddyp[index]=bp;
		PREV_FREE_BLKP(bp) = NULL;
		NEXT_FREE_BLKP(bp) = NULL;
		return;
	}

	/* add buddy to head of appropriate seg list */
	head = buddyp[index];
	PREV_FREE_BLKP(head)=bp;
	buddyp[index]=bp;
	NEXT_FREE_BLKP(bp)=head;
	PREV_FREE_BLKP(bp)=NULL;

	return;
}

/**********************************************************
 * get_buddy_index
 * Find index of appropriate seglist in -> buddyp[]
 * Return index of approriate seglist
 *********************************************************/
int get_buddy_index(size_t size) {
	switch(size) {
		case 4:
			return 0;
		case 8:
			return 1;
		case 16:
			return 2;
		case 32:
			return 3;
		case 64:
			return 4;
		case 128:                                
			return 5;
		case 256:
			return 6;
		case 512:
			return 7;
		case 1024:
			return 8;
		case 2048:
			return 9;
		case 4096:
			return 10;
		case 8192:
			return 11;
		case 16384:
			return 12;
		default:
			return 13; /* for sizes >= 32768 */
	}
}

/**********************************************************
 * buddy_exists
 * Check if block is in free list
 * Return nonzero if the block exists is in free list.
 *********************************************************/
int buddy_exists(void *bp) {
	int i;
	void* np;

	for (i=0; i<NUM_SEG_LISTS;i++) {
		np = buddyp[i];
		while(np!=NULL) {
			if (bp==np) {
				return 1;
			}
			np=NEXT_FREE_BLKP(np);
		}
	}
	return 0;
}

/**********************************************************
 * in_heap
 * Check if block's address is within heap limits
 * Return nonzero if the block is in valid address range.
 *********************************************************/
int in_heap(void *bp) {

	if (bp > mem_heap_hi() || bp < mem_heap_lo()) {
		printf("ERROR in_heap: address %p not within heap limits of mem_heap_lo=%p and mem_heap_hi=%p\n",bp,mem_heap_lo(),mem_heap_hi());
		return 0;
	}

	return 1;

}

/**********************************************************
 * mm_check_init
 * Validate mm_init -> prologue, seg lists and epilogue
 * Return nonzero if init valid
 *********************************************************/
int mm_check_init(void) {
	void * bp;
	int i;
	int success = 1;

	/* validate prologue */
	if (GET_SIZE(HDRP(heap_listp)) != DSIZE || !GET_ALLOC(HDRP(heap_listp)) && HDRP(heap_listp) != FTRP(heap_listp)) {
		printf("ERROR mm_check_init: prologue invalid %d expected %d\n",GET_SIZE(HDRP(heap_listp)),DSIZE);
		success = 0;
	}

	for (i=0; i < NUM_SEG_LISTS; i++) {
		if (buddyp[i]!=NULL){
			printf("ERROR mm_check_init: all seg lists should be empty initially buddyp[%i]=%p\n",i,buddyp[i]);
			success = 0;
		}
	}

	/* validate epilogue */
	bp = mem_heap_lo();
	if (!GET_ALLOC(bp) && GET_SIZE(bp)) {
		printf("ERROR mm_check_init: epilogue invalid\n"); 
		success = 0;
	}
	return success;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){
	void * bp;
	void * pp;
	void * np;

	size_t size;
	int i;
	int success = 1;

	bp = heap_listp;

	/*   first check each block manually			  */
	/*   are free blocks in free list?				  */
	/*   is end of last block flush with end of heap? */

	/*
	 *   don't have to worry about non-coalesced 
	 *   adjacent blocks as we allow this is part of
	 *   our design (non-aggressive merging), but
	 *   let's add a check anyway for fun
	 */

	i=0;

	while (bp!=NULL) {

		if (GET_SIZE(HDRP(bp))==0) { /* reached end of blocks, unless serious error has occured */

			if (bp-1!=mem_heap_hi()) { /* fatal error, have a block of size 0 in heap */
				printf("ERROR mm_check: reached block[%i].size 0 at bp.addr=%p but mem_heap_hi=%p\n",i,bp,mem_heap_hi());
				success=0;
			}
			break;
		}

		/* check if free block is in free list */
		if (GET_ALLOC(HDRP(bp))==0) { 			

			if (!buddy_exists(bp)) {
				printf("ERROR mm_check: Free block[%i] at addr=%p not in free list\n",i,bp);
				success=0;
			}
		}
		bp=NEXT_BLKP(bp);
		i++;

	}

	/* now check the free list								   */ 
	/* is every block in the free list marked as free?		   */
	/* does every pointer point to an address within the heap? */
	/* every head's previous pointer should be null            */
	/* checker already checks for overlapping addresses        */

	int fox=0;

	for (i=0; i<NUM_SEG_LISTS;i++) {

		bp = buddyp[i];

		if ((void**)bp!=0x0 && PREV_FREE_BLKP(bp)!=NULL) { /* make sure we do not deref a null pointer */
			printf("ERROR mm_check: head of list buddyp[%i].prev=%p has non-NULL previous pointer\n",i,PREV_FREE_BLKP(bp));	
			success=0;
		}

		while(bp != NULL) {
			if (GET_ALLOC(HDRP(bp))==1) { /* found an alloc'd block in free list */
				fox=1;			
			}

			if (PREV_FREE_BLKP(bp) && !in_heap(PREV_FREE_BLKP(bp))) {
				printf("ERROR mem_check: buddyp[%i].addr=%p has a prev pointer outside the heap\n");
			}
			if (NEXT_FREE_BLKP(bp) && !in_heap(NEXT_FREE_BLKP(bp))) {
				printf("ERROR mem_check: buddyp[%i].addr=%p has a next pointer outside the heap\n");
			}

			bp=NEXT_FREE_BLKP(bp);
		}
	}
	if (fox==1) {
		printf("ERROR mm_check: Found a fox in the hen house, alloced block in free list.\n");
		success=0;

	}

	return success;
}

/**********************************************************
 * next_power_of_two
 * Return next power of two from v.
 *********************************************************/
size_t next_power_of_two(size_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

/**********************************************************
 * mm_check
 * Return previous power of two from v.
 *********************************************************/
size_t prev_power_of_two(size_t v) {
	v |= v | (v >> 1);
	v |= v | (v >> 2);
	v |= v | (v >> 4);
	v |= v | (v >> 8);
	v |= v | (v >> 16);
	return v-(v>>1);
}
