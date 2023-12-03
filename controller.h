#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include <array>
#include <cstdint>

#include "cache.h"

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
    L1Cache m_L1;

    // Fully-associative victim cache.
    L2Cache m_L2;

    // Set-associative L2 cache.
    VictimCache m_VC;

    // Byte-addressable main memory.
    std::array<uint8_t, MEM_SIZE> m_MM;

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

    // TODO.
    uint8_t loadByte(uint32_t address);

    // TODO.
    void storeByte(uint32_t address, uint8_t byte);
};

#endif // CONTROLLER_H_INCLUDED
