#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include "utils.hpp"
#include "PerlinNoise.hpp"

enum palette_type
{
	SINGLE_VALUED,
	INDIRECT,
	DIRECT
};
bool LIGHT_POSTPROCESSING = true;
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
	char_size calc_light(minecraft::paletted_container data)
	{
		char *dat = data.data.get();
		char a[4096] = {0};
		if (data.type == SINGLE_VALUED)
		{
			if(data.value.num == 0)
			{	
				dat = a;
			}
		}
		
		std::vector<char> light_data;
		int zero_id = 0;
		for (int i = 0; i < data.palette.size();i++)
		{
			if (data.palette[i].num == 0)
			{
				zero_id = i;
				break;
			}
		}
		//std::println("zero id is {}", zero_id);
		light_data.resize(4096);
		for (int z = 0; z < 16;z++)
		{
			for (int x = 0; x < 16; x++)
			{
				char light_ray_intensity = 15;
				for (int y = 15; y >= 0; y--)
				{
					light_data[(y*256) + (z*16) + x] = light_ray_intensity;
					if (dat[(y*256) + (z*16) + x] != zero_id)
					{
						light_ray_intensity = 0;
						//std::println("Got a block at x {} y {} z {}, id {}", x, y, z, (int)dat[(y*256) + (z*16) + x]);
					}
				}
			}
		}
		for (int z = 0; z < 16;z++)
		{
			for (int x = 0; x < 16; x++)
			{
				for (int y = 15; y >= 0; y--)
				{
					if (light_data[(y*256) + (z*16) + x] == 0)
					{
						int end_data = 0;
						for (int i = 1; i < 15;i++)
						{
							if (x - 1 < 0 || y - 1 < 0 || z - 1 < 0)
								break;
							if (x + 1 > 15 || y + 1 > 15 || z + 1 > 15)
								break;
							if (light_data[(y*256) + (z*16) + (x + i)] == 15 || light_data[(y*256) + (z*16) + (x - i)] == 15)
							{
								end_data = 15 - i;
								break;
							}
							if (light_data[(y*256) + ((z + i)*16) + x] == 15 || light_data[(y*256) + ((z - 1)*16) + x] == 15)
							{
								end_data = 15 - i;
								break;
							}
							if (light_data[(y*256) + ((z + i)*16) + (x + i)] == 15 || light_data[(y*256) + ((z - i)*16) + (x - i)] == 15)
							{
								end_data = 15 - i;
								break;
							}
							if (light_data[(y*256) + ((z - i)*16) + (x + i)] == 15 || light_data[(y*256) + ((z + i)*16) + (x - i)] == 15)
							{
								end_data = 15 - i;
								break;
							}
						}
						for (int y_ = y; y_ >= 0; y_--)
						{
							light_data[(y_*256) + (z*16) + x] = end_data;
						}
					}
				}
			}
		}
		char *encoded_val = (char *)calloc(2048, sizeof(char));
		int encoded_index = 0;
		for (int x = 0; x < light_data.size();x += 2)
		{
			encoded_val[encoded_index] |= light_data[x];
			encoded_val[encoded_index] = (encoded_val[encoded_index] << 4) | light_data[x + 1];
			encoded_index++;
		}
		char_size ret = {.data = mem_dup(encoded_val, 2048), .consumed_size = 2048, .max_size = 2048, .start_data = nullptr};
		ret.start_data = ret.data;
		free(encoded_val);
		return ret;
	}

	struct chunk_section
	{
		chunk_section(bool full,bool lighting = false,std::vector<varint> palette = {varint(0), varint(1)})
		{
			if (full)
			{
				block_count = 4096;
				blocks = paletted_container(8, true, 0, palette);
				biome = paletted_container(0, false);
				if (lighting == true)
					light = calc_light(blocks);
			}
			else
			{
				block_count = 0;
				blocks = paletted_container(0, true);
				biome = paletted_container(0, false);
				if (lighting == true)
					light = calc_light(blocks);
			}
		}
		short block_count;
		paletted_container blocks;
		paletted_container biome;
		char_size light;
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
				sections.emplace_back(false, true, palette);
			}
		}

		void place_block(int x, int y, int z, int block_id)
		{
			int section = (y + 64)/16;
			int in_chunk_y = rem_euclid(y, 16);
			if (sections[section].blocks.type == SINGLE_VALUED)
			{
				sections[section].blocks = minecraft::paletted_container(8, true, 0, {minecraft::varint(9), minecraft::varint(0), minecraft::varint(block_id)});
				sections[section].blocks.data[(in_chunk_y*256) + (z*16) + x] = 2;
				sections[section].block_count = 1;
			}
			else if (sections[section].blocks.type == INDIRECT)
			{
				int index = -1;
				for (int i = 0; i < sections[section].blocks.palette.size();i++)
				{
					if (sections[section].blocks.palette[i].num == block_id)
					{
						index = i;
						break;
					}
				}
				if (index == -1)
				{
					sections[section].blocks.palette.emplace_back(block_id);
					sections[section].blocks.data[(in_chunk_y*256) + (z*16) + x] = sections[section].blocks.palette.size() - 1;
					sections[section].block_count += 1;
				}
				else
				{
					sections[section].blocks.data[(in_chunk_y*256) + (z*16) + x] = index;
					sections[section].block_count += 1; 
				}
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
		const siv::PerlinNoise::seed_type seed = 12156212u;

		const siv::PerlinNoise perlin{ seed };
 	 	chunks.emplace(std::piecewise_construct,
                    std::forward_as_tuple(x, z),
                    std::forward_as_tuple(x, z));
		auto &chunk = chunks[{x, z}];
		for (int z_ = 0; z_ < 16; z_++)
		{
			for (int x_ = 0; x_ < 16; x_++)
			{
				const double noise = perlin.octave2D_01(((x_ + (x * 16)) * 0.001), ((z_ + (z * 16)) * 0.001), 8);
				int height_ = static_cast<int>(noise*319);
				for (int y = 0; y < height_; y++)
				{
					if (y == height_ - 1)
						chunk.place_block(x_, y, z_, 9);
					else
						chunk.place_block(x_, y, z_, 10);
				}
			}
		}
	}
	return chunks[{x, z}];
}

void free_chunks()
{
	for (auto chunk: chunks)
	{
		auto a = chunk.second.sections;
		for (int i = 0; i < a.size();i++)
		{
			free((a[i].light).data);
		}
	}
}
