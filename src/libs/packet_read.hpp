#include "utils.hpp"
#include <vector>
#include <map>
#include <any>
#include <string>
#include <cstring>
#include <string.h>
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

std::map<std::string, std::any> pkt_read(packet p, indexed_map types)
{
    std::map<std::string, std::any> ret;
    int size = p.buf_size;
    char *data = p.data;


    for (int i = 0; i < types.index.size(); i++)
    {
        auto var = types.map[types.index[i]];
        if (var->hash_code() == typeid(char).hash_code())
        {
            if (size < sizeof(char))
                break;
            ret.insert({types.index[i], *data});
            data++;
            size--;
        }
        else if (var->hash_code() == typeid(unsigned char).hash_code())
        {
            if (size < sizeof(unsigned char))
                break;
            ret.insert({types.index[i], (unsigned char)*data});
            data++;
            size--;
        }
        else if (var->hash_code() == typeid(bool).hash_code())
        {
            if (size < sizeof(bool))
                break;
            ret.insert({types.index[i], (bool)*data});
            data++;
            size--;
        }
        else if (var->hash_code() == typeid(short).hash_code())
        {
            if (size < sizeof(short))
                break;
            short num = 0;

            std::memcpy(&num, data, sizeof(short));
            num = be16toh(num);
            ret.insert({types.index[i], num});
            data += 2;
            size -= 2;
        }
        else if (var->hash_code() == typeid(unsigned short).hash_code())
        {
            if (size < sizeof(unsigned short))
                break;
            unsigned short num = 0;

            std::memcpy(&num, data, sizeof(unsigned short));
            num = be16toh(num);
            ret.insert({types.index[i], num});
            data += 2;
            size -= 2;
        }
        else if (var->hash_code() == typeid(int).hash_code())
        {
            if (size < sizeof(int))
                break;
            int num = 0;

            std::memcpy(&num, data, sizeof(int));
            num = be32toh(num);
            ret.insert({types.index[i], num});
            data += 4;
            size -= 4;
        }
        else if (var->hash_code() == typeid(unsigned int).hash_code())
        {
            if (size < sizeof(unsigned int))
                break;
            unsigned int num = 0;

            std::memcpy(&num, data, sizeof(unsigned int));
            num = be32toh(num);
            ret.insert({types.index[i], num});
            data += 4;
            size -= 4;
        }
        else if (var->hash_code() == typeid(long).hash_code())
        {
            if (size < sizeof(long))
                break;
            long num = 0;

            std::memcpy(&num, data, sizeof(long));
            num = be64toh(num);
            ret.insert({types.index[i], num});
            data += 8;
            size -= 8;
        }
        else if (var->hash_code() == typeid(unsigned long).hash_code())
        {
            if (size < sizeof(unsigned long))
                break;
            unsigned long num = 0;

            std::memcpy(&num, data, sizeof(unsigned long));
            num = be64toh(num);
            ret.insert({types.index[i], num});
            data += 8;
            size -= 8;
        }
        else if (var->hash_code() == typeid(float).hash_code())
        {
            if (size < sizeof(float))
                break;
            ret.insert({types.index[i], read_float(data)});
            data += 4;
            size -= 4;
        }
        else if (var->hash_code() == typeid(double).hash_code())
        {
            if (size < sizeof(double))
                break;

            ret.insert({types.index[i], read_double(data)});
            data += 8;
            size -= 8;
        }
        else if (var->hash_code() == typeid(minecraft::string).hash_code())
        {
            if (size < 1)
                break;
            unsigned long s = 0;
            std::size_t varint_size = ReadUleb128(data, &s);
            std::string str;
            
            if (size < (s + varint_size))
                break;
            data += varint_size;
            str = data;
            str = str.substr(0, s);
            ret.insert({types.index[i], (minecraft::string){.len = s, .string = str}});
            data += s;
            size += (s + varint_size);
        }
        else if (var->hash_code() == typeid(minecraft::varint).hash_code())
        {
            if (size < 1)
                break;
            unsigned long num = 0;
            std::size_t varint_size = ReadUleb128(data, &num);
            
            ret.insert({types.index[i], (minecraft::varint){.num = num}});
            data += varint_size;
            size += varint_size;
        }
    }
    return ret;
}