#pragma once
#include "chunks.hpp"
#include <vector>
#include <print>

char *calc_light(minecraft::paletted_container data)
{
    char *dat = data.data.get();
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
    std::println("zero id is {}", zero_id);
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
	
	return encoded_val;
}
