#include "thread_manager.hpp"
#include "packet_processing.hpp"
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>
#include "config.hpp"
#include <filesystem>
#include "packet_send.hpp"

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

void config(int sock)
{
	std::string pack;
	std::string features = "minecraft:vanilla";

	pack.push_back(3 + features.length());
	pack.push_back(0x08);
	pack.push_back(0x01);
	write_string(pack, features);
	send(sock, pack.c_str(), pack.length(), 0);
	pack.clear();
	pack.push_back(0x01);
	pack.push_back(0x02);
	send(sock, pack.c_str(), pack.length(), 0);
}

void login_play(User user)
{
	struct packet pkt;
	std::string data;
	char buffer[10] = {0};
	char *ptr = buffer;
	std::string packet;
	long hashed_seed = 1212343;
	int buf = 0;
	unsigned long varint = 110;
	bool f = false;
	char b = 3;

	WriteUleb128(data, varint);
	send(user.get_socket(), data.c_str(), data.length(), 0);
	data.clear();
	varint = 0x29;
	WriteUleb128(data, varint);
	send(user.get_socket(), data.c_str(), data.length(), 0);
	send(user.get_socket(), &buf, sizeof(int), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	data.clear();
	WriteUleb128(data, 1);
	send(user.get_socket(), data.c_str(), data.length(), 0);
	data.clear();
	data.push_back(strlen("minecraft:overworld"));
	data.append("minecraft:overworld");
	send(user.get_socket(), data.c_str(), data.length(), 0);
	data.clear();
	WriteUleb128(data, 1);
	WriteUleb128(data, 1);
	WriteUleb128(data, 1);
	send(user.get_socket(), data.c_str(), data.length(), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	data.clear();
	data.push_back(strlen("minecraft:overworld"));
	data.append("minecraft:overworld");
	data.push_back(strlen("minecraft:overworld"));
	data.append("minecraft:overworld");
	send(user.get_socket(), data.c_str(), data.length(), 0);
	send(user.get_socket(), &hashed_seed, sizeof(long), 0);
	send(user.get_socket(), &b, sizeof(char), 0);
	buf = -1;
	send(user.get_socket(), &buf, sizeof(int), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	send(user.get_socket(), &f, sizeof(bool), 0);
	data.clear();
	WriteUleb128(data, 0);
	send(user.get_socket(), data.c_str(), data.length(), 0);
}

void set_spawn_pos(User user)
{
	packet pkt;
	std::string data;
	std::string packet;
	std::string a;
	long long x, y , z = 0;
	float angle = 0.0f;
	char buffer[33] = {0};
	char *ptr = buffer;

	pkt.id = 0x54;
	a = ((x & 0x3FFFFFF) << 38) | ((z & 0x3FFFFFF) << 12) | (y & 0xFFF);
	std::memcpy(ptr, &a, sizeof(a));
	data.append(ptr);
	memset(ptr, 0, 33);
	std::memcpy(ptr, &angle, sizeof(float));
	data.append(ptr);
	WriteUleb128(packet, data.size());
	packet.append(data);
	log(send(user.get_socket(), packet.c_str(), packet.length(), 0));
}

void sync_client(User user)
{
	packet pkt;
	std::string data;
	std::string packet;
	char buffer[10] = {0};
	char *ptr = buffer;
	double x, y, z = 0.0f;
	float yaw, pitch = 0.0f;

	pkt.id = 0x3E;
	std::memcpy(ptr, &x, sizeof(double));
	data.append(ptr);
	std::memcpy(ptr, &y, sizeof(double));
	data.append(ptr);
	std::memcpy(ptr, &z, sizeof(double));
	data.append(ptr);
	memset(ptr, 0, 8);
	std::memcpy(ptr, &yaw, sizeof(float));
	data.append(ptr);
	std::memcpy(ptr, &pitch, sizeof(float));
	data.append(ptr);
	data.push_back(0);
	data.push_back(0);
	WriteUleb128(packet, data.size());
	packet.append(data);
	log(send(user.get_socket(), packet.c_str(), packet.length(), 0));
}

void game_event(unsigned char event, float value, User user)
{
	packet pkt;
	std::string data;
	std::string packet;
	char *buffer[10] = {0};
	char *ptr;

	pkt.id = 0x20;
	data.push_back(event);
	std::memcpy(ptr, &value, sizeof(float));
	data.append(ptr);
	pkt.data = strdup(data.c_str());
	pkt.size = data.size();
	packet = forge_packet(pkt);
	send(user.get_socket(), packet.c_str(), packet.length(), 0);
	free(pkt.data);
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
				config(user.get_socket());
				ret = 5;
				/*buf.clear();
				buf.push_back((dc_msg.length() + 4));
				buf.push_back(0x01);
				buf.push_back(0x08);
				buf.push_back(0x00);
				write_string(buf, dc_msg);
				send(user.get_socket(), buf.c_str(), buf.length(), 0);
				manager.request_stop_thread(index);
				manager.request_stop_thread(index - 1);*/
			}
			break;
		case 2:
			if (state == 5)
			{
				/*
				buf.clear();
				buf.push_back((dc_msg.length() + 4));
				buf.push_back(0x1B);
				buf.push_back(0x08);
				buf.push_back(0x00);
				write_string(buf, dc_msg);
				send(user.get_socket(), buf.c_str(), buf.length(), 0);
				manager.request_stop_thread(index);
				manager.request_stop_thread(index - 1);*/
				alloc_and_send(LOGIN_PLAY, DEFAULT, user);
				//set_spawn_pos(user);
				//sync_client(user);
				//game_event(13, 0.0f, user);
				ret = 10;
			}
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
