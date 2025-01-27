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



packet process_packet(char_size *pkt, int sock, int status)
{
	int lenght = 0;
	char *next = NULL;
	struct packet p = {0};
	char *data = NULL;
	p.id = 0;
	if (*(pkt->start_data) == '\0')
		return p;
	std::tuple<minecraft::varint, minecraft::varint> header;
	header = read_packet(header, pkt->start_data);
	p.data = mem_dup(&(pkt->start_data[std::get<0>(header).size + std::get<1>(header).size]), std::get<0>(header).num);
	p.id = std::get<1>(header).num;
	p.size = std::get<0>(header).num;
	p.start_data = p.data;
	p.sock = sock;
    return p;
}
