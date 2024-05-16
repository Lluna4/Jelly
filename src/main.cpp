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
#include <format>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <iterator>
#include <fstream>
#include "libs/test_epoll.hpp"
#include <sys/epoll.h>
#include "libs/world_gen.hpp"
#include <openssl/evp.h>
#include <bitset>
#include "libs/chunks.hpp"
#include <math.h>
#include <memory.h>
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
int epfd = 0;


std::vector<packet_def> next_tick_pkt;

std::unordered_map<int, User> users;
//minecraft::chunk empty_chunk;

void commands(User user)
{
	//only /pronoun command
	std::string pron = "pronouns";
	minecraft::node_root root_node;
	if (user.get_uname().compare("Alexandra") == 0 || user.get_uname().compare("carlyjb17") == 0)
		root_node = {.children_num = {.num = 3},
			.children_index = {(minecraft::varint){.num = 1}, (minecraft::varint){.num = 2}, (minecraft::varint){.num = 3}}};
	else
	{
		root_node = {.children_num = {.num = 2},
			.children_index = {(minecraft::varint){.num = 1}, (minecraft::varint){.num = 2}}};	
	}

	minecraft::node_literal pron_command = {.children_num = {.num = 1},
		.children_index = {(minecraft::varint){.num = 2}},
		.name = {.len = pron.length(), .string = pron}};

	minecraft::node_argument args = {.children_num = 0, .name {.len = pron.length(), .string = pron},
	.parser_id = {.num = 5}, .varies = {.num = 2} };
	
	if (user.get_uname().compare("Alexandra") == 0 || user.get_uname().compare("carlyjb17") == 0)
	{
		std::string florecilla = "florecilla";

		minecraft::node_literal pron_command2 = {.children_num = {.num = 0},
			.children_index = {},
			.name = {.len = florecilla.length(), .string = florecilla}};

		next_tick_pkt.emplace_back(packet_def(
				{
				&typeid(minecraft::varint), &typeid(minecraft::node_root), &typeid(minecraft::node_literal),
				&typeid(minecraft::node_argument), &typeid(minecraft::node_literal),&typeid(minecraft::varint)
			},
			{
				(minecraft::varint){.num = 4}, root_node, pron_command, args, pron_command2,(minecraft::varint){.num = 0}
			}, user, 0x11));
	}
	else
	{
		next_tick_pkt.emplace_back(packet_def(
				{
				&typeid(minecraft::varint), &typeid(minecraft::node_root), &typeid(minecraft::node_literal),
				&typeid(minecraft::node_argument), &typeid(minecraft::varint)
			},
			{
				(minecraft::varint){.num = 3}, root_node, pron_command, args, (minecraft::varint){.num = 0}
			}, user, 0x11));
	}
}

void send_tab()
{
	std::vector<const std::type_info *> types = {&typeid(char), &typeid(minecraft::varint)};
	std::vector<std::any> values = {(char)(0x01 | 0x08 | 0x20), (minecraft::varint){.num = (unsigned long)connected}};
	for (auto& value: users)
	{
		std::string formatted_name = std::format("{} [{}]", value.second.get_uname() ,value.second.get_pronouns());
		std::vector<const std::type_info *> buf = {&typeid(minecraft::uuid), &typeid(minecraft::string), &typeid(minecraft::varint), &typeid(bool),&typeid(bool), &typeid(minecraft::string_tag)};
		std::vector<std::any> val = {(minecraft::uuid)value.second.get_uuid(),(minecraft::string){.len = value.second.get_uname().length(), .string = value.second.get_uname()}, (minecraft::varint){.num = 0}, true, true,(minecraft::string_tag){.len = (short)formatted_name.length(), .string = formatted_name}};
		types.insert(types.end(), buf.begin(), buf.end());
		values.insert(values.end(), val.begin(), val.end());
	}
	for (auto& value: users)
	{
		next_tick_pkt.emplace_back(packet_def(types, values, value.second, 0x3C));
	}
}

void remove_tab(User user)
{
	for (auto& value: users)
	{
		next_tick_pkt.emplace_back(packet_def(
			{
				&typeid(minecraft::varint), &typeid(minecraft::uuid)
			},
			{
				(minecraft::varint){.num = 1}, (minecraft::uuid)user.get_uuid()
			}, value.second, 0x3B));
	}
}

void login_succ(User user)
{
	std::string pkt;
	WriteUleb128(pkt, user.get_uname().length() + 16 + 3);
	pkt.push_back(0x02);
	pkt.append(user.get_uuid().uuid);
	pkt.push_back(user.get_uname().length());
	pkt.append(user.get_uname());
	WriteUleb128(pkt, 0x00);
	send(user.get_socket(), pkt.c_str(), pkt.length(), 0);
	log_header();
	std::cout <<  user.get_uname().length() + 16 + 3 << 4 << user.get_uuid().uuid << user.get_uname().length() << user.get_uname() << 0 << std::endl;
}

void config(int sock, User user)
{
	std::string pack;
	std::string features = "minecraft:vanilla";
	std::string channel = "minecraft:brand";
	std::string sv_name = "Jelly";
	
	next_tick_pkt.emplace_back(packet_def({&typeid(minecraft::string), &typeid(minecraft::string)}, 
		{(minecraft::string){.len = channel.length(), .string = channel}, (minecraft::string){.len = sv_name.length(), .string = sv_name}},
		user, 0x00));
	next_tick_pkt.emplace_back(packet_def({&typeid(minecraft::varint), &typeid(minecraft::string)}, {(minecraft::varint){.num = 0x01}, (minecraft::string){.len = features.length(), .string= features}}, user, 0x08));
	next_tick_pkt.emplace_back(packet_def({}, {}, user, 0x02));
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
	json response;
	json fav;
	std::string base64 = std::format("data:image/png;base64,{}", base64_encode(icon_path));
	response ={
		{"version", {
			{"name", "1.20.4"},
			{"protocol", 765}
		}},
		{"players", {
			{"max", 20},
			{"online", connected}
		}},
		{"description", {
			{"text", motd}
		}},
		{"favicon", base64}
	};
	response_str = response.dump();
	std::cout << response_str << std::endl;
	next_tick_pkt.emplace_back(packet_def(
		{
			&typeid(minecraft::string)
		},
		{
			(minecraft::string){.len = response_str.length(), .string = response_str}
		},
		user, 0x00));
}

void send_chat(std::string contents, std::string sender)
{
	short lenght1 = (short)contents.length(), length2 = (short)sender.length();
	for (auto& value: users)
	{
		next_tick_pkt.emplace_back(packet_def({
			&typeid(minecraft::string_tag), &typeid(minecraft::varint),
			&typeid(minecraft::string_tag),
			&typeid(bool)
		},
		{
			(minecraft::string_tag){.len = lenght1, .string = contents},
			(minecraft::varint){.num = 0},
			(minecraft::string_tag){.len = length2, .string = sender},
			false
		}, value.second, 0x1C));
	}
}

void send_chat(std::string contents, std::string sender, User user)
{
	short lenght1 = (short)contents.length(), length2 = (short)sender.length();
	next_tick_pkt.emplace_back(packet_def({
			&typeid(minecraft::string_tag), &typeid(minecraft::varint),
			&typeid(minecraft::string_tag),
			&typeid(bool)
		},
		{
			(minecraft::string_tag){.len = lenght1, .string = contents},
			(minecraft::varint){.num = 0},
			(minecraft::string_tag){.len = length2, .string = sender},
			false
		}, user, 0x1C));
}

void system_chat(std::string contents)
{
	short lenght = contents.length();
	for (auto& value: users)
	{
		next_tick_pkt.emplace_back(packet_def(
			{
				&typeid(minecraft::string_tag), &typeid(bool)
			},
			{
				(minecraft::string_tag){.len = lenght, .string = contents}, false
			}, value.second, 0x69));
	}
}

void system_chat(std::string contents, User user)
{
	short lenght = contents.length();
	next_tick_pkt.emplace_back(packet_def(
		{
			&typeid(minecraft::string_tag), &typeid(bool)
		},
		{
			(minecraft::string_tag){.len = lenght, .string = contents}, false
		}, user, 0x69));
}

void update_list(User user)
{
	User found_user;
	auto found = users.find(user.get_socket());
	if (found != users.end())
		found->second = user;
	else
		return;

}

void set_center_chunk(User user, unsigned long x = 0, unsigned long z = 0)
{
	next_tick_pkt.emplace_back(packet_def(
		{
			&typeid(minecraft::varint), &typeid(minecraft::varint)
		},
		{
			(minecraft::varint){.num = x}, (minecraft::varint){.num = z}
		}, user, 0x52));
}

void send_chunk(User user, int x, int z)
{
	minecraft::chunk_rw chunk = find_chunk({.x = x, .z = z});

	minecraft::varint size = {.num = (unsigned long)chunk.size()};
	std::vector<const std::type_info *> types = {&typeid(int), &typeid(int), &typeid(char), &typeid(char), &typeid(minecraft::varint), 
		&typeid(minecraft::chunk_rw), &typeid(minecraft::varint), &typeid(minecraft::varint), &typeid(long),
		&typeid(minecraft::varint), &typeid(long),&typeid(minecraft::varint),
		&typeid(long),&typeid(minecraft::varint), &typeid(long), &typeid(minecraft::varint), &typeid(minecraft::varint)};
	std::vector<std::any> values = {x, z, (char)0x0a, (char)0x00, size, chunk,
	(minecraft::varint){.num = 0}, (minecraft::varint){.num = 1}, (long)0,(minecraft::varint){.num = 1}, (long)0,(minecraft::varint){.num = 1},
	(long)0, (minecraft::varint){.num = 1}, (long)0,(minecraft::varint){.num = 0}, (minecraft::varint){.num = 0}};
	next_tick_pkt.emplace_back(packet_def(types, values, user, 0x25));
}



void spawn_entities_to_user(User user)
{
	for (auto& value: users)
	{
		if (value.first == user.get_socket())
			continue;

		position pos = value.second.get_position();
		
		std::vector<const std::type_info *> types = {&typeid(minecraft::varint), &typeid(minecraft::uuid), &typeid(minecraft::varint), 
			&typeid(double), &typeid(double), &typeid(double), &typeid(char), &typeid(char), &typeid(char), &typeid(minecraft::varint), 
			&typeid(short), &typeid(short), &typeid(short)};
		
		std::vector<std::any> values = {(minecraft::varint){.num = value.first}, (minecraft::uuid)value.second.get_uuid(),
			(minecraft::varint){.num = 124}, pos.x, pos.y, pos.z, (char)(pos.pitch * 360.0 / 256.0), (char)(pos.yaw * 360.0 / 256.0),
			(char)(pos.yaw * 360.0 / 256.0), (minecraft::varint){.num = 0}, (short)0, (short)0, (short)0};
		
		next_tick_pkt.emplace_back(packet_def(types, values, user, 0x01));
	}
}

void spawn_user_to_users(User user)
{
	position pos = user.get_position();
	
	std::vector<const std::type_info *> types = {&typeid(minecraft::varint), &typeid(minecraft::uuid), &typeid(minecraft::varint), 
		&typeid(double), &typeid(double), &typeid(double), &typeid(char), &typeid(char), &typeid(char), &typeid(minecraft::varint), 
		&typeid(short), &typeid(short), &typeid(short)};
	
	std::vector<std::any> values = {(minecraft::varint){.num = user.get_socket()}, (minecraft::uuid)user.get_uuid(),
		(minecraft::varint){.num = 124}, pos.x, pos.y, pos.z, (char)(pos.pitch * 360.0 / 256.0), (char)(pos.yaw * 360.0 / 256.0),
		(char)(pos.yaw * 360.0 / 256.0), (minecraft::varint){.num = 0}, (short)0, (short)0, (short)0};
	
	for (auto& value: users)
	{
		if (value.first == user.get_socket() || value.second.get_state() != 10)
			continue;
		next_tick_pkt.emplace_back(packet_def(types, values, value.second, 0x01));
	}
}

void update_pos_to_users(User user, position pos, bool on_ground)
{
	position prev_pos = user.get_position();

	std::vector<const std::type_info *> types = {&typeid(minecraft::varint), &typeid(short), &typeid(short), &typeid(short), &typeid(bool)};

	std::vector<std::any> values = {(minecraft::varint){.num = (unsigned long)user.get_socket()}, (short)((pos.x * 32 - prev_pos.x * 32) * 128),
	(short)((pos.y * 32 - prev_pos.y * 32) * 128), (short)((pos.z * 32 - prev_pos.z * 32) * 128),on_ground};

	for (auto& value: users)
	{
		if (value.first == user.get_socket() || value.second.get_state() != 10)
			continue;
		next_tick_pkt.emplace_back(packet_def(types, values, value.second, 0x2C));
	}
}

void update_pos_rot_to_users(User user, position pos, bool on_ground)
{
	position prev_pos = user.get_position();

	std::vector<const std::type_info *> types = {&typeid(minecraft::varint), &typeid(short), &typeid(short), &typeid(short), 
	&typeid(char), &typeid(char),&typeid(bool)};

	std::vector<std::any> values = {(minecraft::varint){.num = (unsigned long)user.get_socket()}, (short)((pos.x * 32 - prev_pos.x * 32) * 128),
	(short)((pos.y * 32 - prev_pos.y * 32) * 128), (short)((pos.z * 32 - prev_pos.z * 32) * 128),  (char)(pos.yaw * 360.0 / 256.0), 
	(char)(pos.pitch * 360.0 / 256.0),on_ground};

	for (auto& value: users)
	{
		if (value.first == user.get_socket() || value.second.get_state() != 10)
			continue;
		next_tick_pkt.emplace_back(packet_def(types, values, value.second, 0x2C));
		next_tick_pkt.emplace_back(packet_def({&typeid(minecraft::varint), &typeid(char)}, 
		{(minecraft::varint){.num = (unsigned long)user.get_socket()}, (char)(pos.yaw * 360.0 / 256.0)}, value.second, 0x46));
	}
	
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
				if (std::filesystem::exists(uname))
				{
					user.from_file(uname);
					log("Read from file!");
				}
				user.set_uname(uname);
				minecraft::uuid uuid_new;
				uuid_new.generate(uname);
				user.set_uuid(uuid_new);
				connected++;
				next_tick_pkt.emplace_back(packet_def(
					{
						&typeid(minecraft::uuid),
						&typeid(minecraft::string),
						&typeid(minecraft::varint)
					},
					{
						(minecraft::uuid)user.get_uuid(),
						(minecraft::string){.len = uname.length(), .string = uname},
						(minecraft::varint){.num = 0}
					},
					user,
					0x02));
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
				close(user.get_socket());
				remove_from_list(user.get_socket(), epfd);
				users.erase(user.get_socket());
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
					(minecraft::varint){.num = 20}, (minecraft::varint){.num = (unsigned long)32}, (minecraft::varint){.num = (unsigned long)32},
					false, true, false, (minecraft::string){.len = strlen("minecraft:overworld"), .string = "minecraft:overworld"},
					(minecraft::string){.len = strlen("minecraft:overworld"), .string = "minecraft:overworld"}, (long)123456, (unsigned char)3,
					(char)-1, false, true, false, (minecraft::varint){.num = 0}
				};
				next_tick_pkt.emplace_back(packet_def(types, values, user, 0x29));
				position pos = user.get_position();
				next_tick_pkt.emplace_back(packet_def(
					{
						&typeid(long long),
						&typeid(float)
					},
					{
						(long long)(htobe64((long long)(((0 & 0x3FFFFFF) << 38) | ((0 & 0x3FFFFFF) << 12) | (0 & 0xFFF)))),
						0.0f
					},
					user, 0x54
				));
				next_tick_pkt.emplace_back(packet_def(
					{
						&typeid(double), &typeid(double), &typeid(double), &typeid(float), &typeid(float), &typeid(char), &typeid(minecraft::varint)
					},
					{
						pos.x, pos.y, pos.z, pos.yaw, pos.pitch, (char)0, (minecraft::varint){.num = 0}
					},
					user, 0x3E
				));
				next_tick_pkt.emplace_back(packet_def(
					{
						&typeid(unsigned char), &typeid(float)
					},
					{
						(unsigned char)13, 0.0f
					},
					user, 0x20
				));
				float eventf = 1;
				size = size - 2;
				next_tick_pkt.emplace_back(packet_def(
					{
						&typeid(unsigned char), &typeid(float)
					},
					{
						(unsigned char)3, eventf
					},
					user, 0x20
				));
				set_center_chunk(user);
				for (int x = -3; x < 3; x++)
				{
					for (int z = -3; z <= 3; z++)
					{
						send_chunk(user, x, z);
					}
				}
				send_tab();
				system_chat(std::format("§e{} connected", user.get_uname()));
				commands(user);
				spawn_entities_to_user(user);
				spawn_user_to_users(user);
				//game_event(13, 0.0f, user);
				ret = 10;
			}
			break;
		case 3:
			if (state == 3)
			{
				log("login awknoleged by the client");
				ret = 4;
			}
			break;
		case 0x04:
			if (state == 10)
			{
				std::string command = read_string(p.data);
				std::string command_contents;
				log("Command: ", command);
				if (command.starts_with("pronouns"))
				{
					command_contents = command.substr(command.find(' ') + 1);
					user.set_pronouns(command_contents);
					update_list(user);
					send_tab();
					if (user.get_locale() == "en_us")
						system_chat(std::format("Changed pronouns to {}", command_contents), user);
					else if(user.get_locale() == "es_es")
						system_chat(std::format("Se cambiaron los pronombres a {}", command_contents), user);
				}
				else if (command.starts_with("florecilla"))
				{
					system_chat("Si puedes ver esto es que eres una florecilla y eres una persona muy muy importante para mi, muchisimas gracias por todo! Gracias a ti soy muy feliz", user);
				}
			}
			break;
		case 0x05:
			if (state == 10)
			{
				std::string message = read_string(p.data);
				std::string sender = std::format("{} [{}]", user.get_uname(), user.get_pronouns());
				send_chat(message, sender);
			}
			break;
		case 0x0A:
			if (state == 10)
			{
				minecraft::varint id = minecraft::read_varint(p.data);
				p.data++;
				std::string command = read_string(p.data);
				if (std::string("pronouns").starts_with(command))
				{
					next_tick_pkt.emplace_back(packet_def(
						{
						&typeid(minecraft::varint), &typeid(minecraft::varint),
						&typeid(minecraft::varint), &typeid(minecraft::varint),
						&typeid(minecraft::string), &typeid(bool)
						},
						{
						id, (minecraft::varint){.num = 0},
						(minecraft::varint){.num = std::string("pronouns").length()},
						(minecraft::varint){.num = 1},
						(minecraft::string){.len = std::string("pronouns").length(),
						.string = std::string("pronouns")}, false

							}, user, 0x10
					));
				}
			}
		case 0x17:
			if (state == 10)
			{
				char *ptr = p.data;
				bool on_ground = true;
				position pos = user.get_position();
				pos.x = read_double(ptr);
				ptr = ptr + sizeof(double);
				pos.y = read_double(ptr);
				ptr = ptr + sizeof(double);
				pos.z = read_double(ptr);
				ptr = ptr + sizeof(double);
				on_ground = *ptr;
				pos.yaw = pos.yaw;
				pos.pitch = pos.pitch;
				update_pos_to_users(user, pos, on_ground);
				user.update_position(pos);
				log_header();
				std::cout << "New pos: x: " << pos.x << " y: " << pos.y << " z: " << pos.z << " yaw: " << pos.yaw << " pitch: " << pos.pitch << std::endl;
				//next_tick_pkt.emplace_back(packet_def({&typeid(long)},{(long)0},user, 0x24);
				
				/*if (pos.y <= 500.0f)
				{
					next_tick_pkt.emplace_back(packet_def(
						{
							&typeid(double), &typeid(double), &typeid(double), &typeid(float), &typeid(float), &typeid(char), &typeid(minecraft::varint)
						},
						{
							pos.x, (double)1000.0f, pos.z, pos.yaw, pos.pitch, (char)0, (minecraft::varint){.num = 0}
						},
						user, 0x3E
					);
				}*/
			}
			break;
		case 0x18:
			if (state == 10)
			{
				char *ptr = p.data;
				position pos = user.get_position();
				bool on_ground = true;
				pos.x = read_double(ptr);
				ptr = ptr + sizeof(double);
				pos.y = read_double(ptr);
				ptr = ptr + sizeof(double);
				pos.z = read_double(ptr);
				ptr = ptr + sizeof(double);
				pos.yaw = read_float(ptr);
				ptr = ptr + sizeof(float);
				pos.pitch = read_float(ptr);
				ptr = ptr + sizeof(double);
				on_ground = *ptr;
				update_pos_to_users(user, pos, on_ground);
				user.update_position(pos);
				log_header();
				std::cout << "New pos2: x: " << pos.x << " y: " << pos.y << " z: " << pos.z << " yaw: " << pos.yaw << " pitch: " << pos.pitch << std::endl;
				//next_tick_pkt.emplace_back(packet_def({&typeid(long)},{(long)0},user, 0x24);
				/*if (pos.y <= 500.0f)
				{
					next_tick_pkt.emplace_back(packet_def(
						{
							&typeid(double), &typeid(double), &typeid(double), &typeid(float), &typeid(float), &typeid(char), &typeid(minecraft::varint)
						},
						{
							pos.x, (double)1000.0f, pos.z, pos.yaw, pos.pitch, (char)0, (minecraft::varint){.num = 0}
						},
						user, 0x3E
					);
				}*/
			}
			break;
		case 0x21:
			if (state == 10)
			{
				char *ptr = p.data;
				minecraft::varint status = minecraft::read_varint(ptr);
				ptr++;
				if (status.num == 0)
				{
					unsigned long val = read_position(ptr);
					ptr += sizeof(long);
					minecraft::varint face = minecraft::read_varint(ptr);
					std::int32_t orig_x,orig_y, orig_z;

					std::int32_t x = val >> 38;
					std::int32_t y = val << 52 >> 52;
					std::int32_t z = val << 26 >> 38;
					if (face.num == 0)
						y--;
					else if (face.num == 1)
						y++;
					else if (face.num == 2)
						z--;
					else if (face.num == 3)
						z++;
					else if (face.num == 4)
						x--;
					else if (face.num == 5)
						x++;				
					orig_x = x;
					orig_z = z;
					orig_y = y;
					position user_pos = user.get_position();
					if (user.get_sneaking() == true)
					{
						if (abs(orig_x - nearbyint(user_pos.x)) < 2 && orig_y == floor(user_pos.y)&& abs(orig_z - nearbyint(user_pos.z)) < 2)
							break;
					}
					else
					{
						if (orig_x == floor(user_pos.x) && orig_y == floor(user_pos.y)&& orig_z == floor(user_pos.z))
							break;
					}
					x = rem_euclid(x, 16);
					y = floor((((y)+ 64)/16));
					int y_chunk_y = rem_euclid(orig_y, 16);
					z = rem_euclid(z, 16);
					
					minecraft::chunk_rw chun = find_chunk({.x = (orig_x >> 4), .z = (orig_z >> 4)});
					std::vector<std::bitset<4>> chunk = chun.chunks[y].blocks.block_indexes_nums;
					std::bitset<4> new_block(0x0);
					chunk[(((y_chunk_y))*256) + (abs(z)*16) + (15 - abs(x))] = new_block;
					std::vector<unsigned long> longs;
					std::string buf;
					for (int i = 0; i < chunk.size(); i += 16)
					{
						for (int x = i; x < (i + 16); x++)
						{
								buf = buf + chunk[x].to_string();
						}
						longs.push_back(std::bitset<64>(buf).to_ulong());
						buf.clear();
					}

					chun.chunks[y].blocks.block_indexes = longs;	
					chun.chunks[y].block_count++;
					chun.chunks[y].blocks.block_indexes_nums = chunk;
					auto c = chunks_r.find({.x = (orig_x >> 4), .z = (orig_z >> 4)});
					c->second = chun;
					for (auto& value: users)
					{
						send_chunk(value.second, (orig_x >> 4), (orig_z >> 4));
					}
				}

			}
			break;
		case 0x22:
			if (state == 10)
			{
				char *ptr = p.data;
				unsigned long buf = 0;

				ptr += ReadUleb128(ptr, &buf);

				minecraft::varint action = minecraft::read_varint(ptr);
				switch (action.num)
				{
					case 0:
						user.set_sneaking(true);
						break;
					case 1:
						user.set_sneaking(false);
				};
			}
		case 0x35:
			if (state == 10)
			{
				char *ptr = p.data;
				ptr++;
				long val = read_position(ptr);
				ptr += sizeof(long);
				minecraft::varint face = minecraft::read_varint(ptr);
				std::int32_t orig_x,orig_y, orig_z;

				std::int32_t x = val >> 38;
				std::int32_t y = val << 52 >> 52;
				std::int32_t z = val << 26 >> 38;
				if (face.num == 0)
					y--;
				else if (face.num == 1)
					y++;
				else if (face.num == 2)
					z--;
				else if (face.num == 3)
					z++;
				else if (face.num == 4)
					x--;
				else if (face.num == 5)
					x++;				
				orig_x = x;
				orig_z = z;
				orig_y = y;
				position user_pos = user.get_position();
				if (user.get_sneaking() == true)
				{
					if (abs(orig_x - nearbyint(user_pos.x)) < 2 && orig_y == floor(user_pos.y)&& abs(orig_z - nearbyint(user_pos.z)) < 2)
						break;
				}
				else
				{
					if (orig_x == floor(user_pos.x) && orig_y == floor(user_pos.y)&& orig_z == floor(user_pos.z))
						break;
				}
				x = rem_euclid(x, 16);
				y = floor((((y)+ 64)/16));
				int y_chunk_y = rem_euclid(orig_y, 16);
				z = rem_euclid(z, 16);

				minecraft::chunk_rw chun = find_chunk({.x = (orig_x >> 4), .z = (orig_z >> 4)});
				std::vector<std::bitset<4>> chunk = chun.chunks[y].blocks.block_indexes_nums;
				std::bitset<4> new_block(0x3);
				chunk[(((y_chunk_y))*256) + (abs(z)*16) + (15 - abs(x))] = new_block;
				std::vector<unsigned long> longs;
    				std::string buf;
				for (int i = 0; i < chunk.size(); i += 16)
				{
					for (int x = i; x < (i + 16); x++)
					{
							buf = buf + chunk[x].to_string();
					}
					longs.push_back(std::bitset<64>(buf).to_ulong());
					buf.clear();
				}

				chun.chunks[y].blocks.block_indexes = longs;	
				chun.chunks[y].block_count++;
				chun.chunks[y].blocks.block_indexes_nums = chunk;
				auto c = chunks_r.find({.x = (orig_x >> 4), .z = (orig_z >> 4)});
				c->second = chun;
				for (auto& value: users)
				{
					send_chunk(value.second, (orig_x >> 4), (orig_z >> 4));
				}
				/*for (int xx = -17; xx < 17; xx++)
				{
					for (int zz = -17; zz <= 17; zz++)
					{
						if (abs(xx) == 17 || abs(zz) == 17)
							send_empty_chunk(user, xx, zz);
						else
							send_mock_chunk(user, xx, zz);
					}
				}*/
			}
			break;
	}
	return ret;
}

void keep_alive_event()
{
	while (true)
	{
		for (auto& value: users)
		{
			if (value.second.get_state() == 10)
			{
				next_tick_pkt.emplace_back(packet_def({&typeid(long)}, {(long)0}, value.second, 0x24));
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

User read_ev(char *pkt, int sock, User user)
{
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;
	std::vector<packet> packets_to_process;
	auto t1 = high_resolution_clock::now();
	packets_to_process = process_packet(pkt);
	int state = user.get_state();
	
	log("State", state);
	log("Size: ", users.size());
	for (int i = 0; i < packets_to_process.size(); i++)
	{
		log("********************");
		log("New packet");
		log("Id: ", packets_to_process[i].id);
		log("Size: ", packets_to_process[i].size);
		log_header();
		std::cout << "Data: ";
		for (int x = 0; x < packets_to_process[i].size; x++)
		{
			if (isalnum(packets_to_process[i].data[x]))
				printf("%c ", packets_to_process[i].data[x]);
			else
				printf("%02hhX ", (int)packets_to_process[i].data[x]);
		}
		std::cout << "\n";
		state = execute_pkt(packets_to_process[i], state, user, 0);
		log("state", state);
		user.set_state(state);
		free(packets_to_process[i].data);
	}
	auto t2 = high_resolution_clock::now();
	auto ms_int = duration_cast<milliseconds>(t2 - t1);

	duration<double, std::milli> ms_double = t2 - t1;
	log_header();
	std::cout << "Time to process: ";
	std::cout << ms_double.count() << "ms\n";
	return user;
}

std::vector<pkt> read_from_epfd(int epfd)
{
    struct epoll_event events[MAX_EVENTS];
	int events_ready = 0;
	int status = 0;
	char *buff = (char *)calloc(1025, sizeof(char));
	std::vector<pkt> ret;

    events_ready = epoll_wait(epfd, events, MAX_EVENTS, -1);

	for (int i = 0; i < events_ready; i++)
	{
		status = recv(events[i].data.fd, buff, 1024, 0);
		if (status == -1 || status == 0)
		{
			close(events[i].data.fd);
			remove_from_list(events[i].data.fd, epfd);
			User removed_user;
			auto user = users.find(events[i].data.fd);
			if (user != users.end())
				removed_user = user->second;
			else
				continue;
			users.erase(user->first);
			connected--;
			removed_user.to_file();
			std::string user_name = removed_user.get_uname();
			remove_tab(removed_user);
			system_chat(std::format("§e{} disconnected", user_name));
		}
		else
		{
			struct pkt new_pkt = {0};
			new_pkt.data = mem_dup(buff, 1024);
			new_pkt.fd = events[i].data.fd;
			ret.emplace_back(new_pkt);
		}
		memset(buff, 0, 1024);
	}
	return ret;
}

void proc_pkt(std::vector<pkt> packets)
{
	for (auto pack: packets)
	{
		User found_user;
		auto found = users.find(pack.fd);
		if (found != users.end())
			found_user = found->second;
		else
			break;
		found->second = read_ev(pack.data, pack.fd, found_user);
	}
}

void game_loop(int epfd)
{
	std::vector<pkt> packets;
	while (true)
	{
		packets = read_from_epfd(epfd);
		proc_pkt(packets);
		for (auto &pkt: next_tick_pkt)
		{
			pkt_send(pkt.types, pkt.values, pkt.user_, pkt.packet_id_);
			log("Sent packet id: ", (int)pkt.packet_id_);
		}
		next_tick_pkt.clear();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

/*void read_loop(int epfd)
{
	struct epoll_event events[MAX_EVENTS];
	int events_ready = 0;
	char buff[1025] = {0};
	int rd_status = 0;
	while (true)
	{
		for (auto &pkt: next_tick_pkt)
		{
			pkt_send(pkt.types, pkt.values, pkt.user_, pkt.packet_id_);
			log("Sent packet id: ", (int)pkt.packet_id_);
		}
		next_tick_pkt.clear();
		events_ready = epoll_wait(epfd, events, MAX_EVENTS, 49);
		for (int i = 0; i < events_ready; i++)
		{

			rd_status = recv(events[i].data.fd, buff, 1024, 0);
			if (rd_status == -1 || rd_status == 0)
			{
				close(events[i].data.fd);
				remove_from_list(events[i].data.fd, epfd);
				User found_user;
				auto found = users.find(events[i].data.fd);
				if (found != users.end())
					found_user = found->second;
				else
					break;
				std::string disc_name;
				found_user.to_file();
				log("User ", found_user.get_uname(), " saved to file!");
				disc_name = found_user.get_uname();
				users.erase(found->first);
				
				connected--;
				system_chat(std::format("§e{} disconnected", disc_name));
				remove_tab(found_user);
			}
			User user;
			int index = 0;
			auto found = users.find(events[i].data.fd);
			if (found != users.end())
				user = found->second;
			else
				break;
			found->second = read_ev(buff, events[i].data.fd, user);
			memset(buff, 0, 1024);
		}
	}
}*/

int main()
{
	int epfd = create_instance();
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
	log("Generating world...");
	for (int x = -3; x < 3; x++)
	{
		for (int z = -3; z <= 3; z++)
		{
			find_chunk({.x = x, .z = z});
		}
	}
	log("World generated!");
	std::thread read_l(game_loop, epfd);
	read_l.detach();
	std::thread keep_alive(keep_alive_event);
	keep_alive.detach();
	
	while (1)
	{
		int client_fd = 0;
		client_fd = accept(sock, nullptr, nullptr);
		users.emplace(client_fd, User(" ", client_fd));
		add_to_list(client_fd, epfd);
	}
}
