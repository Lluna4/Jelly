#include "packet_send_rw.hpp"
#include <iostream>
#include <vector>
#include <typeinfo>

int main()
{
	std::vector<const std::type_info*> v = {&typeid(int), &typeid(char), &typeid(float), &typeid(double)};
	pkt_send(v);	
}
