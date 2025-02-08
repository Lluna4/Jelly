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



std::vector<packet> process_packet(char_size *pkt, int sock, int status)
{
	int lenght = 0;
	char *next = NULL;
    std::vector<packet> packets;
	struct packet p = {0};
	char *data = NULL;
	p.id = 0;
	if (*(pkt->start_data) == '\0')
		return packets;
	std::tuple<minecraft::varint, minecraft::varint> header;
	packet pkt_internal = {.id = 0, .size = 1024, .buf_size =1024, .data = pkt->start_data, .start_data = pkt->start_data, .sock = 0};
	header = read_packet(header, pkt_internal);
	if (std::get<0>(header).num > status)
	{
		if (std::get<0>(header).num + pkt->consumed_size > pkt->max_size)
		{
			pkt->start_data = (char *)realloc(pkt->start_data, pkt->max_size + std::get<0>(header).num + pkt->consumed_size + 1024);
			pkt->max_size += std::get<0>(header).num + pkt->consumed_size + 1024;
			pkt->data = pkt->start_data + pkt->consumed_size;
		}
		recv(sock, pkt->data, std::get<0>(header).num - status, 0);
	}
	p.data = mem_dup(&(pkt->start_data[std::get<0>(header).size + std::get<1>(header).size]), std::get<0>(header).num);
	p.id = std::get<1>(header).num;
	p.size = std::get<0>(header).num;
	p.start_data = p.data;
	p.sock = sock;
	packets.push_back(p);
    return packets;
}