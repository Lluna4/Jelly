#include "../libs/packet_read.hpp"
#include "../libs/utils.hpp"
#include <iostream>
#include <cstring>
#include <stdlib.h>
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
    char *buf = (char *)calloc(20, sizeof(char));
    int a = 21;
    int conv = htobe32((*(uint32_t *)&a));
    buf[4] = 'a';
    
    std::memcpy(buf, &conv, sizeof(int));
    packet aa = {0, 5, buf};
    std::map<std::string, const std::type_info*> types = {{"a", &typeid(int)}, {"b", &typeid(char)}};
    std::map<std::string, std::any> map = pkt_read(aa, types);
    std::cout << std::any_cast<int>(map["a"]) << std::endl;
}