#pragma once
#include <unordered_map>
#include <vector>
#include <utility>
#include "math.h"
#include "logging.hpp"
#include "utils.hpp"
#include "PerlinNoise.hpp"

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

namespace std 
{
  template <>
  struct hash<std::pair<int, int>> 
  {
    size_t operator()(const std::pair<int, int>& product) const {
      return std::hash<int>()(product.first) ^ std::hash<int>()(product.second);
    }
  };
}

struct section
{
	section(int pos, bool single_value = false, minecraft::varint value = 0)
	:position(pos),single_val(single_value) ,val(value)
	{
		if (single_value == false)
		{
			blocks.resize(16);
			for (auto &z: blocks)
			{
				z.resize(16);
				for (auto &x: z)
				{
					x.resize(16);
				}
			}
			block_count = 0;
			//generated = false;
			palette.push_back(minecraft::varint(0));
		}
		else
		{
			palette.push_back(minecraft::varint(0));
			if (value != 0)
			{
				block_count = 4096;
				palette.push_back(value);
			}
			else
			{
				block_count = 0;
			}
		}
	}
	minecraft::varint val;
	bool single_val;
	std::vector<std::vector<std::vector<char>>> blocks;
	short block_count;
	//bool generated;
	int position;
	std::vector<minecraft::varint> palette;

	void place_block(int x, int y, int z, minecraft::varint block_id)
	{
		int index = -1;

		for (int i = 0; i < palette.size(); i++)
		{
			if (palette[i].num == block_id.num)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			palette.push_back(block_id);
			if (single_val == true)
			{
				blocks.resize(16);
				for (auto &z: blocks)
				{
					z.resize(16);
					for (auto &x: z)
					{
						x.resize(16);
						if (val != 0)
							std::fill(x.begin(), x.end(), 1);
					}
				}
				single_val = false;	
			}
			blocks[y][z][x] = palette.size() - 1;
		}
		else
		{
			if (single_val == true)
			{
				blocks.resize(16);
				for (auto &z: blocks)
				{
					z.resize(16);
					for (auto &x: z)
					{
						x.resize(16);
						if (val != 0)
							std::fill(x.begin(), x.end(), 1);
					}
				}
				single_val = false;
			}
			blocks[y][z][x] = index;
		}
		if (block_id == 0)
			block_count--;
		if (block_id != 0)
			block_count++;
		if (block_count == 4096)
		{
			auto value = blocks[0][0][0];
			for (auto &z: blocks)
			{
				for (auto &x: z)
				{
					for (auto block: x)
					if (value != block)
						return;
				}
			}
			single_val = true;
			val = palette[value];
			blocks.clear();
		}
	}
};

struct chunk
{
	chunk(int pos_x, int pos_z)
	:x(pos_x), z(pos_z)
	{
		for (int i = 0; i < 24; i++)
		{
			if (i < 4)
				sections.emplace_back(i, true, 10);
			sections.emplace_back(i, true);
		}
		generated = false;
	}
	std::vector<section> sections;
	int x, z;
	bool generated;

	void place_block(int x, int y, int z, minecraft::varint block_id)
	{
		int section = (y + 64)/16;
		sections[section].place_block(x, rem_euclid(y, 16), z, block_id);
	}
};

struct world
{
	std::unordered_map<std::pair<int, int>, chunk> chunks;

	chunk &generate_chunk(int x, int z)
	{
		auto chunk1 = chunks.find(std::make_pair(x, z));
		if (chunk1 != chunks.end())
			return chunk1->second;
		auto chunk_r = chunks.emplace(std::piecewise_construct,
            std::forward_as_tuple(x, z),
            std::forward_as_tuple(x, z));
		chunk &chunk = chunk_r.first->second;
		const siv::PerlinNoise::seed_type seed = s;

		const siv::PerlinNoise perlin{ seed };
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
				for (int y = 16; y < height_; y++)
				{
					if (y == height_ - 1)
					{
						chunk.place_block(x_, y, z_, 9);
						int randomnumber = random()%10;
						int random2 = random()%1000;
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
		chunk.generated = true;
		return chunk;
	}
	void place_block(int x, int y, int z, minecraft::varint block_id)
	{
		auto &chunk = generate_chunk(floor(x/16), floor(z/16));
		chunk.place_block(rem_euclid(x, 16), y, rem_euclid(z, 16), block_id);
	}
};
