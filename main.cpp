#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread_manager.hpp"
#include "logging.hpp"
#include "packet_processing.hpp"
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>

int SV_PORT = 25565;
thread_man manager;

int execute_pkt(packet p, int state)
{
	int ret = 0;
	switch (p.id)
	{
		case 0:
			if (state == 0)
				ret = (int)*(p.data + p.size - 2);
			else if (state == 2)

			break;
	}
	return ret;
}

void manage_client(std::stop_token stoken, int sock)
{
	int status = 0;
	std::vector<packet> packets;
	std::function<void(std::stop_token, std::vector<packet> &, int)> func = packet_reader;
	manager.add_thread(func, sock, packets);
	while (1)
	{
		if (packets.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		log("New packet");
		log("Id: ", packets.begin()->id);
		log("Size: ", packets.begin()->size);
		std::cout << "Data: ";
		for (int i = 0; i < strlen(packets.begin()->data);i++)
		{
			printf("%02hhX ", (int)packets.begin()->data[i]);
		}
		std::cout << "\n";
		status = execute_pkt(packets[0], status);
		log("New state: ", status);
		packets.erase(packets.begin());
	}
}

int main()
{
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);
	struct sockaddr_in addr =
	{
		AF_INET, 
		htons(SV_PORT),
		0
	};

	log("This server is for testing and educational purposes, it will never be as good as vanilla/paper servers");
	if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == -1)
	{
		log_err("Bind failed");
		return -1;
	}
	log("Server listening at ", "0.0.0.0:25566");
	while (1)
	{
		int client_fd = 0;
	        listen(sock, 32);
        	client_fd = accept(sock, nullptr, nullptr);
       		if (client_fd < 0) 
        	{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
           		continue;
        	}	
		std::function<void(std::stop_token, int)> func = manage_client;
		manager.add_thread(func, client_fd);
	}
}
