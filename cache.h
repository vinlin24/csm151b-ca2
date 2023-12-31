#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <cstdint>

// Number of sets (indexable rows) in our L1 cache design.
#define L1_CACHE_SETS 16

// Number of sets (indexable rows) in our L2 cache design.
#define L2_CACHE_SETS 16

// Number of ways (columns) per set in our L2 cache design.
#define L2_CACHE_WAYS 8

// Bytes per block.
#define BLOCK_SIZE 4

// Number of entries in our victim cache design.
#define VICTIM_SIZE 4

// Number of addressable bytes in our main memory design.
#define MEM_SIZE 4096

// Hit time of L1 cache in cycles, as stated in the spec.
#define L1_HIT_TIME 1

// Hit time of victim cache in cycles, as stated in the spec.
#define VC_HIT_TIME 1

// Hit time of L2 cache in cycles, as stated in the spec.
#define L2_HIT_TIME 8

// Access time of main memory in cycles, as stated in the spec.
#define MM_ACCESS_TIME 100

// The finest level of granularity that our cache operations will operate on.
// Our cache design also assumes that every line is one block.
struct CacheBlock
{
    // The tag used to validate that this cache block actually corresponds to
    // the memory address used to access it. Computed from the remaining bits of
    // the address after computing the offset and index bits.
    uint32_t tag;

    // The actual bytes to be stored at this block.
    uint8_t data[BLOCK_SIZE];

    // Higher means more recently used. Only applicable to associative caches
    // with a LRU eviction policy.
    uint8_t lruPosition;

    // Flag for whether this block is currently occupied with actual data.
    bool valid;
};

// The split version of a memory address as tag, index, offset. This is assuming
// a 16-set cache (4 bit index) and 4-byte block size (2 bit offset).
struct AddressParts
{
    AddressParts(uint32_t address)
        : tag(address >> 6),
          index((address >> 2) & 0b1111),
          offset(address & 0b11) {}

    uint32_t tag;
    uint8_t index;
    uint8_t offset;
};

// A cache-agnostic struct for a collection of 4 contiguous bytes in memory.
// This serves as the "raw" form of a memory block used when bytes need to be
// moved between cache layers with potentially different address segmentation.
struct MemoryBlock
{
    uint32_t address; // 4-byte aligned.
    uint8_t data[BLOCK_SIZE];
};

#endif // CACHE_H_INCLUDED
