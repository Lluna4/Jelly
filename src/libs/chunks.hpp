#pragma once
#include <iostream>
#include <unordered_map>
#include "logging.hpp"
#include <vector>
#include <bitset>
#include "utils.hpp"
#include <functional>

namespace minecraft
{
	struct chunk_pos
	{
		int x;
		int z;

		bool operator == (const chunk_pos& pos) const
		{
			return (x == pos.x) && (z == pos.z);
		}
	};
	struct paletted_container_rw
	{
		unsigned char bits_per_entry;
		varint palette_data_entries;
		std::vector<varint> block_ids;
		varint data_lenght;
		std::vector<unsigned long> block_indexes;
		std::vector<std::bitset<4>> block_indexes_nums;
	};
	struct chunk_section_rw
	{
		short block_count;
		paletted_container_rw blocks;
		paletted_container_rw biome;
	};
	struct chunk_rw
	{
		std::vector<chunk_section_rw> chunks;

		int size()
		{
			int ret = 0;
			std::string buf;
			std::size_t size = 0;
			for (int x = 0; x < chunks.size(); x++)
			{
				ret += sizeof(short);
				minecraft::paletted_container_rw cont = chunks[x].blocks;

				ret += sizeof(unsigned char);

				ret += WriteUleb128(buf, cont.palette_data_entries.num);
				for (int i = 0; i < cont.block_ids.size(); i++)
					ret += WriteUleb128(buf, cont.block_ids[i].num);

				ret += WriteUleb128(buf, cont.data_lenght.num);

				ret += (sizeof(long) * cont.block_indexes.size());
				
				minecraft::paletted_container_rw cont2 = chunks[x].biome;

				ret += sizeof(unsigned char);

				ret += WriteUleb128(buf, cont2.block_ids[0].num);

				ret += WriteUleb128(buf, cont2.data_lenght.num);

				for (int i = 0; i < cont2.block_indexes.size(); i++)
				{
					ret += sizeof(long);
				}
			}
			return ret;
		}
	};

}

namespace std {
  template <>
  struct hash<minecraft::chunk_pos> 
  {
    size_t operator()(const minecraft::chunk_pos& product) const {
      // Combine hash values of id and name using bitwise XOR
      return std::hash<int>()(product.x) ^ std::hash<int>()(product.z);
    }
  };
}

std::unordered_map<minecraft::chunk_pos, minecraft::chunk_rw> chunks_r;


