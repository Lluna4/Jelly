#include "libs/thread_manager.hpp"
#include "libs/packet_processing.hpp"
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>
#include "libs/config.hpp"
#include <filesystem>
#include "libs/packet_send.hpp"
#include <sys/sendfile.h>
#include "libs/packet_send_rw.hpp"
#include <vector>
#include <typeinfo>
#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#endif

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

void config(int sock, User user)
{
	std::string pack;
	std::string features = "minecraft:vanilla";
	
	pkt_send({&typeid(minecraft::varint), &typeid(minecraft::string)}, {(minecraft::varint){.num = 0x01}, (minecraft::string){.len = features.length(), .string= features}}, user, 0x08);
	pack.push_back(0x01);
	pack.push_back(0x02);
	send(sock, pack.c_str(), pack.length(), 0);
}

void registry_data(User user)
{
	std::ifstream file("registry_info.packet", std::ios::binary);
	std::string pkt;
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.seekg(0, std::ios::end);
	std::size_t size = file.tellg();
	WriteUleb128(pkt, size + 1);
	pkt.push_back(0x05);
	pkt.append(buffer.str());
	send(user.get_socket(), pkt.c_str(), pkt.length(), 0);
}

void login_play(User user)
{
	struct packet pkt;
	std::string data;
	char buffer[10] = {0};
	char *ptr = buffer;
	std::string packet;
	long hashed_seed = 0;
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
				pkt_send(
					{
						&typeid(minecraft::uuid), 
						&typeid(minecraft::string),
						&typeid(minecraft::varint)
					}, 
					{
						(minecraft::uuid){.data = user.get_uuid().bytes()},
						(minecraft::string){.len = uname.length(), .string = uname},
						(minecraft::varint){.num = 0}
					},
					user,
					0x02);
				ret = 3;
			}
			else if (state == 4)
			{
				buf2 = read_string(p.data, buf);
				user.set_locale(buf);
				user.set_render_distance((int)p.data[buf2 + 1]);
				log("New locale: ", user.get_locale());
				log("New render distance: ", user.get_render_distance());
				registry_data(user);
				config(user.get_socket(), user);
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
				std::vector<const std::type_info*> types = {
					&typeid(int), &typeid(bool), &typeid(minecraft::varint), &typeid(minecraft::string), &typeid(minecraft::varint), 
					&typeid(minecraft::varint), &typeid(minecraft::varint), &typeid(bool), &typeid(bool), &typeid(bool),
					&typeid(minecraft::string), &typeid(minecraft::string), &typeid(long), &typeid(unsigned char), &typeid(char),
					&typeid(bool), &typeid(bool), &typeid(bool), &typeid(minecraft::varint)
				};
				std::vector<std::any> values = {
					0, false, (minecraft::varint){.num = 1}, (minecraft::string){.len = strlen("minecraft:overworld"), .string = "minecraft:overworld"},
					(minecraft::varint){.num = 20}, (minecraft::varint){.num = (unsigned long)user.get_render_distance()}, (minecraft::varint){.num = (unsigned long)user.get_render_distance()},
					false, true, false, (minecraft::string){.len = strlen("minecraft:overworld"), .string = "minecraft:overworld"},
					(minecraft::string){.len = strlen("minecraft:overworld"), .string = "minecraft:overworld"}, (long)123456, (unsigned char)3, 
					(char)-1, false, true, false, (minecraft::varint){.num = 0}
				};
				pkt_send(types, values, user, 0x29);
				pkt_send(
					{
						&typeid(long long),
						&typeid(float)
					},
					{
						(long long)(htobe64((long long)(((0 & 0x3FFFFFF) << 38) | ((0 & 0x3FFFFFF) << 12) | (0 & 0xFFF)))),
						(float)htobe32(0.0f)
					},
					user, 0x54
				);
				double x = 0;
				double y = 0;
				double z = 0;
				pkt_send(
					{
						&typeid(unsigned long), &typeid(unsigned long), &typeid(unsigned long), &typeid(float), &typeid(float), &typeid(char), &typeid(minecraft::varint)
					},
					{
						htobe64((*(uint64_t *)&x)), htobe64((*(uint64_t *)&y)), htobe64((*(uint64_t *)&z)), 0.0f, 0.0f, (char)0, (minecraft::varint){.num = 0}
					},
					user, 0x3E
				);
				pkt_send(
					{
						&typeid(unsigned char), &typeid(float)
					},
					{
						(unsigned char)13, 0.0f
					},
					user, 0x20
				);
				float eventf = 0;
				pkt_send(
					{
						&typeid(unsigned char), &typeid(unsigned int)
					},
					{
						(unsigned char)3, htobe32((*(uint32_t *)&eventf))
					},
					user, 0x20
				);
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

void manage_client(int sock)
{
	int rd_status = 0;
	int size = 0;
	int state = 0;
	bool closed = false;
	char *buffer = (char *)calloc(1025, sizeof(char));
    char *pkt = (char *)calloc(1025, sizeof(char));
	std::vector<packet> packets_to_process;
	User user;
	user.set_socket(sock);
	int index = manager.get_current_index();
	bool first = true;

    while (1)
    {
        do
        {
            rd_status = recv(sock, buffer, 1024, 0);
            if (rd_status == -1 || rd_status == 0)
            {
                close(sock);
                return;
            }
            memcpy(&pkt[size], buffer, rd_status);
            size += rd_status;
        }
        while (size < pkt[0]);
        packets_to_process = process_packet(pkt);
		memset(pkt, 0, 1025);
		size = 0;
        for (int i = 0; i < packets_to_process.size(); i++)
        {
			log("********************");
			log("New packet");
			log("Id: ", packets_to_process[i].id);
			log("Size: ", packets_to_process[i].size);
			log_header();
			std::cout << "Data: ";
			for (int x = 0; x < strlen(packets_to_process[i].data);x++)
			{
				if (isalnum(packets_to_process[i].data[x]))
					printf("%c ", packets_to_process[i].data[x]);
				else
					printf("%02hhX ", (int)packets_to_process[i].data[x]);
			}
			std::cout << "\n";
			state = execute_pkt(packets_to_process[i], state, user, index);
            free(packets_to_process[i].data);
        }
        packets_to_process.clear();
    }
}

int main()
{
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);

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
	listen(sock, 32);
	while (1)
	{
		int client_fd = 0;
		client_fd = accept(sock, nullptr, nullptr);
		if (client_fd < 0) 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}	
		std::thread sv_th(manage_client, client_fd);
		sv_th.detach();
	}
}
