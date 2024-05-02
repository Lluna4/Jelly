#include "../libs/world_gen.hpp"
#include "../libs/utils.hpp"
#include <iostream>
#include <fstream>


int main()
{
	minecraft::chunk_indirect chun;
	int in = 0;
	while (in < 24)
	{
		minecraft::chunk_section_indirect section;

		if (in < 8)
		{
			section = {.block_count = 4096, .blocks = world_gen_inderect(), .biome = biome_gen()};
			if (in == 7)
				section = {.block_count = 4096, .blocks = world_gen_inderect(true), .biome = biome_gen()};
		}
		else
			section = {.block_count = 0, .blocks = world_gen_inderect_empty(), .biome = biome_gen()};
		chun.chunks.push_back(section);
		in++;
	}
	std::ofstream file("chunk.bin", std::ios::binary);
	for (int x = 0; x < chun.chunks.size(); x++)
	{
		chun.chunks[x].block_count = htobe16(*(uint16_t*)&chun.chunks[x].block_count);
		file << chun.chunks[x].block_count;
		minecraft::paletted_container_indirect cont = chun.chunks[x].blocks;
		file << cont.bits_per_entry;
		file << (unsigned char)cont.palette_data_entries.num;
		for (int i = 0; i < cont.block_ids.size(); i++)
			file << (unsigned char)cont.block_ids[i].num;
		std::string aa;
		WriteUleb128(aa, cont.data_lenght.num);
		file << aa;

		for (int i = 0; i < cont.block_indexes.size(); i++)
		{
			file << cont.block_indexes[i];
		}
		
		minecraft::paletted_container cont2 = chun.chunks[x].biome;
		
		file << cont2.bits_per_entry;
		//send_varint(fd, cont2.palette_data_entries.num);
		file << (unsigned char)cont2.block_ids[0].num;
		file << (unsigned char)cont2.data_lenght.num;

		/*for (int i = 0; i < cont2.block_indexes.size(); i++)
		{
			send(fd, &cont2.block_indexes[i], sizeof(long), 0);
		}*/
	}

}
