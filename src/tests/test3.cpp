#include "packet_send_rw.hpp"
#include <iostream>
#include <vector>
#include <typeinfo>
#include "utils.hpp"

int main()
{
	std::vector<const std::type_info*> v = {&typeid(int), &typeid(int), &typeid(char), &typeid(double), &typeid(minecraft::string)};
	struct minecraft::string str = {.len = 2, .string = "hi!"};
	std::vector<std::any> t = {10, 10, 'a', 10.10f, str};
	pkt_send(v, t);	
}
