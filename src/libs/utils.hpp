#pragma once
#include <iostream>
#include <string.h>
#include <sys/socket.h>

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
  

  	while (1) {
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
	str.push_back(src.length());
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
	double num = 0.0f;
	memcpy(&num, buf, sizeof(double));
	return num;
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

	struct uuid
	{
		std::string data;
	};
}

void send_varint(int fd, unsigned long val)
{
	std::string buf;
	WriteUleb128(buf, val);
	send(fd, buf.c_str(), buf.length(), 0);
}
