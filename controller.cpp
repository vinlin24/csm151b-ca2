#include "controller.h"

#include <iostream>

using namespace std;

Controller::Controller() : m_stats{0, 0, 0, 0, 0, 0}
{
    for (size_t index = 0; index < L1_CACHE_SETS; index++)
        m_L1[index].valid = false;

    for (size_t way = 0; way < VICTIM_SIZE; way++)
    {
        CacheBlock &block = m_VC[way];
        block.valid = false;
        // TODO: IDK if this is the best way to initialize this.
        block.lruPosition = way;
    }

    for (size_t index = 0; index < L2_CACHE_SETS; index++)
    {
        for (int way = 0; way < L2_CACHE_WAYS; way++)
        {
            CacheBlock &block = m_L2[index][way];
            block.valid = false;
            // TODO: IDK if this is the best way to initialize this.
            block.lruPosition = way;
        }
    }

    for (size_t address = 0; address < MEM_SIZE; address++)
        m_MM[address] = 0;
}

uint8_t Controller::loadByte(uint32_t address)
{
    return 0; // TODO.
}

void Controller::storeByte(uint32_t address, uint8_t byte)
{
    AddressParts parts(address);   // L1 & L2 address segmentation.
    uint32_t VCTag = address >> 2; // VC address has different segmentation.

    // Case A: L1 Hit.

    CacheBlock &block = m_L1[parts.index];
    if (block.valid && block.tag == parts.tag)
    {
        block.data[parts.offset] = byte;
        m_MM[address] = byte; // Write-through.
        return;
    }

    // Case B: L1 Miss -> VC Hit.

    for (size_t way = 0; way < VICTIM_SIZE; way++)
    {
        CacheBlock &block = m_VC[way];
        if (block.valid && block.tag == VCTag)
        {
            block.data[parts.offset] = byte;
            updateVictimMRU(way);
            m_MM[address] = byte; // Write-through.
            return;
        }
    }

    // Case C: L1 Miss -> VC Miss -> L2 Hit.

    for (size_t way = 0; way < L2_CACHE_WAYS; way++)
    {
        CacheBlock &block = m_L2[parts.index][way];
        if (block.valid && block.tag == parts.tag)
        {
            block.data[parts.offset] = byte;
            updateL2MRU(parts.index, way);
            m_MM[address] = byte; // Write-through.
            return;
        }
    }

    // Case D: L1 Miss, VC Miss, L2 Miss.

    m_MM[address] = byte; // Write-no-allocate.
}

double Controller::getL1MissRate() const
{
    return static_cast<double>(m_stats.missL1) / m_stats.accessL1;
}

double Controller::getL2MissRate() const
{
    return static_cast<double>(m_stats.missL2) / m_stats.accessL2;
}

double Controller::getAAT() const
{
    return 0; // TODO.
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

void Controller::updateVictimMRU(uint8_t mruWay)
{
    m_VC[mruWay].lruPosition = VICTIM_SIZE - 1;

    for (size_t otherWay = 0; otherWay < VICTIM_SIZE; otherWay++)
    {
        if (mruWay == otherWay || m_VC[otherWay].lruPosition == 0)
            continue;
        m_VC[otherWay].lruPosition--;
    }
}

void Controller::updateL2MRU(uint8_t setIndex, uint8_t mruWay)
{
    CacheBlock *set = m_L2[setIndex];
    set[mruWay].lruPosition = L2_CACHE_WAYS - 1;

    for (size_t otherWay = 0; otherWay < L2_CACHE_WAYS; otherWay++)
    {
        if (otherWay == mruWay || set[otherWay].lruPosition == 0)
            continue;
        set[otherWay].lruPosition--;
    }
}
