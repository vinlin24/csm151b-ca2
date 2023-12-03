// #include "cache.h"

// #include <iostream>
// #include <tuple>

// using namespace std;

// // Split a memory address into tag, index, offset. This is assuming a 16-set
// // cache (4 bit index) and 4-byte block size (2 bit offset).
// static tuple<uint32_t, uint8_t, uint8_t> splitAddress(uint32_t address)
// {
//     uint8_t offset = address & 0b11;
//     uint8_t index = (address >> 2) & 0b1111;
//     uint32_t tag = address >> 6;
//     return tuple(tag, index, offset);
// }

// L1Cache::L1Cache()
// {
//     for (size_t index = 0; index < L1_CACHE_SETS; index++)
//         m_blocks[index].valid = false;
// }

// optional<uint8_t> L1Cache::readByte(AddressParts const &parts) const
// {
//     CacheBlock const &block = m_blocks[parts.index];
//     if (!block.valid || block.tag != parts.tag)
//         return nullopt;
//     return block.data[parts.offset];
// }

// bool L1Cache::writeByte(AddressParts const &parts, uint8_t byte)
// {
//     CacheBlock &block = m_blocks[parts.index];
//     if (!block.valid || block.tag != parts.tag)
//         return false;
//     block.data[parts.offset] = byte;
//     return true;
// }

// L2Cache::L2Cache()
// {
//     for (size_t index = 0; index < L2_CACHE_SETS; index++)
//     {
//         for (int way = 0; way < L2_CACHE_WAYS; way++)
//         {
//             CacheBlock &block = m_blocks[index][way];
//             block.valid = false;
//             // TODO: IDK if this is the best way to initialize this.
//             block.lruPosition = way;
//         }
//     }
// }

// optional<uint8_t> L2Cache::readByte(uint32_t address)
// {
//     auto [tag, index, offset] = splitAddress(address);
//     for (size_t way = 0; way < L2_CACHE_WAYS; way++)
//     {
//         CacheBlock &block = m_blocks[index][way];
//         if (block.valid && block.tag == tag)
//         {
//             updateMRU(index, way);
//             return block.data[offset];
//         }
//     }
//     return nullopt;
// }

// bool L2Cache::writeByte(uint32_t address, uint8_t byte)
// {
//     auto [tag, index, offset] = splitAddress(address);
//     for (size_t way = 0; way < L2_CACHE_WAYS; way++)
//     {
//         CacheBlock &block = m_blocks[index][way];
//         if (block.valid && block.tag == tag)
//         {
//             block.data[offset] = byte;
//             updateMRU(index, way);
//             return true;
//         }
//     }
//     return false;
// }

// void L2Cache::updateMRU(size_t index, size_t way)
// {
//     CacheBlock *set = m_blocks[index];
//     set[way].lruPosition = L2_CACHE_WAYS - 1; // MRU.

//     for (size_t otherWay = 0; otherWay < L2_CACHE_WAYS; otherWay++)
//     {
//         if (otherWay == way || set[otherWay].lruPosition == 0)
//             continue;
//         set[otherWay].lruPosition--;
//     }
// }

// VictimCache::VictimCache()
// {
//     for (size_t way = 0; way < VICTIM_SIZE; way++)
//     {
//         CacheBlock &block = m_blocks[way];
//         block.valid = false;
//         // TODO: IDK if this is the best way to initialize this.
//         block.lruPosition = way;
//     }
// }

// optional<uint8_t> VictimCache::readByte(uint32_t address)
// {
//     uint8_t offset = address & 0b11;
//     uint32_t tag = address >> 2;
//     for (size_t way = 0; way < VICTIM_SIZE; way++)
//     {
//         CacheBlock &block = m_blocks[way];
//         if (block.valid && block.tag == tag)
//         {
//             updateMRU(way);
//             return block.data[offset];
//         }
//     }
//     return nullopt;
// }

// bool VictimCache::writeByte(uint32_t address, uint8_t byte)
// {
//     uint8_t offset = address & 0b11;
//     uint32_t tag = address >> 2;
//     for (size_t way = 0; way < VICTIM_SIZE; way++)
//     {
//         CacheBlock &block = m_blocks[way];
//         if (block.valid && block.tag == tag)
//         {
//             block.data[offset] = byte;
//             updateMRU(way);
//             return true;
//         }
//     }
//     return false;
// }

// void VictimCache::updateMRU(size_t way)
// {
//     m_blocks[way].lruPosition = VICTIM_SIZE - 1; // MRU.
//     for (size_t otherWay = 0; otherWay < VICTIM_SIZE; otherWay++)
//     {
//         if (way == otherWay || m_blocks[otherWay].lruPosition == 0)
//             continue;
//         m_blocks[otherWay].lruPosition--;
//     }
// }
