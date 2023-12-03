#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <cstdint>
#include <optional>

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

class L1Cache
{
public:
    L1Cache();

    std::optional<uint8_t> readByte(uint32_t address) const;
    bool writeByte(uint32_t address, uint8_t byte);

private:
    CacheBlock m_blocks[L1_CACHE_SETS];
};

class L2Cache
{
public:
    L2Cache();

    std::optional<uint8_t> readByte(uint32_t address);
    bool writeByte(uint32_t address, uint8_t byte);

private:
    CacheBlock m_blocks[L2_CACHE_SETS][L2_CACHE_WAYS];
    void updateMRU(size_t index, size_t way);
};

class VictimCache
{
public:
    VictimCache();

    std::optional<uint8_t> readByte(uint32_t address);
    bool writeByte(uint32_t address, uint8_t byte);

private:
    CacheBlock m_blocks[VICTIM_SIZE];
    void updateMRU(size_t way);
};

#endif // CACHE_H_INCLUDED
