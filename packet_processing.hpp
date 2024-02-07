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
#include <cstring>
#include <stdlib.h>



void process_packet(char *pkt, std::vector<packet> &packets)
{
	int lenght = 0;
	char *next = NULL;
	struct packet p = {0};
	char *data = NULL;
	
	while(1)
	{
		if (*pkt == '\0')
			break;
		ReadUleb128(pkt, &(p.size));
		next = pkt + (p.size + 1);
		pkt++;
		ReadUleb128(pkt, &(p.id));
		data = (char *)calloc(p.size, sizeof(char));
		pkt++;
		std::memcpy(data, pkt, p.size - 1);
		p.data = strdup(data);
		free(data);
		pkt = next;
		packets.push_back(p);
		
	}
}

void packet_reader(std::stop_token stoken, std::vector<packet> &packets, int sock)
{
	struct pollfd pfds[1];
	int status = 0;
	int num_events = 0;
	char *buffer = (char *)calloc(1024, sizeof(char));
	pfds[0].fd = sock;
	pfds[0].events = POLLIN;
	
	while(1)
	{
		if (stoken.stop_requested())
		{
			close(sock);
			break;
		}
		num_events = poll(pfds, 1, 1000);
		if (num_events == 0)
			continue;
		status = read(sock, buffer, 1024);
		if (status == -1)
			break;
		if (buffer[0] == '\0')
			break;
		process_packet(buffer, packets);
		memset(buffer, 0, 1024);
	}
	free(buffer);
}
