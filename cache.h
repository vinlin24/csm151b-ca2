#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <array>
#include <cstdint>
#include <tuple>

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

enum Operation
{
    READ,
    WRITE,
};

struct Trace
{
    Operation op;
    uint32_t address;
    uint8_t data;
};

struct Stats
{
    unsigned missL1;
    unsigned missL2;
    unsigned missVC;
    unsigned accessL1;
    unsigned accessL2;
    unsigned accessVC;
};

struct CacheBlock
{
    // The tag used to validate that this cache block actually corresponds to
    // the memory address used to access it. Computed from the remaining bits of
    // the address after computing the offset and index bits.
    uint32_t tag;

    // The actual 4-byte data to be stored at this block.
    uint8_t data[4];

    // Higher means more recently used. Only applicable to associative caches
    // with a LRU eviction policy.
    uint8_t lruPosition;

    // Flag for whether this block is currently occupied with actual data.
    bool valid;
};

class Controller
{
public:
    Controller();
    void processTrace(Trace const &trace);

    float getL1MissRate() const;
    float getL2MissRate() const;
    float getAAT() const;

    // Print the contents of main memory to stderr for debugging purposes.
    void dumpMemory() const;

private:
    // Direct-mapped L1 cache.
    CacheBlock m_L1[L1_CACHE_SETS];

    // Fully-associative victim cache.
    CacheBlock m_VC[VICTIM_SIZE];

    // Set-associative L2 cache.
    CacheBlock m_L2[L2_CACHE_SETS][L2_CACHE_WAYS];

    // Byte-addressable main memory.
    std::array<uint8_t, MEM_SIZE> m_MM;

    // Keeps track of the current load stats (access and miss counts).
    Stats m_stats;

    // TODO.
    uint8_t loadByte(uint32_t address);

    // TODO.
    void storeByte(uint32_t address, uint8_t byte);

    // Split a memory address into tag, index, offset.
    static std::tuple<uint32_t, uint8_t, uint8_t>
    splitAddress(uint32_t address);
};

#endif // CACHE_H_INCLUDED
