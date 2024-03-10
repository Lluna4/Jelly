#include "libs/packet_processing.cpp"
#include <poll.h>
#include <fcntl.h>
#include <chrono>
#include <string>
#include "libs/config.hpp"
#include <filesystem>
#include <sys/sendfile.h>
#include "libs/packet_send.hpp"
#include <vector>
#include <typeinfo>
#include <nlohmann/json.hpp>
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

using json = nlohmann::json;
int connected = 0;


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

void status_response(User user)
{
	std::string response_str;
	json response ={
		{"version", {
			{"name", "1.20.4"},
			{"protocol", 765}
		}},
		{"players", {
			{"max", 20},
			{"online", connected}
		}},
		{"description",
		{
			{"text", motd}
		}}
	};
	response_str = response.dump();
	pkt_send(
		{
			&typeid(minecraft::string)
		},
		{
			(minecraft::string){.len = response_str.length(), .string = response_str}
		},
		user, 0x00);
}

int execute_pkt(packet p, int state, User &user)
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
			else if (state == 1)
			{
				status_response(user);
			}
			else if (state == 2)
			{
				ReadUleb128(p.data, &size);
				if (size < 1)
					log_err("Invalid username size!");
				uname = p.data;
				uname = uname.substr(1, size);
				user.set_uname(uname);
				connected++;
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
		case 1:
			if (state == 1)
			{
				send(user.get_socket(), (char *)(&p.size), sizeof(char), 0);
				send(user.get_socket(), (char *)(&p.id), sizeof(char), 0);
				send(user.get_socket(), p.data, sizeof(long), 0);
			}
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
				position pos = user.get_position();
				pkt_send(types, values, user, 0x29);
				pkt_send(
					{
						&typeid(long long),
						&typeid(float)
					},
					{
						(long long)(htobe64((long long)(((0 & 0x3FFFFFF) << 38) | ((0 & 0x3FFFFFF) << 12) | (0 & 0xFFF)))),
						0.0f
					},
					user, 0x54
				);
				pkt_send(
					{
						&typeid(double), &typeid(double), &typeid(double), &typeid(float), &typeid(float), &typeid(char), &typeid(minecraft::varint)
					},
					{
						pos.x, pos.y, pos.z, 0.0f, 0.0f, (char)0, (minecraft::varint){.num = 0}
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
						&typeid(unsigned char), &typeid(float)
					},
					{
						(unsigned char)3, eventf
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
	bool first = true;

    while (1)
    {
        do
        {
            rd_status = recv(sock, buffer, 1024, 0);
            if (rd_status == -1 || rd_status == 0)
            {
                close(sock);
				if (connected > 0)
					connected--;
				log("Thread closed");
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
			state = execute_pkt(packets_to_process[i], state, user);
            free(packets_to_process[i].data);
        }
        packets_to_process.clear();
    }
	close(sock);
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
