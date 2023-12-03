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
        uint32_t word = loadWord(trace.address);
        cerr << "Loaded " << word << "." << endl;
    }
    else
    {
        storeWord(trace.address, trace.data);
        cerr << "Stored " << trace.data << " to " << trace.address << "."
             << endl;
    }
}

uint32_t Controller::loadWord(uint32_t address)
{
    // TODO.
}

void Controller::storeWord(uint32_t address, uint32_t data)
{
    auto [tag, index, offset] = splitAddress(address);

    // Case A: L1 hit.

    CacheBlock &L1Block = m_L1[index];
    if (L1Block.valid && L1Block.tag == tag)
    {
        L1Block.data = data;
        writeWordToMM(address, data); // Write-through.
        return;
    }

    // Case B: L1 Miss, VC hit.

    uint32_t VCTag = address >> 2;
    for (size_t way = 0; way < VICTIM_SIZE; way++)
    {
        CacheBlock &VCBlock = m_VC[way];
        if (!VCBlock.valid || VCBlock.tag != VCTag)
            continue;

        VCBlock.data = data;

        // Update LRU positions. TODO: Make this into a helper.
        VCBlock.lruPosition = VICTIM_SIZE - 1;
        for (size_t i = 0; i < VICTIM_SIZE; i++)
        {
            if (i == way || m_VC[i].lruPosition == 0)
                continue;
            m_VC[i].lruPosition--;
        }

        writeWordToMM(address, data); // Write-through.
        return;
    }

    // Case C: L1 Miss, VC Miss L2 Hit.

    for (size_t way = 0; way < L2_CACHE_WAYS; way++)
    {
        CacheBlock &L2Block = m_L2[index][way];
        if (!L2Block.valid || L2Block.tag != tag)
            continue;

        L2Block.data = data;

        // Update LRU positions. TODO: Make this into a helper.
        L2Block.lruPosition = L2_CACHE_WAYS - 1;
        for (size_t i = 0; i < L2_CACHE_WAYS; i++)
        {
            if (i == way || m_L2[index][i].lruPosition == 0)
                continue;
            m_L2[index][i].lruPosition--;
        }

        writeWordToMM(address, data); // Write-through.
        return;
    }

    // Case D: L1 Miss, VC Miss, L2 Miss.
    writeWordToMM(address, data); // Write-no-allocate.
}

uint32_t Controller::readWordFromMM(uint32_t address)
{
    uint8_t b0 = m_MM[address];
    uint8_t b1 = m_MM[address + 1];
    uint8_t b2 = m_MM[address + 2];
    uint8_t b3 = m_MM[address + 3];

    uint32_t word = 0;
    word |= b0;
    word |= (b1 << 8);
    word |= (b2 << 16);
    word |= (b3 << 24);
    return word;
}

void Controller::writeWordToMM(uint32_t address, uint32_t data)
{
    uint8_t b0 = data & 0xFF;
    uint8_t b1 = (data >> 8) & 0xFF;
    uint8_t b2 = (data >> 16) & 0xFF;
    uint8_t b3 = (data >> 24) & 0xFF;

    m_MM[address] = b0;
    m_MM[address + 1] = b1;
    m_MM[address + 2] = b2;
    m_MM[address + 3] = b3;
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
    uint8_t index = (address >> 2) && 0b1111;
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
