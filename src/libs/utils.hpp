#pragma once
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <vector>
#include <openssl/evp.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <bitset>

struct packet
{
	int id;
	int size;
	char *data;
};

std::size_t ReadUleb128(const char* addr, unsigned long* ret)
{
  unsigned long result = 0;
  int shift = 0;
  std::size_t count = 0;


  	while (1) 
	{
		unsigned char byte = *reinterpret_cast<const unsigned char*>(addr);
		addr++;
		count++;

		result |= (byte & 0x7f) << shift;
		shift += 7;

		if (!(byte & 0x80)) break;
  	}

  *ret = result;

  return count;
}

std::size_t WriteUleb128(std::string &dest, unsigned long val)
{
  std::size_t count = 0;

  do {
	unsigned char byte = val & 0x7f;
	val >>= 7;

	if (val != 0)
	  byte |= 0x80;  // mark this byte to show that more bytes will follow

	dest.push_back(byte);
	count++;
  } while (val != 0);

  return count;
}

void write_string(std::string &str, std::string src)
{
	WriteUleb128(str, src.length());
	str.append(src);
}



unsigned long read_string(char *str, std::string &dest)
{
	unsigned long ret = 0;

	ReadUleb128(str, &ret);
	str++;
	dest = str;
	dest = dest.substr(0, ret);
	return ret;
}


std::string read_string(char *str)
{
	unsigned long ret = 0;
	std::string dest;

	ReadUleb128(str, &ret);
	str++;
	dest = str;
	dest = dest.substr(0, ret);
	return dest;
}

double read_double(char *buf)
{
	uint64_t num_as_uint64;
	double num;

	memcpy(&num_as_uint64, buf, sizeof(uint64_t));
	num_as_uint64 = be64toh(num_as_uint64);
	memcpy(&num, &num_as_uint64, sizeof(double));

	return num;
}

float read_float(char *buf)
{
	uint32_t num_as_uint32;
	float num;

	memcpy(&num_as_uint32, buf, sizeof(uint32_t));
	num_as_uint32 = be32toh(num_as_uint32);
	memcpy(&num, &num_as_uint32, sizeof(float));

	return num;
}

long read_position(char *buf)
{
	uint64_t num_as_uint64;
	long num;

	memcpy(&num_as_uint64, buf, sizeof(uint64_t));
	num_as_uint64 = be64toh(num_as_uint64);
	memcpy(&num, &num_as_uint64, sizeof(long));

	return num;
}

char *mem_dup(char *buf, int size)
{
	char *ret = (char *)calloc(size, sizeof(char));
	memcpy(ret, buf, (size - 1));
	return ret;
}

char	*ft_strdup(const char *s1)

{
	int		n;
	char	*ret;

	n = strlen(s1);
	ret = (char *)malloc((n + 1) * sizeof(char));
	if (!ret)
		return (0);
	ret = (char *)memcpy(ret, s1, n + 1);
	return (ret);
}

std::string forge_packet(packet pkt)
{
	std::string ret;
	ret.push_back(pkt.size + 1);
	ret.push_back(pkt.id);
	ret.append(pkt.data);
	return ret;
}

namespace minecraft
{
	struct string
	{
		unsigned long len;
		std::string string;
	};

	struct varint
	{
		unsigned long num;
	};

	struct uuid {
		unsigned char *buff = (unsigned char *)calloc(17, sizeof(char));
		std::string uuid;
		unsigned int len = 0;

		void generate(std::string uname) 
		{
			EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
			int ret = EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
			EVP_DigestUpdate(mdctx, uname.c_str(), uname.length());
			EVP_DigestFinal_ex(mdctx, buff, &len);
			EVP_MD_CTX_destroy(mdctx); // Clean up context
			std::string buf((char *)buff);
		}
	};

	struct string_tag
	{
		short len;
		std::string string;
	};

	struct varint read_varint(char *buf)
	{
		unsigned long ret = 0;

		ReadUleb128(buf, &ret);
		return (varint){.num = ret};
	}

	struct node_root
	{
		char flag = 0x00;
		varint children_num;
		std::vector<varint> children_index;
	};

	struct node_literal
	{
		char flag = 0x01;
		varint children_num;
		std::vector<varint> children_index;
		string name;
	};

	struct node_argument
	{
		char flag = 0x02;
		varint children_num;
		std::vector<varint> children_index;
		string name;
		varint parser_id;
		varint varies;
	};
}

void send_varint(int fd, unsigned long val)
{
	std::string buf;
	WriteUleb128(buf, val);
	send(fd, buf.c_str(), buf.length(), 0);
}

void send_string(int fd, minecraft::string str)
{
	send_varint(fd, str.len);
	send(fd, str.string.c_str(), str.string.length(), 0);
}

static std::vector<std::bitset<6>> proc_6bit(std::string hi)
{
    std::string bits;
    std::vector<std::bitset<8>> vec;
    std::vector<std::bitset<6>> vec2;
    int index = 0;
    std::string res;
    for (int i = 0; i < hi.length(); i++)
    {
        vec.emplace_back(hi[i]);
    }
    for (int i = 0; i < vec.size(); i++)
        bits.append(vec[i].to_string());

    while(index < bits.length())
    {
        if (index > 0 && index%6 == 0)
        {
            vec2.emplace_back(res);
            res.clear();
        }
        res.push_back(bits[index]);
        index++;
    }
	while (res.length() < 6)
		res.push_back('0');
	vec2.emplace_back(res);
	
	return vec2;
}

std::string base64_encode(std::string file_path)
{
	std::string ret;
	if (std::filesystem::exists(file_path))
	{
		std::ifstream file(file_path, std::ios::binary);
		std::string buf;
		std::stringstream buffer;
		buffer << file.rdbuf();
		buf = buffer.str();
		std::vector<std::bitset<6>>nums = proc_6bit(buf); //converts 8 bit nums into a 6 bit array

		for (int i = 0; i < nums.size();i++)
		{
			unsigned long num = nums[i].to_ulong();
			if (num < 26)
				ret.push_back(num+ 'A');
			else if (num >= 26 && num < 52)
				ret.push_back((num - 26) + 'a');
			else if(num >= 52 && num < 62)
				ret.push_back((num - 52) + '0');
			else if (num == 62)
				ret.push_back('+');
			else if (num == 63)
				ret.push_back('/');
		}
		if (ret.length()%4 != 0)
		{
			while(ret.length()%4 != 0)
				ret.push_back('=');
		}
	}
	return ret;
}

std::bitset<26> reverse16(std::bitset<26> set)
{
    int n = static_cast<int>(set.to_ulong());
    return std::bitset<26>((n >> 13) | (n << 13));
}

std::bitset<12> reverse16(std::bitset<12> set)
{
    int n = static_cast<int>(set.to_ulong());
    return std::bitset<12>((n >> 6) | (n << 6));
}

int rem_euclid(int a, int b)
{
	int ret = a %b;
    if (ret < 0)
	{
        ret += b;
    }
	return ret;
}
