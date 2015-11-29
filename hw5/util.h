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
 * 00001    alive and no neighbours
 * 00011    alive and one neighbour
 * 00110    dead and 3 neighbours
 */

#define SET_ALIVE(c) (c |=  (1 << (4)))
#define SET_DEAD(c) (c &= ~(1 << (4)))
#define IS_ALIVE(c) ((c >> 4) & 1)
#define TO_DIE (c) (c <= (char) 0x11 || c > (char) 0x13)
#define TO_SPAWN(c) (c == (char)0x3)

#define INCREMENT(__board, __i, __j) (BOARD(__board, __i, __j) += 1)
#define DECREMENT(__board, __i, __j) (BOARD(__board, __i, __j) -= 1)



#endif /* _util_h */
