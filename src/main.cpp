#include "libs/netlib.h"
#include "libs/utils.hpp"
#include <vector>
#include <map>
#include <thread>
#include "libs/packet_processing.hpp"
#include <chrono>
#include <errno.h>
#include <nlohmann/json.hpp>
#include <sys/sendfile.h>
#include <math.h>
#include "libs/comp_time_write.hpp"
#include "libs/comp_time_read.hpp"
#include "libs/world_gen.hpp"

int epfd;
std::vector<packet> tick_packets;
using login_play = std::tuple<
                minecraft::varint, int, bool, minecraft::varint, minecraft::string, minecraft::varint,
                minecraft::varint, minecraft::varint, bool, bool, bool,
                minecraft::varint, minecraft::string, long,
                unsigned char, char, bool, bool, bool, minecraft::varint, bool>;

using chunk_data_light = std::tuple<minecraft::varint, int, int, char, char, minecraft::varint, minecraft::chunk_rw,
                                    minecraft::varint, char, char, char, char, minecraft::varint,
                                    minecraft::varint>;

using json = nlohmann::json;

enum states
{
    HANDSHAKE,
    STATUS,
    LOGIN,
    CONFIGURATION,
    PLAY
};

struct position
{
    double x, y, z;
    
};

struct angles
{
    float yaw, pitch;
};

class User
{
    public:
        User(int sock)
        :sockfd(sock)
        {
            state = HANDSHAKE;
            pronouns = "they/them";
            name = "placeholder name";
            uuid.generate(name);
            locale = "en_US";
            render_distance = 8;
            pos = {2.0f, 64.0f, 2.0f};
            prev_pos = {2.0f, 64.0f, 2.0f};
            angle = {0.0f, 0.0f};
            on_ground = true;
            sneaking = false;
        }
        
        int state;
        int render_distance;
        minecraft::uuid uuid;
        std::string name;
        std::string pronouns;
        std::string locale;
        position pos;
        position prev_pos;
        angles angle;
        bool on_ground;
        bool sneaking;
        int sockfd;
};
std::map<int, User> users;

void accept_th(int sock)
{
    sockaddr_in addr = {0};
    unsigned int addr_size = sizeof(addr);
    char str[INET_ADDRSTRLEN];
    while (true)
    {
        int new_client = accept(sock, (sockaddr *)&addr, &addr_size);
        netlib::add_to_list(new_client, epfd);
        struct in_addr ipAddr = addr.sin_addr;
        std::println("{} connected", inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN));
        users.emplace(new_client, User(new_client));
    }
}

void status_response(User user)
{
	std::string response_str;
	json response;
	json fav;
	std::string base64 = std::format("data:image/png;base64,{}", base64_encode("a.png"));
	response ={
		{"version", {
			{"name", "1.21.1"},
			{"protocol", 767}
		}},
		{"players", {
			{"max", 20},
			{"online", users.size()}
		}},
		{"description", {
			{"text", "Hiiiii!"}
		}}
	};
	response_str = response.dump();
	std::cout << response_str << std::endl;
    std::string dummy;
    size_t size = WriteUleb128(dummy, response_str.length());
    minecraft::string str(response_str);
    std::tuple<minecraft::varint, minecraft::string> status_pkt = {minecraft::varint(0), str};
    send_packet(status_pkt, user.sockfd);
}

void registry_data(User user)
{
    int fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/banner_pattern.data", O_RDONLY);
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);
    
    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/chat_type.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);
    
    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/damage_type.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);
    
    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/dimension_type.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);

    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/painting_variant.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);

    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/trim_material.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);

    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/trim_pattern.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);

    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/wolf_variant.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);

    fd = open("../Minecraft-DataRegistry-Packet-Generator/registries/1.21-registry/created-packets/worldgen/biome.data", O_RDONLY);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(user.sockfd, fd, 0, file_size);
    close(fd);
}



void update_players_position(User user)
{
    for (auto us: users)
    {
        if (us.second.sockfd != user.sockfd && us.second.state == PLAY && user.state == PLAY)
        {
            std::tuple<minecraft::varint, minecraft::varint, double, double, double, char, char, bool> update_pos =
            {
                minecraft::varint(0x70), minecraft::varint(us.second.sockfd),
                us.second.pos.x,
                us.second.pos.y,
                us.second.pos.z,
                (char)((us.second.angle.yaw/ 360) * 256), (char)((us.second.angle.pitch/ 360) * 256),
                us.second.on_ground
            };
            send_packet(update_pos, user.sockfd);
            std::tuple<minecraft::varint, minecraft::varint, char> head_angle =
            {
                minecraft::varint(0x48), minecraft::varint(us.second.sockfd), (char)((us.second.angle.yaw/ 360) * 256)
            };
            send_packet(head_angle, user.sockfd);
        }
    }
}

template<typename ...T>
void send_everyone(std::tuple<T...> packet)
{
    for (auto us: users)
    {
        if (us.second.state == PLAY)
        {
            send_packet(packet, us.second.sockfd);
        }
    }
}

template<typename ...T>
void send_everyone_except_user(std::tuple<T...> packet, int id)
{
    for (auto us: users)
    {
        if (us.second.state == PLAY && us.second.sockfd != id)
        {
            send_packet(packet, us.second.sockfd);
        }
    }
}

void send_chat(std::string message, std::string name)
{
    std::tuple<minecraft::varint, minecraft::string_tag, minecraft::varint, minecraft::string_tag, bool> chat_message =
    {
        minecraft::varint(0x1E), (minecraft::string_tag){.len = (short)message.length(), .string = message},
        minecraft::varint(1), (minecraft::string_tag){.len = (short)name.length(), .string = name}, false,
    };
    send_everyone(chat_message);
}

bool check_collision(position player, position block)
{
   if (player.x + 0.25 < block.x || player.x - 0.25 > block.x + 1) return false;
    
   if (player.y + 1.8 < block.y || player.y > block.y + 1) return false;
    
   if (player.z + 0.25 < block.z || player.z - 0.25 > block.z + 1) return false;
    
    return true;
}

void execute_packet(packet pkt, User &user)
{
    if (user.state == HANDSHAKE)
    {
        switch (pkt.id)
        {
            case 0:
                std::tuple<minecraft::varint, minecraft::string, unsigned short, minecraft::varint> handshake;
                handshake = read_packet(handshake, pkt.data);
                log(std::format("Got a handshake packet, version: {}, address {}, port {}, state {}", 
                std::get<0>(handshake).num, std::get<1>(handshake).str, std::get<2>(handshake), std::get<3>(handshake).num));
                user.state = std::get<3>(handshake).num;
                break;
        }
        return;
    }
    if (user.state == STATUS)
    {
        if (pkt.id == 0x0)
        {
            status_response(user);
        }
        else if (pkt.id == 0x1)
        {

            std::tuple<long> ping;
            ping = read_packet(ping, pkt.data);
            
            std::tuple<minecraft::varint, long> ping_repl = {minecraft::varint(1), std::get<0>(ping)};
            send_packet(ping_repl, user.sockfd);
        }
    }
    if (user.state == LOGIN)
    {
        if (pkt.id == 0x0)
        {
            std::tuple<minecraft::string> login_start;
            login_start = read_packet(login_start, pkt.data);
            user.name = std::get<0>(login_start).str;
            user.uuid.generate(user.name);

            std::tuple<minecraft::varint, minecraft::uuid, minecraft::string, minecraft::varint, unsigned char> login_succ =
                {
                    minecraft::varint(0x02), user.uuid, minecraft::string(user.name), 
                    minecraft::varint(0x00),(unsigned char)true
                };
            send_packet(login_succ, user.sockfd);
        }
        if (pkt.id == 0x3)
        {
            log("login awknowledged");
            user.state = CONFIGURATION;
            return;
        }
    }
    if (user.state == CONFIGURATION)
    {
        if (pkt.id == 0x0)
        {
            std::tuple<minecraft::string, char, minecraft::varint, 
                       bool, unsigned char, minecraft::varint, bool, bool> client_information;
            client_information = read_packet(client_information, pkt.data);
            log(std::format("Locale {}, render distance {}, chat mode {}", std::get<0>(client_information).str, 
            (int)std::get<1>(client_information), std::get<2>(client_information).num));
            user.locale = std::get<0>(client_information).str;
            user.render_distance = (int)std::get<1>(client_information);

            std::tuple<minecraft::varint, minecraft::varint, minecraft::string, minecraft::string, minecraft::string> known_packs = {
                minecraft::varint(0x0E), minecraft::varint(1),
                minecraft::string("minecraft"), minecraft::string("core"), minecraft::string("1.21")
            };
            send_packet(known_packs, user.sockfd);
            registry_data(user);
            std::tuple<minecraft::varint> end_config = {minecraft::varint(0x03)};
            send_packet(end_config, user.sockfd);
        }
        if (pkt.id == 0x03)
        {
            user.state = PLAY;
            login_play login = 
            {
                minecraft::varint(0x2B), user.sockfd, false, minecraft::varint(1), 
                minecraft::string("minecraft:overworld"), minecraft::varint(20),
                minecraft::varint(user.render_distance), minecraft::varint(8),
                false, true, false, minecraft::varint(0),
                minecraft::string("minecraft:overworld"), 123456, 1, -1, false,
                false, false, minecraft::varint(0), false
            };
            send_packet(login, user.sockfd);

            std::tuple<minecraft::varint, double, double, double, float, float, char, minecraft::varint> sync_pos =
            {
                minecraft::varint(0x40), user.pos.x, user.pos.y, user.pos.z, user.angle.yaw, user.angle.pitch, 0, minecraft::varint(0x0)
            };
            send_packet(sync_pos, user.sockfd);

            std::string user_name = std::format("{} [{}]", user.name, user.pronouns);
            std::tuple<minecraft::varint, char, minecraft::varint, minecraft::uuid,
                    minecraft::string, minecraft::varint, bool ,bool, minecraft::string_tag> info_update_head_user =
                    {
                        minecraft::varint(0x3E), 0x01 |0x08 | 0x20, minecraft::varint(1), user.uuid, 
                        minecraft::string(user.name), minecraft::varint(0), true, true,
                        minecraft::string_tag{.len = (short)user_name.length(), .string = user_name}
                    };
            std::tuple<minecraft::varint, minecraft::varint, minecraft::uuid, minecraft::varint, double,
            double, double, char, char, char, minecraft::varint,
            short, short, short> 
            spawn_entity_user =
            {
                minecraft::varint(0x01), minecraft::varint(user.sockfd), user.uuid, minecraft::varint(122 + 6),
                user.pos.x, user.pos.y, user.pos.z, 
                (user.angle.pitch/ 360) * 256, (user.angle.yaw/ 360) * 256,
                (user.angle.yaw/ 360) * 256, minecraft::varint(0),0,0,0
            };
            for (auto us: users)
            {
                if (us.second.state == PLAY)
                {
                    std::string name = std::format("{} [{}]", us.second.name, us.second.pronouns);
                    std::tuple<minecraft::varint, char, minecraft::varint, minecraft::uuid,
                            minecraft::string, minecraft::varint, bool ,bool, minecraft::string_tag> info_update_head =
                            {
                                minecraft::varint(0x3E), 0x01 |0x08 | 0x20, minecraft::varint(1), us.second.uuid, 
                                minecraft::string(us.second.name), minecraft::varint(0), true, true,
                                minecraft::string_tag{.len = (short)name.length(), .string = name}
                            };
                    send_packet(info_update_head, user.sockfd);
                    if (user.sockfd != us.second.sockfd)
                    {
                        send_packet(info_update_head_user, us.second.sockfd);
                        
                        std::tuple<minecraft::varint, minecraft::varint, minecraft::uuid, minecraft::varint, double,
                                    double, double, char, char, char, minecraft::varint,
                                    short, short, short> 
                        spawn_entity =
                        {
                            minecraft::varint(0x01), minecraft::varint(us.second.sockfd), us.second.uuid, minecraft::varint(122 + 6),
                            us.second.pos.x, us.second.pos.y, us.second.pos.z, 
                           (char)((us.second.angle.yaw/ 360) * 256), (char)((us.second.angle.pitch/ 360) * 256),
                           (us.second.angle.yaw/ 360) * 256, minecraft::varint(0),0,0,0
                        };
                        send_packet(spawn_entity, user.sockfd);
                        send_packet(spawn_entity_user, us.second.sockfd);
                    }
                }
            }

            std::tuple<minecraft::varint, unsigned char, float> game_event = 
            {
                minecraft::varint(0x22), 13, 0.0f
            };
            send_packet(game_event, user.sockfd);
            
            minecraft::chunk_rw chunk = find_chunk({.x = 0, .z = 0});
            chunk_data_light chunk_data = {
                    minecraft::varint(0x27), 0, 0, 0x0a, 0x0, minecraft::varint(chunk.size()), chunk, 
                    minecraft::varint(0),0, 0, 0, 0, minecraft::varint(0), minecraft::varint(0)
                };
            send_packet(chunk_data, user.sockfd);
        }
    }
    if (user.state == PLAY)
    {
        if (pkt.id == 0x06)
        {
            std::tuple<minecraft::string> message;
            message = read_packet(message, pkt.data);
            std::string name = std::format("{} [{}]", user.name, user.pronouns);
            send_chat(std::get<0>(message).str, name);
        }
        else if (pkt.id == 0x1A)
        {
            std::tuple<double, double, double, bool> position_set;
            position_set = read_packet(position_set, pkt.data);
            user.pos.x = std::get<0>(position_set);
            user.pos.y = std::get<1>(position_set);
            user.pos.z = std::get<2>(position_set);
            user.on_ground = std::get<3>(position_set);
            log(std::format("Moved to x {} y {} z {} on_ground {} (1)", user.pos.x, user.pos.y, user.pos.z, user.on_ground));
        }
        else if (pkt.id == 0x1B)
        {
            std::tuple<double, double, double, float, float ,bool> position_rotation_set;
            position_rotation_set = read_packet(position_rotation_set, pkt.data);
            user.pos.x = std::get<0>(position_rotation_set);
            user.pos.y = std::get<1>(position_rotation_set);
            user.pos.z = std::get<2>(position_rotation_set);
            user.angle.yaw = std::get<3>(position_rotation_set);
            user.angle.pitch = std::get<4>(position_rotation_set);
            user.on_ground = std::get<5>(position_rotation_set);
            log(std::format("Moved to x {} y {} z {} on_ground {} (2)", user.pos.x, user.pos.y, user.pos.z, user.on_ground));
        }
        else if (pkt.id == 0x1C)
        {
            std::tuple<float, float ,bool> rotation_set;
            rotation_set = read_packet(rotation_set, pkt.data);
            user.angle.yaw = std::get<0>(rotation_set);
            user.angle.pitch = std::get<1>(rotation_set);
            user.on_ground = std::get<2>(rotation_set);
        }
        else if (pkt.id == 0x1D)
        {
            std::tuple<bool> set_on_ground;
            set_on_ground = read_packet(set_on_ground, pkt.data);
            user.on_ground = std::get<0>(set_on_ground);
        }
        else if (pkt.id == 0x36)
        {
            std::tuple<minecraft::varint> swing_arm;
            swing_arm = read_packet(swing_arm, pkt.data);
            unsigned char anim = 0;
            if (std::get<0>(swing_arm).num == 1)
                anim = 3;

            std::tuple<minecraft::varint, minecraft::varint, unsigned char> entity_animation =
            {
                minecraft::varint(0x03), minecraft::varint(user.sockfd), anim
            };
            send_everyone_except_user(entity_animation, user.sockfd);
        }
        else if (pkt.id == 0x38)
        {
            std::tuple<minecraft::varint, long, minecraft::varint, float, float, float, bool, minecraft::varint> use_item_on;
            use_item_on = read_packet(use_item_on, pkt.data);
            long val = std::get<1>(use_item_on);
            std::int32_t x = val >> 38;
            std::int32_t y = val << 52 >> 52;
            std::int32_t z = val << 26 >> 38;
            minecraft::varint face = std::get<2>(use_item_on);
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
    	    if (check_collision(user.pos, (position){.x = (double)x, .y = (double)y, .z = (double)z}) == true)
		    return;
	    
            unsigned char anim = 0;
            if (std::get<0>(use_item_on).num == 1)
                anim = 3;
            std::tuple<minecraft::varint, minecraft::varint, unsigned char> entity_animation =
            {
                minecraft::varint(0x03), minecraft::varint(user.sockfd), anim
            };
            send_everyone_except_user(entity_animation, user.sockfd);

            std::tuple<minecraft::varint, long, minecraft::varint> update_block =
            {
                minecraft::varint(0x09), (long long)((((x & (unsigned long)0x3FFFFFF) << 38) | ((z & (unsigned long)0x3FFFFFF) << 12) | (y & (unsigned long)0xFFF))), minecraft::varint(9)
            };
            send_everyone(update_block);

            std::tuple<minecraft::varint, minecraft::varint> awk_block =
            {
                minecraft::varint(0x05), std::get<7>(use_item_on)
            };
            send_packet(awk_block, user.sockfd);
    }
    }
}

void recv_thread()
{
    int events_ready = 0;
    epoll_event events[1024];
    char *main_buffer = (char *)calloc(1024, sizeof(char));

    while (true)
    {
        events_ready = epoll_wait(epfd, events, 1024, -1);
        if (events_ready == -1)
            log("Error! ", strerror(errno));
        //log("Events ready ", events_ready);
        for (int i = 0; i < events_ready;i++)
        {
            int current_fd = events[i].data.fd;
            int status = recv(current_fd, main_buffer, 1024, 0);
            //log("status: ", status);
            if (status == -1 || status == 0)
            {
                netlib::disconnect_server(current_fd, epfd);
                minecraft::uuid uuid = users.find(current_fd)->second.uuid;
                users.erase(current_fd);
                
                std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> remove_entity = 
                {
                    minecraft::varint(0x42), minecraft::varint(1), minecraft::varint(current_fd)
                };
                std::tuple<minecraft::varint, minecraft::varint, minecraft::uuid> remove_info = 
                {
                    minecraft::varint(0x3D), minecraft::varint(1), uuid
                };
                for (auto user: users)
                {
                    if (user.second.state == PLAY)
                    {
                        send_packet(remove_entity, user.second.sockfd);
                        send_packet(remove_info, user.second.sockfd);
                    }
                }
            }
            std::vector<packet> packets = process_packet(main_buffer, current_fd);
            //log("read ", packets.size(), " packets");
            for (auto pack: packets)
            {
                if (pack.id == 0x1 && pack.size == 9)
                {
                    execute_packet(pack, users.find(current_fd)->second);
                    continue;
                }
                tick_packets.push_back(pack);
            }
            memset(main_buffer, 0, 1024);
        }
    }
}

int main()
{
    using clock = std::chrono::system_clock;
    using ms = std::chrono::duration<double, std::milli>;
    int sock = netlib::init_server("0.0.0.0", 25565);
    if (sock == -1)
    {
        log_err("server init failed!");
        return -1;
    }
    epfd = epoll_create1(0);
    log("epfd is ", epfd);
    std::thread accept_t(accept_th, sock);
    accept_t.detach();
    std::thread read_t(recv_thread);
    read_t.detach();
    int ticks_until_keep_alive = 200;
    while (true)
    {   
        const auto before = clock::now();
        for (auto pack: tick_packets)
        {
            log(std::format("Got a packet with id {} and size {}", pack.id, pack.size));
            auto user_ = users.find(pack.sock);
            if (user_ == users.end())
                continue;
            execute_packet(pack, user_->second);
            free(pack.start_data);
        }
        tick_packets.clear();
        for (auto user: users)
        {
            update_players_position(user.second);
        }
        for (auto user: users)
        {
            user.second.prev_pos = user.second.pos;
        }
        if (ticks_until_keep_alive == 0)
        {
            std::tuple<minecraft::varint, long> keep_alive = {minecraft::varint(0x26), 12324};
            for (auto user: users)
            {
                if (user.second.state == PLAY)
                    send_packet(keep_alive, user.second.sockfd);
            }
            ticks_until_keep_alive = 200;
        }
        else
            ticks_until_keep_alive--;
        const ms duration = clock::now() - before;
        //log("MSPT ", duration.count(), "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(50) - duration);
    }
}
