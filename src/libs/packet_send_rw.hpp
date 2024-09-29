#pragma once
#include <typeinfo>
#include <variant>
#include <vector>
#include <iostream>
#include <any>
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
		if (types[i]->hash_code() == typeid(unsigned int).hash_code())
		{
			size += sizeof(unsigned int);
		}
		else if (types[i]->hash_code() == typeid(char).hash_code())
		{
			size += sizeof(char);
		}
		else if (types[i]->hash_code() == typeid(float).hash_code())
		{
			size += sizeof(float);
		}
		else if (types[i]->hash_code() == typeid(double).hash_code())
		{
			size += sizeof(double);
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
		else if (types[i]->hash_code() == typeid(minecraft::varint).hash_code())
		{
			size += 1;
		}
		else if (types[i]->hash_code() == typeid(minecraft::uuid).hash_code())
		{
			size += 16;
		}
	}
	return size;
}

void pkt_send(std::vector<const std::type_info*> types, std::vector<std::any> values, int sock, int id)
{
	std::string a;
	std::string buf;
	int fd = sock;
	int size = calc_len(types, values) + 1;
	std::cout << size << std::endl;
	send_varint(fd, size);
	send_varint(fd, id);
	for (int i = 0; i < types.size(); i++)
	{
		if (types[i]->hash_code() == typeid(int).hash_code())
		{
			int a = std::any_cast<int>(values[i]);
			send(fd, &a, sizeof(int), 0);
		}
		if (types[i]->hash_code() == typeid(unsigned int).hash_code())
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
			send(fd, &a, sizeof(float), 0);
		}
		else if (types[i]->hash_code() == typeid(double).hash_code())
		{
			double a = std::any_cast<double>(values[i]);
			send(fd, &a, sizeof(double), 0);
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
			send(fd, &a, sizeof(short), 0);
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
			send_varint(fd, str.len);
			send(fd, str.str.c_str(), str.str.length(), 0);
		}
		else if (types[i]->hash_code() == typeid(minecraft::uuid).hash_code())
		{
			struct minecraft::uuid str = std::any_cast<minecraft::uuid>(values[i]);
			send(fd, str.buff, str.len, 0);
		}
		else if (types[i]->hash_code() == typeid(minecraft::varint).hash_code())
		{
			minecraft::varint a = std::any_cast<minecraft::varint>(values[i]);
			send_varint(fd, a.num);
		}	
	}
}
