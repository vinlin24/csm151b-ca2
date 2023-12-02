#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#define L1_CACHE_SETS 16
#define L2_CACHE_SETS 16
#define VICTIM_SIZE 4
#define L2_CACHE_WAYS 8
#define MEM_SIZE 4096
#define BLOCK_SIZE 4 // Bytes per block.
#define DM 0
#define SA 1

struct CacheBlock
{
    int tag;          // You need to compute offset and index to find the tag.
    int lru_position; // For SA only.
    int data;         // The actual data stored in the cache/memory.
    bool valid;

    // TODO: Add more things here if needed.
};

struct Stat
{
    int missL1;
    int missL2;
    int accL1;
    int accL2;
    int accVic;
    int missVic;

    // TODO: Add more stat if needed. Don't forget to initialize!
};

class Cache
{
public:
    Cache();
    void controller(bool MemR, bool MemW, int *data, int adr, int *myMem);

    // TODO: Add more functions here ...

private:
    CacheBlock L1[L1_CACHE_SETS];                // 1 set per row.
    CacheBlock L2[L2_CACHE_SETS][L2_CACHE_WAYS]; // x ways per row.

    // TODO: Add your Victim cache here...

    Stat myStat;

    // TODO: Add more things here.
};
