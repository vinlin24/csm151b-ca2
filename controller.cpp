#include "controller.h"

#include <cstring>
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
    AddressParts parts(address);   // L1 & L2 address segmentation.
    uint32_t VCTag = address >> 2; // VC address has different segmentation.

    uint8_t byte;

    // Case A: L1 Hit.

    CacheBlock &block = m_L1[parts.index];
    m_stats.accessL1++;
    if (block.valid && block.tag == parts.tag)
    {
        byte = block.data[parts.offset];
        return byte;
    }
    m_stats.missL1++;

    // Case B: L1 Miss -> VC Hit.

    m_stats.accessVC++;
    for (size_t way = 0; way < VICTIM_SIZE; way++)
    {
        CacheBlock &block = m_VC[way];
        if (block.valid && block.tag == VCTag)
        {
            byte = block.data[parts.offset];

            // Promote VC block to L1.
            MemoryBlock VCBytes = popFromVC(way);
            auto evictResult = insertIntoL1(VCBytes);
            if (evictResult.has_value())
            {
                MemoryBlock evictedBytes = evictResult.value();
                cerr << "Discarded block for addresses " << evictedBytes.address
                     << "-" << evictedBytes.address + BLOCK_SIZE - 1 << "."
                     << endl;
            }

            return byte;
        }
    }
    m_stats.missVC++;

    // Case C: L1 Miss -> VC Miss -> L2 Hit.

    m_stats.accessL2++;
    CacheBlock *set = m_L2[parts.index];
    for (size_t way = 0; way < L2_CACHE_WAYS; way++)
    {
        CacheBlock &block = set[way];
        if (block.valid && block.tag == parts.tag)
        {
            byte = block.data[parts.offset];

            // Promote L2 block to L1.
            MemoryBlock L2Bytes = popFromL2(parts.index, way);
            auto evictResult = insertIntoL1(L2Bytes);
            if (evictResult.has_value())
            {
                MemoryBlock evictedBytes = evictResult.value();
                cerr << "Discarded block for addresses " << evictedBytes.address
                     << "-" << evictedBytes.address + BLOCK_SIZE - 1 << "."
                     << endl;
            }

            return byte;
        }
    }
    m_stats.missL2++;

    // Case D: L1 Miss -> VC Miss -> L2 Miss -> MM Access.

    byte = m_MM[address];

    // Bring up a new cache block from main memory.
    MemoryBlock MMBytes;
    MMBytes.address = address & ~(0b11); // 4-byte aligned.
    memcpy(MMBytes.data, m_MM + MMBytes.address, BLOCK_SIZE);
    auto evictResult = insertIntoL1(MMBytes);
    if (evictResult.has_value())
    {
        MemoryBlock evictedBytes = evictResult.value();
        cerr << "Discarded block for addresses " << evictedBytes.address
             << "-" << evictedBytes.address + BLOCK_SIZE - 1 << "."
             << endl;
    }

    return byte;
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
    // AAT = HitTime + MissRate * MissPenalty.

    double MR_L2 = static_cast<double>(m_stats.missL2) / m_stats.accessL2;
    double AAT_L2 = L2_HIT_TIME + MR_L2 * MM_ACCESS_TIME;

    double MR_VC = static_cast<double>(m_stats.missVC) / m_stats.accessVC;
    double AAT_VC = VC_HIT_TIME + MR_VC * AAT_L2;

    double MR_L1 = static_cast<double>(m_stats.missL1) / m_stats.accessL1;
    double AAT_L1 = L1_HIT_TIME + MR_L1 * AAT_VC;

    return AAT_L1;
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

void Controller::dumpCacheState() const
{
    cerr << "L1 blocks: ";
    for (size_t index = 0; index < L1_CACHE_SETS; index++)
    {
        CacheBlock const &block = m_L1[index];
        if (!block.valid)
            continue;

        uint32_t address = (block.tag << 6) | (index << 2);
        uint32_t wordNum = address >> 2; // To match Campuswire debugging.
        cerr << wordNum << " ";
    }
    cerr << endl;

    cerr << "VC blocks: ";
    for (size_t way = 0; way < VICTIM_SIZE; way++)
    {
        CacheBlock const &block = m_VC[way];
        if (!block.valid)
            continue;

        uint32_t address = block.tag << 2;
        uint32_t wordNum = address >> 2; // To match Campuswire debugging.
        cerr << wordNum << " ";
    }
    cerr << endl;

    cerr << "L2 blocks: ";
    for (size_t index = 0; index < L2_CACHE_SETS; index++)
    {
        for (size_t way = 0; way < L2_CACHE_WAYS; way++)
        {
            CacheBlock const &block = m_L2[index][way];
            if (!block.valid)
                continue;

            uint32_t address = (block.tag << 6) | (index << 2);
            uint32_t wordNum = address >> 2; // To match Campuswire debugging.
            cerr << wordNum << " ";
        }
    }
    cerr << endl;
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

optional<MemoryBlock> Controller::insertIntoL1(MemoryBlock const &bytes)
{
    uint8_t setIndex = (bytes.address >> 2) & 0b1111;
    CacheBlock &block = m_L1[setIndex];

    // Create a copy before overwriting the current block.
    CacheBlock evictedBlock(block);

    // Overwrite current block with incoming data.
    block.tag = bytes.address >> 6;
    memcpy(block.data, bytes.data, BLOCK_SIZE);
    block.valid = true;

    // If the block was a vacancy, it wasn't an eviction.
    if (!evictedBlock.valid)
    {
        return nullopt;
    }

    // Otherwise, try to insert the evicted bytes into the lower level cache.
    MemoryBlock evictedBytes;
    evictedBytes.address = (evictedBlock.tag << 6) | (setIndex << 2);
    memcpy(evictedBytes.data, evictedBlock.data, BLOCK_SIZE);
    return insertIntoVC(evictedBytes);
}

optional<MemoryBlock> Controller::insertIntoVC(MemoryBlock const &bytes)
{
    // Find the LRU block. If there's still a vacancy, just use that.
    CacheBlock *blockToUse = nullptr;
    for (CacheBlock &block : m_VC)
    {
        // Vacancy.
        if (!block.valid)
        {
            blockToUse = &block;
            break;
        }
        // Otherwise continue finding the LRU block.
        if (blockToUse == nullptr || block.lruPosition < blockToUse->lruPosition)
        {
            blockToUse = &block;
        }
    }

    CacheBlock &block = *blockToUse; // No nullptr?

    // Create a copy before overwriting the current block.
    CacheBlock evictedBlock(block);

    // Overwrite current block with incoming data.
    block.tag = bytes.address >> 2;
    memcpy(block.data, bytes.data, BLOCK_SIZE);
    // Set the just-written block to the MRU.
    updateVictimMRU(blockToUse - m_VC);
    block.valid = true;

    // If the block was a vacancy, it wasn't an eviction.
    if (!evictedBlock.valid)
        return nullopt;

    // Otherwise, try to insert the evicted bytes into the lower level cache.
    MemoryBlock evictedBytes;
    evictedBytes.address = evictedBlock.tag << 2;
    memcpy(evictedBytes.data, evictedBlock.data, BLOCK_SIZE);
    return insertIntoL2(evictedBytes);
}

optional<MemoryBlock> Controller::insertIntoL2(MemoryBlock const &bytes)
{
    uint8_t setIndex = (bytes.address >> 2) & 0b1111;

    // Find the LRU block. If there's still a vacancy, just use that.
    CacheBlock *blockToUse = nullptr;
    for (CacheBlock &block : m_L2[setIndex])
    {
        // Vacancy.
        if (!block.valid)
        {
            blockToUse = &block;
            break;
        }
        // Otherwise continue finding the LRU block.
        if (blockToUse == nullptr || block.lruPosition < blockToUse->lruPosition)
        {
            blockToUse = &block;
        }
    }

    CacheBlock &block = *blockToUse; // No nullptr?

    // Create a copy before overwriting the current block.
    CacheBlock evictedBlock(block);

    // Overwrite current block with incoming data.
    block.tag = bytes.address >> 6;
    memcpy(block.data, bytes.data, BLOCK_SIZE);
    // Set the just-written block to the MRU.
    updateL2MRU(setIndex, blockToUse - m_L2[setIndex]);
    block.valid = true;

    // If the block was a vacancy, it wasn't an eviction.
    if (!evictedBlock.valid)
        return nullopt;

    // Otherwise, return the evicted bytes.
    MemoryBlock evictedBytes;
    evictedBytes.address = (evictedBlock.tag << 6) | (setIndex << 2);
    memcpy(evictedBytes.data, evictedBlock.data, BLOCK_SIZE);
    return bytes;
}

MemoryBlock Controller::popFromVC(uint8_t way)
{
    CacheBlock &block = m_VC[way];
    block.valid = false;

    MemoryBlock bytes;
    bytes.address = block.tag << 2;
    memcpy(bytes.data, block.data, BLOCK_SIZE);
    return bytes;
}

MemoryBlock Controller::popFromL2(uint8_t index, uint8_t way)
{
    CacheBlock &block = m_L2[index][way];
    block.valid = false;

    MemoryBlock bytes;
    bytes.address = (block.tag << 6) | (index << 2);
    memcpy(bytes.data, block.data, BLOCK_SIZE);
    return bytes;
}
