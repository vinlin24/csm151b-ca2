#include <iostream>

#include "cache.h"

using namespace std;

Controller::Controller() : m_MM{0}, m_stats{0}
{
    for (size_t index = 0; index < L1_CACHE_SETS; index++)
        m_L1[index].valid = false;

    for (size_t way = 0; way < VICTIM_SIZE; way++)
        m_VC[way].valid = false;

    for (size_t index = 0; index < L2_CACHE_SETS; index++)
        for (int way = 0; way < L2_CACHE_WAYS; way++)
            m_L2[index][way].valid = false;
}

void Controller::processTrace(Trace const &trace)
{
    if (trace.op == READ)
    {
        uint8_t byte = loadByte(trace.address);
        cerr << "Loaded " << static_cast<int>(byte) << "." << endl;
    }
    else
    {
        storeByte(trace.address, trace.data);
        cerr << "Stored " << static_cast<int>(trace.data) << " to "
             << trace.address << "." << endl;
    }
}

uint8_t Controller::loadByte(uint32_t address)
{
    // TODO.
}

void Controller::storeByte(uint32_t address, uint8_t byte)
{
    auto [tag, index, offset] = splitAddress(address);

    // Case A: L1 hit.

    CacheBlock &L1Block = m_L1[index];
    if (L1Block.valid && L1Block.tag == tag)
    {
        L1Block.data[offset] = byte;
        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case B: L1 Miss, VC hit.

    uint32_t VCTag = address >> 2;
    for (size_t way = 0; way < VICTIM_SIZE; way++)
    {
        CacheBlock &VCBlock = m_VC[way];
        if (!VCBlock.valid || VCBlock.tag != VCTag)
            continue;

        VCBlock.data[offset] = byte;

        // Update LRU positions. TODO: Make this into a helper.
        VCBlock.lruPosition = VICTIM_SIZE - 1;
        for (size_t i = 0; i < VICTIM_SIZE; i++)
        {
            if (i == way || m_VC[i].lruPosition == 0)
                continue;
            m_VC[i].lruPosition--;
        }

        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case C: L1 Miss, VC Miss L2 Hit.

    for (size_t way = 0; way < L2_CACHE_WAYS; way++)
    {
        CacheBlock &L2Block = m_L2[index][way];
        if (!L2Block.valid || L2Block.tag != tag)
            continue;

        L2Block.data[offset] = byte;

        // Update LRU positions. TODO: Make this into a helper.
        L2Block.lruPosition = L2_CACHE_WAYS - 1;
        for (size_t i = 0; i < L2_CACHE_WAYS; i++)
        {
            if (i == way || m_L2[index][i].lruPosition == 0)
                continue;
            m_L2[index][i].lruPosition--;
        }

        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case D: L1 Miss, VC Miss, L2 Miss.
    m_MM[address] = byte; // Write-no-allocate.
}

float Controller::getL1MissRate() const
{
    return static_cast<float>(m_stats.missL1) / m_stats.accessL1;
}

float Controller::getL2MissRate() const
{
    return static_cast<float>(m_stats.missL2) / m_stats.accessL2;
}

float Controller::getAAT() const
{
    return 0; // TODO.
}

tuple<uint32_t, uint8_t, uint8_t> Controller::splitAddress(uint32_t address)
{
    uint8_t offset = address & 0b11;
    uint8_t index = (address >> 2) & 0b1111;
    uint32_t tag = address >> 6;
    return tuple(tag, index, offset);
}

void Controller::dumpMemory() const
{
    for (size_t address = 0; address < MEM_SIZE; address++)
    {
        uint8_t byte = m_MM[address];
        if (byte == 0)
            continue;
        cerr << address << ": " << static_cast<int>(byte) << endl;
    }
}
