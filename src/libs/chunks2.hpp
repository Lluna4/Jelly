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

struct structure
{
	structure()
	{
		y_max = 15;
		x_max = 11;
		z_max = 11;
		blocks.resize(15);
		for (auto &block: blocks)
		{
			block.resize(11);
			for (auto &b: block)
			{
				b.resize(11);
			}
		}
		int x_ = 5;
		int z_ = 5;
		int height_ = 0;
		blocks[height_][z_][x_] = 146;
		blocks[height_ + 1][z_][x_] = 146;
		blocks[height_ + 2][z_][x_] = 146;
		blocks[height_ + 3][z_][x_] = 146;
		blocks[height_ + 3][ z_][x_ + 1] = 146;
		blocks[height_ + 3][ z_][x_ + 2] = 146;
		blocks[height_ + 4][ z_][x_ + 2] = 146;
		blocks[height_ + 5][ z_][x_ + 2] = 146;
		int diff = 4;
		int diff2 = 0;
		for (int i = height_ + 5; i < height_ + 10;i++)
		{
			if (i == height_ + 7)
			{
				diff++;
				diff2--;
			}
			if (i == height_ + 8)
			{
				diff--;
				diff2++;
			}
			if (i == height_ + 9)
			{
				diff--;
				diff2++;
			}
			for (int ii = (z_ - 2) + diff2; ii < z_ + diff; ii++)
			{
				for (int iii = x_ + diff2; iii < x_ + (diff + 2); iii++)
				{
					int r = 50;
					if (r < 80)
						blocks[i][ii][iii] = 404;
				}
			}
		}
	}
	structure(int x_m, int y_m, int z_m)
	:x_max(x_m), y_max(y_m), z_max(z_m)
	{
		blocks.resize(y_m);
		for (auto &block: blocks)
		{
			block.resize(z_m);
			for (auto &b: block)
			{
				b.resize(x_m);
			}
		}
	}
	structure(std::vector<std::vector<std::vector<int>>> structure,int x_m, int y_m, int z_m)
	:blocks(structure), x_max(x_m), y_max(y_m), z_max(z_m)
	{
		blocks.resize(y_m);
		for (auto &block: blocks)
		{
			block.resize(z_m);
			for (auto &b: block)
			{
				b.resize(x_m);
			}
		}
	}
	std::vector<std::vector<std::vector<int>>> blocks;
	int x_max, y_max, z_max;
};

struct position_int
{
	int x, y, z;
};

struct block_pos
{
	block_pos(int x_, int y_, int z_, minecraft::varint b_id)
	:x(x_), y(y_), z(z_), block_id(b_id)
	{}
	int x, y, z;
	minecraft::varint block_id;
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
std::unordered_map<std::pair<int, int>, std::vector<block_pos>> blocks_to_place;
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
		heights.resize(16*16);
	}
	std::vector<section> sections;
	int x, z;
	bool generated;
	std::vector<position_int> trees;
	std::vector<int> heights;
	void place_block(int x, int y, int z, minecraft::varint block_id)
	{
		int section = (y + 64)/16;
		sections[section].place_block(x, rem_euclid(y, 16), z, block_id);
	}
	std::pair<position_int, position_int> translate_to_other_chunk(position_int block_pos)
	{
		position_int ret = block_pos;
		position_int chunk_ret = {x, 0, z}; 

		// Handle X coordinate
		if (block_pos.x >= 16 || block_pos.x < 0) {
			int chunk_offset = (block_pos.x >= 0) ? block_pos.x / 16 : (block_pos.x - 15) / 16;
			ret.x = rem_euclid(block_pos.x, 16);
			chunk_ret.x += chunk_offset;
		}

		// Handle Z coordinate 
		if (block_pos.z >= 16 || block_pos.z < 0) {
			int chunk_offset = (block_pos.z >= 0) ? block_pos.z / 16 : (block_pos.z - 15) / 16;
			ret.z = rem_euclid(block_pos.z, 16);
			chunk_ret.z += chunk_offset;
		}

		return std::make_pair(ret, chunk_ret);
	}
};

struct world
{
	std::unordered_map<std::pair<int, int>, chunk> chunks;
	structure Structure;

	void place_translated_block(position_int block_p, chunk &chunk, minecraft::varint block_id)
	{
		auto positions = chunk.translate_to_other_chunk(block_p);
		auto bl = blocks_to_place.find(std::make_pair(positions.second.x, positions.second.z));
		if (bl != blocks_to_place.end())
		{
			bl->second.push_back({positions.first.x, positions.first.y, positions.first.z, block_id});
		}
		else
		{
			std::vector<block_pos> a = {{positions.first.x, positions.first.y, positions.first.z, block_id}};
			blocks_to_place.insert({std::make_pair(positions.second.x, positions.second.z), a});
		}
	}

	chunk &generate_chunk(int x, int z, bool trees = true)
	{
		auto chunk1 = chunks.find(std::make_pair(x, z));
		if (chunk1 != chunks.end())
		{
			return chunk1->second;
		}
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
				int worldX = x * 16 + x_;
				int worldZ = z * 16 + z_;
				const double noise = perlin.octave2D_11((worldX * 0.002), (worldZ * 0.002), 6);
				
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
				chunk.heights[z_ * 16 + x_] = height_;
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
						if (random2 > 995 && y >= 64 && trees == true)
						{
							for (int yy = 0; yy < Structure.y_max; yy++)
							{
								for (int zz = 0; zz < Structure.z_max; zz++)
								{
									for (int xx = 0; xx < Structure.x_max; xx++)
									{
										if (Structure.blocks[yy][zz][xx] == 0)
											continue;
										place_translated_block({x_ + (xx - Structure.x_max/2), height_ + yy, z_ + (zz - Structure.z_max/2)}, chunk, minecraft::varint(Structure.blocks[yy][zz][xx]));
									}
								}
							}
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
		auto &chunk = generate_chunk(floor((float)x/16.0f), floor((float)z/16.0f));
		chunk.place_block(rem_euclid(x, 16), y, rem_euclid(z, 16), block_id);
	}
	
	int get_height(int x, int z)
	{
	    auto &chunk = generate_chunk(floor((float)x/16.0f), floor((float)z/16.0f));
		return chunk.heights[rem_euclid(z, 16) * 16 + rem_euclid(z, 16)];
	}

	void place_tree(position_int pos)
	{
		int x = pos.x;
        int z = pos.z;
        int y = pos.y;
        place_block(x, y, z, 146);
        place_block(x, y + 1, z, 146);
        place_block(x, y + 2, z, 146);
        place_block(x, y + 3, z, 146);
        place_block(x + 1, y + 3, z, 146);
        place_block(x + 2, y + 3, z, 146);
        place_block(x + 2, y + 4, z, 146);
        place_block(x + 2, y + 5, z, 146);
        int diff = 4;
        int diff2 = 0;
        for (int i = y + 5; i < y + 10;i++)
        {
            if (i == y + 7)
            {
                diff++;
                diff2--;
            }
            if (i == y + 8)
            {
                diff--;
                diff2++;
            }
            if (i == y + 9)
            {
                diff--;
                diff2++;
            }
            for (int ii = (z - 2) + diff2; ii < z + diff; ii++)
            {
                for (int iii = x + diff2; iii < x + (diff + 2); iii++)
                {
                    int r = 50;
                    if (r < 80)
                        place_block(iii, i, ii, 404);
                }
            }
        }
	}
};
