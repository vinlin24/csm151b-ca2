#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include <cstdint>
#include <optional>

#include "cache.h"

class Controller
{
public:
    Controller();

    uint8_t loadByte(uint32_t address);
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

    void updateVictimMRU(uint8_t mruWay);
    void updateL2MRU(uint8_t setIndex, uint8_t mruWay);

    void promoteFromVC(uint8_t L1SetIndex, uint8_t VCWay);
    void promoteFromL2(uint8_t L1SetIndex, uint8_t L2SetIndex, uint8_t L2Way);
    void cacheFromMM(uint32_t address);

    std::optional<MemoryBlock> insertIntoL1(MemoryBlock const &bytes);
    std::optional<MemoryBlock> insertIntoVC(MemoryBlock const &bytes);
    std::optional<MemoryBlock> insertIntoL2(MemoryBlock const &bytes);

    MemoryBlock popFromVC(uint8_t way);
    MemoryBlock popFromL2(uint8_t index, uint8_t way);
};

#endif // CONTROLLER_H_INCLUDED
