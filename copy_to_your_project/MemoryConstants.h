#pragma once

#include <climits>
#include <stdint.h>


// self remainder:: 1LL << x == 2 ** x

// word
typedef int word_t;

// number of bits in a word
#define WORD_WIDTH (sizeof(word_t) * CHAR_BIT)

#ifdef TEST_CONSTANTS

#define OFFSET_WIDTH 1

#define PHYSICAL_ADDRESS_WIDTH 4

#define VIRTUAL_ADDRESS_WIDTH 5

#else

// number of bits in the offset, that is, number of bits used to encode a single page in the
// hierarchy, or the frame("last page")
#define OFFSET_WIDTH 4

// number of bits in a physical address
#define PHYSICAL_ADDRESS_WIDTH 10


// number of bits in a virtual address
#define VIRTUAL_ADDRESS_WIDTH 20

#endif

// page/frame size in words
// in this implementation this is also the number of entries in a table
#define PAGE_SIZE (1LL << OFFSET_WIDTH)



// RAM size in words
#define RAM_SIZE (1LL << PHYSICAL_ADDRESS_WIDTH)


// virtual memory size in words
#define VIRTUAL_MEMORY_SIZE (1LL << VIRTUAL_ADDRESS_WIDTH)

// number of frames in the RAM
#define NUM_FRAMES (RAM_SIZE / PAGE_SIZE)

// number of pages in the virtual memory
#define NUM_PAGES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)

// TODO: why do we have '-1' and not '-OFFSET_WIDTH'?
//       maybe as an alternative to ceil
#define TABLES_DEPTH ((VIRTUAL_ADDRESS_WIDTH - 1) / OFFSET_WIDTH)
