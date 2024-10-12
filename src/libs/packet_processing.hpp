#pragma once
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <vector> 
#include "utils.hpp"
#include "logging.hpp"
#include <cstring>
#include <stdlib.h>
#include "comp_time_read.hpp"



std::vector<packet> process_packet(char *pkt, int sock)
{
	int lenght = 0;
	char *next = NULL;
    std::vector<packet> packets;
	struct packet p = {0};
	char *data = NULL;
	p.id = 0;
	if (*pkt == '\0')
		return packets;
	std::tuple<minecraft::varint, minecraft::varint> header;
	header = read_packet(header, pkt);
	pkt += std::get<0>(header).size + std::get<1>(header).size;
	p.data = mem_dup(pkt, std::get<0>(header).num);
	p.id = std::get<1>(header).num;
	p.size = std::get<0>(header).num;
	p.start_data = p.data;
	p.sock = sock;
	packets.push_back(p);
    return packets;
}
