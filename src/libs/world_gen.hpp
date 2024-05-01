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
    minecraft::paletted_container ret = {.bits_per_entry = 0, 
    .block_ids = {(minecraft::varint){.num = 9}}, .data_lenght = (minecraft::varint){.num = 0}, .block_indexes = {}};
    return ret;
}

minecraft::paletted_container world_gen_empty()
{
    minecraft::paletted_container ret = {.bits_per_entry = 0, 
    .block_ids = {(minecraft::varint){.num = 0}}, .data_lenght = (minecraft::varint){.num = 0}, .block_indexes = {}};
    return ret;
}

std::bitset<64> concat(std::vector<std::bitset<4>> &vec, int index)
{
    std::string buf;
    for (int i = index; i < (index + 16); i++)
    {
        buf = buf + vec[index].to_string();
    }
    return std::bitset<64>(buf);
}

std::bitset<64> concat(std::vector<std::bitset<4>> &vec, int index, int change_index, std::bitset<4> change)
{
    std::string buf;
    for (int i = index; i < (index + 16); i++)
    {
        if (i == change_index)
            buf = buf + change.to_string();
        else 
            buf = buf + vec[index].to_string();
    }
    return std::bitset<64>(buf);
}

minecraft::paletted_container_indirect world_gen_inderect(bool surface = false)
{
    minecraft::paletted_container_indirect ret = {.bits_per_entry = 4, .palette_data_entries = (minecraft::varint){.num = 3}, 
    .block_ids = {(minecraft::varint){.num = 0}, (minecraft::varint){.num = 9}, (minecraft::varint){.num = 10}}, 
    .data_lenght = (minecraft::varint){.num = 256}};

    int index = 0;
    std::vector<long> longs;
    std::vector<std::bitset<4>> numbers;
    int x = 0, y = 0, z = 0;

    for (int y = 0; y < 16; y++)
    {
        for (int z = 0; z < 16; z++)
        {
           for (int x = 0; x < 16; x++) 
           {
                std::bitset<4> new_num;
                if (y < 15)
                    new_num = 0x2;
                else if (surface == true)
                    new_num = 0x1;
                else if (surface == false)
                    new_num = 0x2;
                numbers.push_back(new_num);
           }
        }
    }
    for (int i = 0; i < numbers.size(); i += 16)
    {
        longs.push_back((long)concat(numbers, i).to_ulong());
    }
    ret.block_indexes = longs;
    ret.block_indexes_nums = numbers;
    return ret;
}

minecraft::paletted_container_indirect world_gen_inderect_empty()
{
    minecraft::paletted_container_indirect ret = {.bits_per_entry = 4, .palette_data_entries = (minecraft::varint){.num = 3}, 
    .block_ids = {(minecraft::varint){.num = 0}, (minecraft::varint){.num = 9}, (minecraft::varint){.num = 10}}, 
    .data_lenght = (minecraft::varint){.num = 256}};

    int index = 0;
    std::vector<long> longs;
    std::vector<std::bitset<4>> numbers;
    int x = 0, y = 0, z = 0;

    for (int y = 0; y < 16; y++)
    {
        for (int z = 0; z < 16; z++)
        {
           for (int x = 0; x < 16; x++) 
           {
                std::bitset<4> new_num;
                new_num = 0x0;
                numbers.push_back(new_num);
           }
        }
    }
    for (int i = 0; i < numbers.size(); i += 16)
    {
        longs.push_back((long)concat(numbers, i).to_ulong());
    }
    ret.block_indexes = longs;
    ret.block_indexes_nums = numbers;
    return ret;
}

/*minecraft::paletted_container world_gen_empty()
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
            new_long |= (0x0ULL << i);
        }
        longs.push_back(new_long.to_ulong());
        index++;
    }
    ret.block_indexes = longs;
    return ret;
}*/

minecraft::paletted_container biome_gen()
{
    minecraft::paletted_container ret = {.bits_per_entry = 0, 
    .block_ids = {(minecraft::varint){.num = 0}}, .data_lenght = (minecraft::varint){.num = 0}, .block_indexes = {}};
    return ret;
}
