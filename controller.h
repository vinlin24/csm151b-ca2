#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include <cstdint>
#include <optional>

#include "cache.h"

class Controller
{
public:
    Controller();

    // Walk the memory hierarchy to load the byte at the specified address.
    uint8_t loadByte(uint32_t address);

    // Store a byte at the specified address, updating the memory hierarchy as
    // appropriate for our write-through, write-no-allocate strategies.
    void storeByte(uint32_t address, uint8_t byte);

    double getL1MissRate() const;
    double getL2MissRate() const;
    double getAAT() const;

    // Print the contents of main memory to stderr for debugging purposes.
    void dumpMemory() const;

    // Print the blocks current in each cache to stderr for debugging purposes.
    void dumpCacheState() const;

private:
    // Direct-mapped L1 cache.
    CacheBlock m_L1[L1_CACHE_SETS];

    // Fully-associative victim cache.
    CacheBlock m_VC[VICTIM_SIZE];

    // Set-associative L2 cache.
    CacheBlock m_L2[L2_CACHE_SETS][L2_CACHE_WAYS];

    // Byte-addressable main memory.
    uint8_t m_MM[MEM_SIZE];

    // Keeps track of the current load stats (access and miss counts).
    struct Stats
    {
        unsigned missL1;
        unsigned missL2;
        unsigned missVC;
        unsigned accessL1;
        unsigned accessL2;
        unsigned accessVC;
    } m_stats;

    // Set the block at the specifed way in the victim cache to be the MRU
    // block, and update the LRU positions of the other blocks appropriately.
    void updateVictimMRU(uint8_t mruWay);

    // Set the block at the specified way in the specified set in the L2 cache
    // to be the MRU block, and update the LRU positions of the other blocks
    // within the set appropriately.
    void updateL2MRU(uint8_t setIndex, uint8_t mruWay);

    // Invalidate the block at the specified way in the victim cache and return
    // the memory that was at it.
    MemoryBlock popFromVC(uint8_t way);

    // Invalidate the block at the specified set and way in the L2 cache and
    // return the memory that was at it.
    MemoryBlock popFromL2(uint8_t index, uint8_t way);

    // Insert a block of memory into the L1 cache. If an eviction is required,
    // the evicted block is automatically demoted through `insertIntoVC`. Return
    // the memory that was evicted from the entire cache if the demotion fell
    // through the LLC.
    std::optional<MemoryBlock> insertIntoL1(MemoryBlock const &bytes);

    // Insert a block of memory into the victim cache. If an eviction is
    // required, the evicted block is automatically demoted through
    // `insertIntoL2`. Return the memory that was evicted from the entire cache
    // if the demotion fell through the LLC.
    std::optional<MemoryBlock> insertIntoVC(MemoryBlock const &bytes);

    // Insert a block of memory into the L2 cache. Return the memory that was
    // evicted from the entire cache if the demotion fell through the LLC.
    std::optional<MemoryBlock> insertIntoL2(MemoryBlock const &bytes);
};

#endif // CONTROLLER_H_INCLUDED
