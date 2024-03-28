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

std::vector<packet> process_packet(char *pkt)
{
	int lenght = 0;
	char *next = NULL;
    std::vector<packet> packets;
	struct packet p = {0};
	char *data = NULL;
	p.id = 0;
	while(1)
	{
		if (*pkt == '\0')
			break;
        p.size = *pkt;
		next = pkt + (p.size + 1);
		pkt++;
        p.id = *pkt;
		data = (char *)calloc(p.size, sizeof(char));
		pkt++;
		std::memcpy(data, pkt, p.size - 1);
		p.data = mem_dup(data, p.size);
		free(data);
		pkt = next;
		packets.push_back(p);
	}
    return packets;
}
