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

std::map<std::string, std::any> pkt_read(packet p, std::map<std::string, const std::type_info*> types)
{
    std::map<std::string, std::any> ret;
    int size = p.size;
    char *data = p.data;

    for (auto &var: types)
    {
        if (var.second->hash_code() == typeid(char).hash_code())
        {
            if (size < sizeof(char))
                break;
            ret.insert({var.first, *data});
            data++;
            size--;
        }
        else if (var.second->hash_code() == typeid(unsigned char).hash_code())
        {
            if (size < sizeof(unsigned char))
                break;
            ret.insert({var.first, (unsigned char)*data});
            data++;
            size--;
        }
        else if (var.second->hash_code() == typeid(short).hash_code())
        {
            if (size < sizeof(short))
                break;
            short num = 0;

            std::memcpy(&num, data, sizeof(short));
            num = be16toh(num);
            ret.insert({var.first, num});
            data += 2;
            size -= 2;
        }
        else if (var.second->hash_code() == typeid(unsigned short).hash_code())
        {
            if (size < sizeof(unsigned short))
                break;
            unsigned short num = 0;

            std::memcpy(&num, data, sizeof(unsigned short));
            num = be16toh(num);
            ret.insert({var.first, num});
            data += 2;
            size -= 2;
        }
        else if (var.second->hash_code() == typeid(int).hash_code())
        {
            if (size < sizeof(int))
                break;
            int num = 0;

            std::memcpy(&num, data, sizeof(int));
            num = be32toh(num);
            ret.insert({var.first, num});
            data += 4;
            size -= 4;
        }
        else if (var.second->hash_code() == typeid(unsigned int).hash_code())
        {
            if (size < sizeof(unsigned int))
                break;
            unsigned int num = 0;

            std::memcpy(&num, data, sizeof(unsigned int));
            num = be32toh(num);
            ret.insert({var.first, num});
            data += 4;
            size -= 4;
        }
        else if (var.second->hash_code() == typeid(long).hash_code())
        {
            if (size < sizeof(long))
                break;
            long num = 0;

            std::memcpy(&num, data, sizeof(long));
            num = be64toh(num);
            ret.insert({var.first, num});
            data += 8;
            size -= 8;
        }
        else if (var.second->hash_code() == typeid(unsigned long).hash_code())
        {
            if (size < sizeof(unsigned long))
                break;
            unsigned long num = 0;

            std::memcpy(&num, data, sizeof(unsigned long));
            num = be64toh(num);
            ret.insert({var.first, num});
            data += 8;
            size -= 8;
        }
        else if (var.second->hash_code() == typeid(float).hash_code())
        {
            if (size < sizeof(float))
                break;
            float num = 0.0f;

            std::memcpy(&num, data, sizeof(float));
            num = be32toh(num);
            ret.insert({var.first, num});
            data += 4;
            size -= 4;
        }
        else if (var.second->hash_code() == typeid(double).hash_code())
        {
            if (size < sizeof(double))
                break;
            double num = 0.0f;

            std::memcpy(&num, data, sizeof(double));
            num = be32toh(num);
            ret.insert({var.first, num});
            data += 8;
            size -= 8;
        }
    }
    return ret;
}