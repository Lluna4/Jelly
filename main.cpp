#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread_manager.hpp"
#include "packet_processing.hpp"
#include "user.hpp"
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>
#include "config.hpp"
#include <filesystem>

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
	log_header();
	std::cout <<  user.get_uname().length() + 16 + 3 << 4 << user.get_uuid().str() << user.get_uname().length() << user.get_uname() << 0 << std::endl;
}

int execute_pkt(packet p, int state, User &user, int index)
{
	int ret = state;
	unsigned long size = 0;
	std::string uname;
	std::string buf;
	unsigned long buf2 = 0;
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
			else if (state == 4)
			{
				buf2 = read_string(p.data, buf);
				user.set_locale(buf);
				user.set_render_distance((int)p.data[buf2 + 1]);
				log("New locale: ", user.get_locale());
				log("New render distance: ", user.get_render_distance());
				buf.clear();
				buf.push_back((dc_msg.length() + 4));
				buf.push_back(0x01);
				buf.push_back(0x08);
				buf.push_back(0x00);
				buf.push_back(dc_msg.length());
				buf.append(dc_msg);
				send(user.get_socket(), buf.c_str(), buf.length(), 0);
				manager.request_stop_thread(index);
				manager.request_stop_thread(index - 1);
			}
			break;
		case 3:
			if (state == 3)
			{
				log("login awknoleged by the client");
				ret = 4;
			}

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
	int index = manager.get_current_index();
	while (1)
	{
		if (stoken.stop_requested())
		{
			log("stopped processing thread");
			close(sock);
			return;
		}
		if (packets.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		log("New packet");
		log("Id: ", packets.begin()->id);
		log("Size: ", packets.begin()->size);
		log_header();
		std::cout << "Data: ";
		for (int i = 0; i < strlen(packets.begin()->data);i++)
		{
			if (isalnum(packets.begin()->data[i]))
				printf("%c ", packets.begin()->data[i]);
			else
				printf("%02hhX ", (int)packets.begin()->data[i]);
		}
		std::cout << "\n";
		status = execute_pkt(packets[0], status, user, index);
		log("New state: ", status);
		free(packets.begin()->data);
		packets.erase(packets.begin());
	}
}

int main()
{
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);

	if (std::filesystem::exists("server.properties") == false)
	{
		create_config();
		log("Config created!");
	}
	load_config();
    if (SV_PORT > 0xFFFF)
		log_err(std::format("Warning! The port provided in the config is higher than {} the port will be truncated into {}", 0xFFFF, (short)SV_PORT));
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
	log(std::format("Server listening to {}:{}", SV_IP, SV_PORT));
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
