#pragma once
#include <typeinfo>
#include <variant>
#include <vector>
#include <iostream>
#include <any>

int calc_len(std::vector<const std::type_info*> types)
{
	int size = 0; 
	for (int i = 0; i < types.size(); i++)
	{
		if (types[i]->hash_code() == typeid(int).hash_code())
		{
			size += sizeof(int);
		}
		else if (types[i]->hash_code() == typeid(char).hash_code())
		{
			size += sizeof(char);
		}
		else if (types[i]->hash_code() == typeid(float).hash_code())
		{
			size += sizeof(float);
		}	
	}
}

void pkt_send(std::vector<const std::type_info*> types, std::vector<std::any> values)
{
	std::string a;
	for (int i = 0; i < types.size(); i++)
	{
		std::cout << types[i]->name() << std::endl;
		a = types[i]->name();
	}
}
