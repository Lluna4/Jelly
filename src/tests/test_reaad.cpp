#include "../libs/packet_read.hpp"
#include "../libs/utils.hpp"
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <chrono>
#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#endif

int main()
{
    using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;

    char *buf = (char *)calloc(20, sizeof(char));
    int a = 21;
    int conv = htobe32((*(uint32_t *)&a));
    buf[4] = 'a';
    
    std::memcpy(buf, &conv, sizeof(int));
    packet aa = {0, 5, buf};
    std::map<std::string, const std::type_info*> types = {{"var", &typeid(int)}, {"char", &typeid(char)}};
    std::vector<std::string> index = {"var", "char"};
    indexed_map aaa = {types, index};
    auto t1 = high_resolution_clock::now();
    std::map<std::string, std::any> map = pkt_read(aa, aaa);
	auto t2 = high_resolution_clock::now();
	auto ms_int = duration_cast<milliseconds>(t2 - t1);

	duration<double, std::milli> ms_double = t2 - t1;
	std::cout << "Time to process: ";
	std::cout << ms_double.count() << "ms\n";
    std::cout << std::any_cast<int>(map["var"]) << std::endl;
    std::cout << std::any_cast<char>(map["char"]) << std::endl;
}