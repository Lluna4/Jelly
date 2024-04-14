#pragma once
#include <typeinfo>
#include <variant>
#include <vector>
#include <iostream>
#include <any>
#include "user.hpp"
#include "utils.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

int calc_len(std::vector<const std::type_info*> types, std::vector<std::any> values)
{
	int size = 0; 
	for (int i = 0; i < types.size(); i++)
	{
		if (types[i]->hash_code() == typeid(int).hash_code())
		{
			size += sizeof(int);
		}
		else if (types[i]->hash_code() == typeid(unsigned int).hash_code())
		{
			size += sizeof(unsigned int);
		}
		else if (types[i]->hash_code() == typeid(char).hash_code())
		{
			size += sizeof(char);
		}
		else if (types[i]->hash_code() == typeid(float).hash_code())
		{
			size += sizeof(unsigned int);
		}
		else if (types[i]->hash_code() == typeid(double).hash_code())
		{
			size += sizeof(unsigned long);
		}
		else if (types[i]->hash_code() == typeid(long).hash_code())
		{
			size += sizeof(long);
		}
		else if (types[i]->hash_code() == typeid(unsigned long).hash_code())
		{
			size += sizeof(unsigned long);
		}
		else if (types[i]->hash_code() == typeid(long long).hash_code())
		{
			size += sizeof(long long);
		}
		else if (types[i]->hash_code() == typeid(short).hash_code())
		{
			size += sizeof(short);
		}
		else if (types[i]->hash_code() == typeid(unsigned char).hash_code())
		{
			size += sizeof(unsigned char);
		}	
		else if (types[i]->hash_code() == typeid(unsigned short).hash_code())
		{
			size += sizeof(unsigned short);
		}
		else if (types[i]->hash_code() == typeid(bool).hash_code())
		{
			size += sizeof(bool);
		}
		else if (types[i]->hash_code() == typeid(minecraft::string).hash_code())
		{
			size += std::any_cast<minecraft::string>(values[i]).len + 1;
		}
		else if (types[i]->hash_code() == typeid(minecraft::string_tag).hash_code())
		{
			size += std::any_cast<minecraft::string_tag>(values[i]).len + strlen("text") + 7;
		}
		else if (types[i]->hash_code() == typeid(minecraft::varint).hash_code())
		{
			std::size_t len = 0;
			std::string buf;
			len = WriteUleb128(buf ,std::any_cast<minecraft::varint>(values[i]).num);
			size += len;
		}
		else if (types[i]->hash_code() == typeid(minecraft::uuid).hash_code())
		{
			size += 16;
		}
		else if (types[i]->hash_code() == typeid(minecraft::node_root).hash_code())
		{
			size += sizeof(char) * 2 + std::any_cast<minecraft::node_root>(values[i]).children_index.size(); 
		}
		else if (types[i]->hash_code() == typeid(minecraft::node_literal).hash_code())
		{
			minecraft::node_literal root = std::any_cast<minecraft::node_literal>(values[i]); 
			size += sizeof(char) * 3 + root.children_index.size() + root.name.len; 
		}
		else if (types[i]->hash_code() == typeid(minecraft::node_argument).hash_code())
		{
			minecraft::node_argument root = std::any_cast<minecraft::node_argument>(values[i]); 
			size += sizeof(char) * 5 + root.children_index.size() + root.name.len; 
		}
		else if (types[i]->hash_code() == typeid(minecraft::paletted_container).hash_code())
		{
			minecraft::paletted_container cont = std::any_cast<minecraft::paletted_container>(values[i]);
			size += (1 * 3) + (sizeof(long) * cont.data_lenght.num);
		}
		else if (types[i]->hash_code() == typeid(minecraft::chunk).hash_code())
		{
			minecraft::chunk chun = std::any_cast<minecraft::chunk>(values[i]);
			size += chun.size();
		}
	}
	return size;
}

void pkt_send(std::vector<const std::type_info*> types, std::vector<std::any> values, User user, int id, bool headless = false)
{
	std::string a;
	std::string buf;
	int fd = user.get_socket();
	int size = 0;
	if (headless)
		size = calc_len(types, values);
	else
		size = calc_len(types, values) + 1;
	std::cout << size << std::endl;
	if (headless == false)
		send_varint(fd, size);
		send_varint(fd, id);
	for (int i = 0; i < types.size(); i++)
	{
		if (types[i]->hash_code() == typeid(int).hash_code())
		{
			int a = std::any_cast<int>(values[i]);
			int conv = htobe32((*(uint32_t *)&a));
			send(fd, &conv, sizeof(int), 0);
		}
		else if (types[i]->hash_code() == typeid(unsigned int).hash_code())
		{
			unsigned int a = std::any_cast<unsigned int>(values[i]);
			send(fd, &a, sizeof(unsigned int), 0);
		}
		else if (types[i]->hash_code() == typeid(char).hash_code())
		{
			char a = std::any_cast<char>(values[i]);
			send(fd, &a, sizeof(char), 0);
		}
		else if (types[i]->hash_code() == typeid(float).hash_code())
		{
			float a = std::any_cast<float>(values[i]);
			unsigned int conv = htobe32((*(uint32_t *)&a));
			send(fd, &conv, sizeof(unsigned int), 0);
		}
		else if (types[i]->hash_code() == typeid(double).hash_code())
		{
			double a = std::any_cast<double>(values[i]);
			unsigned long conv = htobe64((*(uint64_t *)&a));
			send(fd, &conv, sizeof(unsigned long), 0);
		}
		else if (types[i]->hash_code() == typeid(long).hash_code())
		{
			long a = std::any_cast<long>(values[i]);
			send(fd, &a, sizeof(long), 0);
		}
		else if (types[i]->hash_code() == typeid(unsigned long).hash_code())
		{
			unsigned long a = std::any_cast<unsigned long>(values[i]);
			send(fd, &a, sizeof(unsigned long), 0);
		}
		else if (types[i]->hash_code() == typeid(long long).hash_code())
		{
			long long a = std::any_cast<long long>(values[i]);
			send(fd, &a, sizeof(long long), 0);
		}
		else if (types[i]->hash_code() == typeid(short).hash_code())
		{
			short a = std::any_cast<short>(values[i]);
			short conv = htobe16((*(uint16_t *)&a));
			send(fd, &conv, sizeof(short), 0);
		}
		else if (types[i]->hash_code() == typeid(unsigned char).hash_code())
		{
			unsigned char a = std::any_cast<unsigned char>(values[i]);
			send(fd, &a, sizeof(unsigned char), 0);
		}	
		else if (types[i]->hash_code() == typeid(unsigned short).hash_code())
		{
			unsigned short a = std::any_cast<unsigned short>(values[i]);
			send(fd, &a, sizeof(unsigned short), 0);
		}
		else if (types[i]->hash_code() == typeid(bool).hash_code())
		{
			bool a = std::any_cast<bool>(values[i]);
			send(fd, &a, sizeof(bool), 0);
		}	
		else if (types[i]->hash_code() == typeid(minecraft::string).hash_code())
		{
			struct minecraft::string str = std::any_cast<minecraft::string>(values[i]);
			send_string(fd, str);
		}
		else if (types[i]->hash_code() == typeid(minecraft::string_tag).hash_code())
		{
			struct minecraft::string_tag str = std::any_cast<minecraft::string_tag>(values[i]);
			std::string buf;
			char id = 0x08;
			char comp = 0x0a;
			std::string text = "text";
			short lenght = 4;
			char zero = 0;
			short len = str.len;
			short len2 = lenght;
			lenght = htobe16(*(uint16_t*)&lenght);
			str.len = htobe16(*(uint16_t*)&str.len);
			
			send(fd, &comp, sizeof(char), 0);
			send(fd, &id, sizeof(char), 0);
			send(fd, &lenght, sizeof(short), 0);
			send(fd, text.c_str(), len2, 0);
			send(fd, &str.len, sizeof(short), 0);
			send(fd, str.string.c_str(), len, 0);
			send(fd, &zero, sizeof(char), 0);
		}
		else if (types[i]->hash_code() == typeid(minecraft::uuid).hash_code())
		{
			struct minecraft::uuid str = std::any_cast<minecraft::uuid>(values[i]);
			send(fd, str.data.c_str(), str.data.length(), 0);
		}
		else if (types[i]->hash_code() == typeid(minecraft::varint).hash_code())
		{
			minecraft::varint a = std::any_cast<minecraft::varint>(values[i]);
			send_varint(fd, a.num);
		}
		else if (types[i]->hash_code() == typeid(minecraft::node_root).hash_code())
		{
			minecraft::node_root root = std::any_cast<minecraft::node_root>(values[i]); 
			send(fd, &root.flag, sizeof(char), 0);
			send(fd, &root.children_num.num, sizeof(char), 0);
			for (int i = 0; i < root.children_index.size(); i++)
			{
				send(fd, &root.children_index[i].num, sizeof(char), 0);
			}
		}
		else if (types[i]->hash_code() == typeid(minecraft::node_literal).hash_code())
		{
			minecraft::node_literal root = std::any_cast<minecraft::node_literal>(values[i]); 
			send(fd, &root.flag, sizeof(char), 0);
			send_varint(fd, root.children_num.num);
			for (int i = 0; i < root.children_index.size(); i++)
			{
				send_varint(fd, root.children_index[i].num);
			}
			send_string(fd, root.name);
		}
		else if (types[i]->hash_code() == typeid(minecraft::node_argument).hash_code())
		{
			minecraft::node_argument root = std::any_cast<minecraft::node_argument>(values[i]); 
			send(fd, &root.flag, sizeof(char), 0);
			send_varint(fd, root.children_num.num);
			for (int i = 0; i < root.children_index.size(); i++)
			{
				send_varint(fd, root.children_index[i].num);
			}
			send_string(fd, root.name);
			send_varint(fd, root.parser_id.num);
			send_varint(fd, root.varies.num);
		}
		else if (types[i]->hash_code() == typeid(minecraft::paletted_container).hash_code())
		{
			minecraft::paletted_container cont = std::any_cast<minecraft::paletted_container>(values[i]);

			send(fd, &cont.bits_per_entry, sizeof(unsigned char), 0);
			//send_varint(fd, cont.palette_data_entries.num);
			for (int i = 0; i < cont.block_ids.size(); i++)
			{
				send_varint(fd, cont.block_ids[i].num);
			}
			send_varint(fd, cont.data_lenght.num);

			for (int i = 0; i < cont.block_indexes.size(); i++)
			{
				send(fd, &cont.block_indexes[i], sizeof(long), 0);
			}
		}
		else if (types[i]->hash_code() == typeid(minecraft::chunk).hash_code())
		{
			minecraft::chunk chun = std::any_cast<minecraft::chunk>(values[i]);

			for (int x = 0; x < chun.chunks.size(); x++)
			{
				chun.chunks[x].block_count = htobe16(*(uint16_t*)&chun.chunks[x].block_count);
				send(fd, &chun.chunks[x].block_count, sizeof(short), 0);
				minecraft::paletted_container cont = chun.chunks[x].blocks;

				send(fd, &cont.bits_per_entry, sizeof(unsigned char), 0);
				//send_varint(fd, cont.palette_data_entries.num);
				send_varint(fd, cont.block_ids[0].num);
				send_varint(fd, cont.data_lenght.num);

				/*for (int i = 0; i < cont.block_indexes.size(); i++)
				{
					send(fd, &cont.block_indexes[i], sizeof(long), 0);
				}*/
				
				minecraft::paletted_container cont2 = chun.chunks[x].biome;

				send(fd, &cont2.bits_per_entry, sizeof(unsigned char), 0);
				//send_varint(fd, cont2.palette_data_entries.num);
				send_varint(fd, cont2.block_ids[0].num);
				send_varint(fd, cont2.data_lenght.num);

				/*for (int i = 0; i < cont2.block_indexes.size(); i++)
				{
					send(fd, &cont2.block_indexes[i], sizeof(long), 0);
				}*/
			}
		}
	}
}
