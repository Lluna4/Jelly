#pragma once
#include "utils.hpp"
#include "logging.hpp"
#include <vector>
#include <iostream>
#include <bitset>

void printBits(long n) {
  for (int i = sizeof(n) * 8 - 1; i >= 0; i--) {
    // Isolate the bit at position i
    int mask = 1 << i;
    // Check if the bit is set
    bool bitSet = (n & mask) > 0;
    std::cout << bitSet;
  }
  std::cout << std::endl;
}


minecraft::paletted_container world_gen()
{
    minecraft::paletted_container ret = {.bits_per_entry = 4, .palette_data_entries = (minecraft::varint){.num = 2}, 
    .block_ids = {(minecraft::varint){.num = 0}, (minecraft::varint){.num = 9}}, .data_lenght = (minecraft::varint){.num = 256}};

    int index = 0;
    std::vector<long> longs;

    while (index < 256)
    {
        std::bitset<64> new_long;
        for (int i = 0; i < 64; i += 4) 
        {
            new_long |= (0x1ULL << i);
        }
        longs.push_back((long)new_long.to_ulong());
        index++;
    }
    ret.block_indexes = longs;
    return ret;
}

minecraft::paletted_container world_gen_empty()
{
    minecraft::paletted_container ret = {.bits_per_entry = 4, .palette_data_entries = (minecraft::varint){.num = 2}, 
    .block_ids = {(minecraft::varint){.num = 0}, (minecraft::varint){.num = 9}}, .data_lenght = (minecraft::varint){.num = 256}};

    int index = 0;
    std::vector<long> longs;

    while (index < 256)
    {
        std::bitset<64> new_long;
        for (int i = 0; i < 64; i += 4) 
        {
            new_long |= (0x1ULL << i);
        }
        longs.push_back(new_long.to_ulong());
        index++;
    }
    ret.block_indexes = longs;
    return ret;
}

minecraft::paletted_container biome_gen()
{
    minecraft::paletted_container ret = {.bits_per_entry = 4, .palette_data_entries = (minecraft::varint){.num = 0}, 
    .block_ids = {}, .data_lenght = (minecraft::varint){.num = 4}};

    int index = 0;
    std::vector<long> longs;

    while (index < 4)
    {
        std::bitset<64> new_long;
        longs.push_back(new_long.to_ulong());
        index++;
    }
    ret.block_indexes = longs;
    return ret;
}