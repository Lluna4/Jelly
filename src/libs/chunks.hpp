#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include "utils.hpp"

enum palette_type
{
	SINGLE_VALUED,
	INDIRECT,
	DIRECT
};



namespace minecraft
{
	struct paletted_container
	{
		paletted_container()
		{
			type = SINGLE_VALUED;
			value = 0;
			data_size = varint(0);
			data = std::make_unique<char []>(1);
			data[0] = 0;
		}
		paletted_container(unsigned char bpe,bool chunk, varint val = 0, std::vector<varint> pal = {0})
		:bits_per_entry(bpe)
		{
			if (bpe == 0)
			{
				type = SINGLE_VALUED;
				value = val;
				data_size = varint(0);
				data = std::make_unique<char []>(1);
				data[0] = 0;
			}
			else if (bpe == 8)
			{
				if (chunk == true)
				{
					type = INDIRECT;
					palette = pal;
					data = std::make_shared<char []>(4097);
					if (data == NULL)
						std::runtime_error("Malloc failed");
					if (palette.size() > 1)
						memset(data.get(), 1, 4096);
					else 
						memset(data.get(), 0, 4096);
					data_size = varint(512);
				}
			}
		}
		int type;
		unsigned char bits_per_entry;
		varint value;
		std::vector<varint> palette;
		varint data_size;
		std::shared_ptr<char[]> data;
	};
	
	struct chunk_section
	{
		chunk_section(bool full, std::vector<varint> palette = {varint(0), varint(1)})
		{
			if (full)
			{
				block_count = 4096;
				blocks = paletted_container(8, true, 0, palette);
				biome = paletted_container(0, false);
			}
			else
			{
				block_count = 0;
				blocks = paletted_container(0, true);
				biome = paletted_container(0, false);
			}
		}
		short block_count;
		paletted_container blocks;
		paletted_container biome;
	};
	struct chunk
	{
		chunk()
		{
			for (int i = 0; i < 24; i++)
			{
					sections.emplace_back(false);
			}
			x = 0;
			z = 0;
		}
		chunk(int x_, int z_, char surface = 8,std::vector<varint> palette = {varint(0), varint(9)})
		:x(x_), z(z_)
		{
			for (int i = 0; i < 24; i++)
			{
				if (i < surface)
					sections.emplace_back(true, palette);
				else
					sections.emplace_back(false, palette);
			}
		}
		int x;
		int z;
		std::vector<chunk_section> sections;
	};
};

namespace std {
  template <>
  struct hash<std::pair<int, int>> 
  {
    size_t operator()(const std::pair<int, int>& product) const {
      return std::hash<int>()(product.first) ^ std::hash<int>()(product.second);
    }
  };
}

std::unordered_map<std::pair<int, int>, minecraft::chunk> chunks;

minecraft::chunk find_chunk(int x, int z)
{
	if (chunks.find({x, z}) == chunks.end()) //if it doesnt find a chunk it generates one
	{
		chunks.insert({{x, z}, minecraft::chunk(x, z)});
	}
	return chunks[{x, z}];
}
