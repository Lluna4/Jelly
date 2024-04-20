#pragma once
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <vector>
#include <openssl/evp.h>
#include <cstdint>

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

	struct paletted_container
	{
		unsigned char bits_per_entry;
		std::vector<varint> block_ids;
		varint data_lenght;
		std::vector<long> block_indexes;
	};

	struct chunk_section
	{
		short block_count;
		paletted_container blocks;
		paletted_container biome;
	};

	struct chunk
	{
		std::vector<chunk_section> chunks;

		int size()
		{
			int ret = 0;
			std::string buf;
			std::size_t size = 0;
			for (int x = 0; x < chunks.size(); x++)
			{
				ret += sizeof(short);
				minecraft::paletted_container cont = chunks[x].blocks;

				ret += sizeof(unsigned char);

				ret += WriteUleb128(buf, cont.block_ids[0].num);

				ret += WriteUleb128(buf, cont.data_lenght.num);

				/*for (int i = 0; i < cont.block_indexes.size(); i++)
				{
					send(fd, &cont.block_indexes[i], sizeof(long), 0);
				}*/
				
				minecraft::paletted_container cont2 = chunks[x].biome;

				ret += sizeof(unsigned char);

				ret += WriteUleb128(buf, cont2.block_ids[0].num);

				ret += WriteUleb128(buf, cont2.data_lenght.num);

				/*for (int i = 0; i < cont2.block_indexes.size(); i++)
				{
					send(fd, &cont2.block_indexes[i], sizeof(long), 0);
				}*/
			}
			return ret;
		}
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