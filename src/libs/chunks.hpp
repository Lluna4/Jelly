#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include "utils.hpp"
#include "PerlinNoise.hpp"

enum palette_type
{
	SINGLE_VALUED,
	INDIRECT,
	DIRECT
};
bool LIGHT_POSTPROCESSING = true;
unsigned int s = 0;

struct dot
{
	dot(int start_h, int end_h, double start_n, double end_n)
	:start_height(start_h), end_height(end_h), start_noise(start_n), end_noise(end_n)
	{
		n = abs(end_height - start_height)/abs(end_noise - start_noise);
	}
	int start_height;
	int end_height;
	double start_noise;
	double end_noise;
	double n;

	int get_height(double noise)
	{
		return (n * (noise - start_noise) + start_height);
	}
};

struct position_int
{
	int x, y, z;
};


std::map<double, dot> t_map = {{-1.0, dot(30, 64, -1.0, -0.1)}, {-0.1, dot(64, 75, -0.1, 0.4)}, {0.4, dot(75, 90, 0.4, 0.6)},
								{0.6, dot(90, 100, 0.6, 0.8)}, {0.8, dot(100, 130, 0.8, 0.9)}, {0.9, dot(130, 140, 0.9, 1.0)}};
std::map<double, dot> t_map2 = {{-1.0, dot(-5, -2, -1.0, -0.5)}, {-0.5, dot(-2, 1, -0.5, 0.0)}, {0.0, dot(1, 4, 0.0, 1.0)}};
std::map<double, dot> t_map3 = {{-1.0, dot(-30, -20, -1.0, -0.8)}, {-0.8, dot(-20, -5, -0.8, 0.0)}, {0.0, dot(-5, 30, 0.0, 0.5)},
								{0.5, dot(15, 20, 0.5, 1.0)}};

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

		void place_block(int x, int y, int z, minecraft::varint block_id)
		{
			int section_index = (y + 64)/16;
			int index = 0;
			if (sections[section_index].blocks.type == SINGLE_VALUED)
			{
				sections[section_index].blocks = paletted_container(8, true, 0, {minecraft::varint(0), minecraft::varint(block_id)});
				index = 1;
			}
			else if (sections[section_index].blocks.type == INDIRECT)
			{
				bool found = false;
				for (int i = 0; i < sections[section_index].blocks.palette.size(); i++)
				{
					if (sections[section_index].blocks.palette[i] == block_id)
					{
						index = i;
						found = true;
						break;
					}
				}
				if (found == false)
				{
					index = sections[section_index].blocks.palette.size();
					sections[section_index].blocks.palette.push_back(block_id);
				}
			}
			sections[section_index].blocks.data[(rem_euclid(y, 16)*256) + (z*16) + x] = index;
			sections[section_index].block_count += 1;
		}
		int x;
		int z;
		std::vector<chunk_section> sections;
		std::vector<position_int> trees;
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

minecraft::chunk &find_chunk(int x, int z)
{
	if (chunks.find({x, z}) == chunks.end()) //if it doesnt find a chunk it generates one
	{
		const siv::PerlinNoise::seed_type seed = s;

		const siv::PerlinNoise perlin{ seed };
 	 	chunks.emplace(std::piecewise_construct,
                    std::forward_as_tuple(x, z),
                    std::forward_as_tuple(x, z));
		auto &chunk = chunks[{x, z}];
		for (int z_ = 0; z_ < 16; z_++)
		{
			for (int x_ = 0; x_ < 16; x_++)
			{
				const double noise = perlin.octave2D_11(((x_ + (x * 16)) * 0.002), ((z_ + (z * 16)) * 0.002), 6);
				
				int height_ = 0;
				for (auto p: t_map)
				{
					if (noise >= p.first)
					{
						if (noise <= p.second.end_noise)
						{
							height_ = p.second.get_height(noise);
							break;
						}
					}
				}
				//log(height_, INFO);
				for (int y = 0; y < height_; y++)
				{
					if (y == height_ - 1)
					{
						chunk.place_block(x_, y, z_, 9);
						int randomnumber = random()%10;
						int random2 = random()%1000;
						if (random2 > 995)
						{
							chunk.trees.emplace_back(x_ +(x * 16), height_, z_ + (z * 16));
						}
						if (randomnumber < 3)
						{
							chunk.place_block(x_, y + 1, z_, 2005);
						}
						else if (randomnumber > 9)
						{
							chunk.place_block(x_, y + 1, z_, 10756);
						}
					}
					else
						chunk.place_block(x_, y, z_, 10);
				}
				if (height_ < 64)
				{
					chunk.place_block(x_, height_ - 1, z_, 10);
					for (int y = height_; y < 64; y++)
					{
						chunk.place_block(x_, y, z_, 80);
					}
				}
				if (height_ >= 120)
				{
					chunk.place_block(x_, height_ - 1, z_, 8);
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
