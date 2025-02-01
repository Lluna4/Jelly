#include "../libs/chunks2.hpp"
#include <print>

int main()
{
	world World;

	chunk Chunk = World.generate_chunk(0, 0);

	std::println("Generated = {}", Chunk.generated);

	for (auto Section: Chunk.sections)
	{
		std::println("Section {} Block count {}", Section.position, Section.block_count);
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				for (int x = 0; x < 16; x++)
				{
					std::print("{} ", (int)Section.blocks[y][z][x]);
				}
				std::println();
			}
		}
	}
}
