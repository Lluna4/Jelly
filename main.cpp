#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
//include <format>
#include <string.h>
#include <thread>
#include <vector>
#include "packet_manager.hpp"

int stat = 0;
std::vector<User> users;



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
}

User manage_login(unsigned long len, char *pkt, int sock)
{
	unsigned long str = 0;
	ReadUleb128(pkt, &str);
	std::string uname;
	uname = pkt;
	uname = uname.substr(1, str);
	pkt = pkt + str + 1;
	User new_user(uname, sock);
	send_pkt_succ(uname, new_user.get_uuid(), sock);
	log("New user ");
	log("Name: ", uname);
	log("UUID: ", new_user.get_uuid().str());
	return new_user;
}

void manage_pkt(unsigned long len, char *pkt, int *stat)
{
	unsigned long ver = 0;
	unsigned long str = 0;
	std::size_t prot_size = 0;
	prot_size = ReadUleb128(pkt, &ver);
	log("Protocol version ", ver);
	if (ver == 765)
	{
		pkt = pkt + prot_size;
		ReadUleb128(pkt, &str);
		pkt = pkt + 1;
		//std::cout << pkt << std::endl;
		pkt = pkt + (str + 2);
		log((int)*pkt);
		*stat = (int)*pkt;
	}
}

void manage_sv(int sock)
{
	log("New client");
	std::vector<packet> packets;
	std::thread manager(pkt_manager, sock, std::ref(packets));
	manager.detach();
	int status = 0;
	User th_user;
	while (1)
	{
		if (packets.empty() == false)
		{
			switch (packets.begin()->id)
			{
				case 0x00:
					log("HS packet ", status);
					if (status == 0)
						manage_pkt(packets.begin()->lenght, (char *)packets.begin()->data.c_str(), &status);
					else if (status == 2)
					{	
						User buff = manage_login(packets.begin()->lenght, (char *)packets.begin()->data.c_str(), sock);
						th_user.set_socket(buff.get_socket());
						th_user.set_uname(buff.get_uname());
						th_user.set_uuid(buff.get_uuid());
						status = -1;
					}
					break;
			}
			packets.erase(packets.begin());
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = {
		AF_INET,
		htons(25566),
		0
	};
	log("This server is for testing and educational purposes, it will never be as good as vanilla/paper servers");
	if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		log_err("Bind failed");
		return -1;
	}
	log("Server listening at ", "0.0.0.0:25566");
	while (1)
	{
		int client_fd = 0;
		listen(sockfd, 32);
		client_fd = accept(sockfd, nullptr, nullptr);
		std::thread cli_th(manage_sv, client_fd);
		cli_th.detach();
	}
}
