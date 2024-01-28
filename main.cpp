#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
//include <format>
#include <string.h>
#include <thread>
#include <uuid_v4/uuid_v4.h>
#include <uuid_v4/endianness.h>

int stat = 0;

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

std::size_t WriteUleb128(std::string &dest, unsigned long val) {
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

void send_pkt_succ(std::string uname, UUIDv4::UUID uuid, int sock)
{
	std::string pkt;
	int pos = 0;
	pos = (int)WriteUleb128(pkt, uname.length() + 16 + 3);
	pkt.push_back(0x02);
	pkt.append(uuid.bytes());
    pkt.push_back((int)uname.length());
	pkt.append(uname);
	WriteUleb128(pkt, 0x00);
	send(sock, pkt.c_str(), pkt.length(), 0);
	std::cout <<  uname.length() + 16 + 3 << 4 << uuid.str() << uname.length() << uname << 0 << std::endl;
	std::cout <<  uuid.bytes() << std::endl;
	std::cout << pkt << std::endl;
	stat = -1;
}

void manage_login(unsigned long len, char *pkt, std::string &uname, UUIDv4::UUID &uuid, int sock)
{
	unsigned long str = 0;
	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	ReadUleb128(pkt, &str);
	uname = pkt;
	uname = uname.substr(1, str);
	std::cout << pkt << std::endl;
	std::cout << uname << std::endl;
	pkt = pkt + str + 1;
	uuid = uuidGenerator.getUUID();
	std::cout << uuid.str() << std::endl;
	send_pkt_succ(uname, uuid, sock);
}

void manage_pkt(unsigned long len, char *pkt)
{
	unsigned long ver = 0;
	unsigned long str = 0;
	std::size_t prot_size = 0;
	prot_size = ReadUleb128(pkt, &ver);
	if (ver == 765)
	{
		pkt = pkt + prot_size;
		ReadUleb128(pkt, &str);
		pkt = pkt + 1;
		//std::cout << pkt << std::endl;
		pkt = pkt + (str + 2);
		std::cout << (int)*pkt << std::endl;
		stat = (int)*pkt;
	}
}

void manage_sv(int sock)
{
	char buffer[1024] = {0};
	char *ptr = buffer;
	char *buff2;
	unsigned long len = 0;
	std::string uname;
	UUIDv4::UUID uuid;
	
	while(1)
	{
		if (*ptr == '\0')
		{
			ptr = buffer;
			recv(sock, buffer, 1023, 0);
			len = (unsigned long)buffer[0];
		}
		else
		{
			len = (unsigned long)*ptr;
		}
		buff2 = (char *)calloc(len + 1, sizeof(char));

		ptr = ptr + 2;
		strncpy(buff2, ptr, len);
		if (stat == 0)
			manage_pkt(len, buff2);
		else if (stat == 2)
		{
			manage_login(len, buff2, uname, uuid, sock);
		}
		free(buff2);
		ptr = ptr + len;
	}
}

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = {
		AF_INET,
		htons(25565),
		0
	};
	if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		std::cout << "Bind failed\n";
		return -1;
	}
	while (1)
	{
		int client_fd = 0;
		listen(sockfd, 32);
		client_fd = accept(sockfd, nullptr, nullptr);
		std::thread cli_th(manage_sv, client_fd);
		cli_th.detach();
	}
}
