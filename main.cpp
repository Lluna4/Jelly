#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread_manager.hpp"
#include "logging.hpp"
#include "packet_processing.hpp"
#include "user.hpp"
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>

int SV_PORT = 25565;
thread_man manager;

void login_succ(User user)
{
	std::string pkt;
	WriteUleb128(pkt, user.get_uname().length() + 16 + 3);
	pkt.push_back(0x02);
	pkt.append(user.get_uuid().bytes());
	pkt.push_back(user.get_uname().length());
	pkt.append(user.get_uname());
	WriteUleb128(pkt, 0x00);
	send(user.get_socket(), pkt.c_str(), pkt.length(), 0);
	std::cout <<  user.get_uname().length() + 16 + 3 << 4 << user.get_uuid().str() << user.get_uname().length() << user.get_uname() << 0 << std::endl;
}

int execute_pkt(packet p, int state, User &user)
{
	int ret = state;
	unsigned long size = 0;
	std::string uname;
	switch (p.id)
	{
		case 0:
			if (state == 0)
				ret = (int)*(p.data + p.size - 2);
			else if (state == 2)
			{
				ReadUleb128(p.data, &size);
				if (size < 1)
					log_err("Invalid username size!");
				uname = p.data;
				uname = uname.substr(1, size);
				user.set_uname(uname);
				login_succ(user);
				ret = 3;
			}
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
	User user;
	user.set_socket(sock);
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
		status = execute_pkt(packets[0], status, user);
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
