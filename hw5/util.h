#ifndef _util_h
#define _util_h

/**
 * C's mod ('%') operator is mathematically correct, but it may return
 * a negative remainder even when both inputs are nonnegative.  This
 * function always returns a nonnegative remainder (x mod m), as long
 * as x and m are both positive.  This is helpful for computing
 * toroidal boundary conditions.
 */
static inline int 
mod (int x, int m)
{
  return (x < 0) ? ((x % m) + m) : (x % m);
}

/**
 * Given neighbor count and current state, return zero if cell will be
 * dead, or nonzero if cell will be alive, in the next round.
 */
static inline char 
alivep (char count, char state)
{
  return (! state && (count == (char) 3)) ||
    (state && (count >= 2) && (count <= 3));
}

/**
 * Bits represent state of a cell. Examples:
 * 00000    dead and no neighbours
 * 10000    alive and no neighbours
 * 10001    alive and one neighbour
 * 00011    dead and 3 neighbours
 */

#define SPAWN(c) (c |=  (1 << (5)))
#define PERISH(c) (c &= ~(1 << (5)))
#define ALIVE(c) ((c >> 5) & 1)
#define OVERPOPULATION(c) (c >= (char) 0x24) 
#define STARVATION(c) (c <= (char) 0x21)
#define SPAWNING(c) (c == (char)0x3)

#define INCREMENT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] ++ )
#define DECREMENT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] -- )

#endif /* _util_h */
