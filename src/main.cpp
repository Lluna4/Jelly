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
#include "libs/chunks.hpp"
#include "libs/tokenize.hpp"
#include "libs/config.hpp"

int epfd;

using login_play = std::tuple<
                minecraft::varint, int, bool, minecraft::varint, minecraft::string, minecraft::varint,
                minecraft::varint, minecraft::varint, bool, bool, bool,
                minecraft::varint, minecraft::string, long,
                unsigned char, char, bool, bool, bool, minecraft::varint, bool>;

using chunk_data_light = std::tuple<minecraft::varint, int, int, char, char, minecraft::chunk,
                                    minecraft::varint, minecraft::varint,long, minecraft::varint, long, 
                                    minecraft::varint, long, minecraft::varint, long, minecraft::varint, 
                                    std::tuple<minecraft::varint, char_size>,
                                    std::tuple<minecraft::varint, char_size>,
                                    std::tuple<minecraft::varint, char_size>,
                                    std::tuple<minecraft::varint, char_size>,
                                    minecraft::varint>;

using json = nlohmann::json;

json blocks;
json items;
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

long time_ticks = 0;
int connected_users = 0;

struct chunk_pos_
{
    int x, z;
};

struct angles
{
    float yaw, pitch;
};

std::map<std::string, std::string> languages = {
    {"af", "afrikaans"},
    {"ar", "arabic"},
    {"as", "asturianu"},
    {"az", "azerbaijani"},
    {"ba", "bashkir"},
    {"be", "belarusian"},
    {"bg", "bulgarian"},
    {"br", "breton"},
    {"bs", "bosnian"},
    {"ca", "catalan"},
    {"cs", "czech"},
    {"cy", "welsh"},
    {"da", "danish"},
    {"de", "german"},
    {"el", "greek"},
    {"en", "english"},
    {"eo", "esperanto"},
    {"es", "spanish"},
    {"et", "estonian"},
    {"eu", "basque"},
    {"fa", "persian"},
    {"fi", "finnish"},
    {"fil", "filipino"},
    {"fo", "faroese"},
    {"fr", "french"},
    {"fy", "frisian"},
    {"ga", "irish"},
    {"gd", "scottish gaelic"},
    {"gl", "galician"},
    {"gv", "manx"},
    {"ha", "hawaiian"},
    {"he", "hebrew"},
    {"hi", "hindi"},
    {"hr", "croatian"},
    {"hu", "hungarian"},
    {"hy", "armenian"},
    {"id", "indonesian"},
    {"ig", "igbo"},
    {"io", "ido"},
    {"is", "icelandic"},
    {"it", "italian"},
    {"ja", "japanese"},
    {"jb", "lojban"},
    {"ka", "georgian"},
    {"kk", "kazakh"},
    {"kn", "kannada"},
    {"ko", "korean"},
    {"ks", "colognian"},
    {"kw", "cornish"},
    {"la", "latin"},
    {"lb", "luxembourgish"},
    {"li", "limburgish"},
    {"lt", "lithuanian"},
    {"lv", "latvian"},
    {"mi", "maori"},
    {"mk", "macedonian"},
    {"mn", "mongolian"},
    {"ms", "malay"},
    {"mt", "maltese"},
    {"nd", "low german"},
    {"nl", "dutch"},
    {"nn", "norwegian nynorsk"},
    {"no", "norwegian"},
    {"oc", "occitan"},
    {"pl", "polish"},
    {"pt", "portuguese"},
    {"qy", "quenya"},
    {"ro", "romanian"},
    {"ru", "russian"},
    {"se", "northern sami"},
    {"sk", "slovak"},
    {"sl", "slovenian"},
    {"so", "somali"},
    {"sq", "albanian"},
    {"sr", "serbian"},
    {"sv", "swedish"},
    {"sx", "saxon"},
    {"sz", "silesian"},
    {"ta", "tamil"},
    {"th", "thai"},
    {"tl", "tagalog"},
    {"tlh", "klingon"},
    {"tr", "turkish"},
    {"tt", "tatar"},
    {"uk", "ukrainian"},
    {"val", "valencian"},
    {"vec", "venetian"},
    {"vi", "vietnamese"},
    {"yi", "yiddish"},
    {"yo", "yoruba"},
    {"zh", "chinese"}
};

struct message_locale
{
    message_locale(std::map<std::string, std::string> messages_, std::string def)
    :messages(messages_), default_message(def)
    {}
    std::map<std::string, std::string> messages;
    std::string default_message;
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
            language = "english";
            render_distance = 8;
            pos = {2.0f, 75.0f, 2.0f};
            prev_pos = {2.0f, 75.0f, 2.0f};
            angle = {0.0f, 0.0f};
            on_ground = true;
            sneaking = false;
            selected_inv = 36;
            inventory.resize(46);
            inventory_item.resize(46);
            overwhelmed = false;
            chunk_pos = {0, 0};
            loading = false;
            ticks_since_keep_alive = 0;
            ticks_to_keep_alive = 200;
        }
        int selected_inv;
        int state;
        int render_distance;
        minecraft::uuid uuid;
        std::string name;
        std::string pronouns;
        std::string locale;
        std::string language;
        position pos;
        chunk_pos_ chunk_pos;
        position prev_pos;
        angles angle;
        bool on_ground;
        bool sneaking;
        int sockfd;
        bool overwhelmed;
        bool loading;
        std::vector<unsigned long> inventory;
        std::vector<unsigned long> inventory_item;
        int ticks_to_keep_alive;
        int ticks_since_keep_alive;
        std::vector<packet> tick_packets;
        void to_file()
        {
            int fd = open(std::format("{}.dat", name).c_str(), O_WRONLY | O_CREAT);
            if (fd == -1)
            {
                std::print("Couldnt save user in file");
                return;
            }
            auto user_data = std::make_tuple(selected_inv, state, render_distance, uuid, minecraft::string(name), 
                minecraft::string(pronouns),
                minecraft::string(locale), pos.x, pos.y, pos.z, chunk_pos.x, chunk_pos.z, angle.pitch, angle.yaw, on_ground, sneaking,
                overwhelmed, inventory);
            write_to_file(user_data, fd);
            close(fd);
        }
        void from_file(std::string file_name)
        {
            int fd = open(file_name.c_str(), O_RDONLY);
            if (fd == -1)
            {
                std::print("Couldnt load user from file");
                return;
            }
            char *buf = (char *)calloc(1024, sizeof(char));
            std::tuple<int, int, int, minecraft::uuid, minecraft::string, minecraft::string, minecraft::string, double, double, double, int, int, float, float, bool, bool, bool> data;
            read(fd, buf, 1024);
            data = read_packet(data, buf);
            //render_distance = std::get<2>(data);
            pronouns = std::get<5>(data).str;
            //locale = std::get<6>(data).str;
            pos.x = std::get<7>(data);
            pos.y = std::get<8>(data);
            pos.z = std::get<9>(data);
            chunk_pos.x = std::get<10>(data);
            chunk_pos.z = std::get<11>(data);
            angle.pitch = std::get<12>(data);
            angle.yaw = std::get<12>(data);
            on_ground = std::get<13>(data);
            sneaking = std::get<14>(data);
            overwhelmed = std::get<15>(data);
            close(fd);
            free(buf);
        }
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
        if (users.find(new_client) != users.end())
        {
            users.erase(new_client);
        }
        users.emplace(new_client, User(new_client));
    }
}

void status_response(User user)
{
	std::string response_str;
	json response;
	json fav;
	std::string base64 = std::format("data:image/png;base64,{}", base64_encode(icon_path));
	response ={
		{"version", {
			{"name", "1.21.1"},
			{"protocol", 767}
		}},
		{"players", {
			{"max", 20},
			{"online", connected_users}
		}},
		{"description", {
			{"text", motd}
		}},
        {"favicon", base64}
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
        if (us.second.sockfd != user.sockfd && us.second.state == PLAY && user.state == PLAY && user.loading == false)
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
        if (us.second.state == PLAY && us.second.loading == false)
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
        if (us.second.state == PLAY && us.second.sockfd != id && us.second.loading == false)
        {
            send_packet(packet, us.second.sockfd);
        }
    }
}

template<typename ...T>
void send_everyone_except_overwhelmed(std::tuple<T...> packet)
{
    for (auto us: users)
    {
        if (us.second.state == PLAY && us.second.overwhelmed == false && us.second.loading == false)
        {
            send_packet(packet, us.second.sockfd);
        }
    }
}

template<typename ...T>
void send_everyone_visible(std::tuple<T...> packet, int x, int y, int z)
{
    float chunk_pos_x = floor(x/16.0f);
    float chunk_pos_z = floor(z/16.0f);
    for (auto us: users)
    {
        if (us.second.state == PLAY && us.second.loading == false)
        {
            if (us.second.chunk_pos.x + us.second.render_distance > (int)chunk_pos_x && us.second.chunk_pos.x - us.second.render_distance < (int)chunk_pos_x)
            {
                if (us.second.chunk_pos.z + us.second.render_distance > (int)chunk_pos_z && us.second.chunk_pos.z - us.second.render_distance < (int)chunk_pos_z)
                    send_packet(packet, us.second.sockfd);
            }
        }
    }
}

void send_chat(std::string message, std::string name)
{
    std::tuple<minecraft::varint, minecraft::string_tag, minecraft::varint, minecraft::string_tag, bool> chat_message =
    {
        minecraft::varint(0x1E), minecraft::string_tag(message),
        minecraft::varint(1), minecraft::string_tag(name), false,
    };
    send_everyone(chat_message);
}

bool check_collision(position player, position block)
{  
    return (player.x - 0.3 < block.x + 1 && player.x + 0.3 > block.x &&
		player.y < block.y + 1 && player.y + 1.8 > block.y &&
		player.z - 0.3 < block.z + 1 && player.z + 0.3 > block.z);
}

void send_chunk(User user, int x, int z)
{
    minecraft::chunk chunk = find_chunk(x, z);
    long sky_light_mask = 0;
    std::bitset<64> b(sky_light_mask);
    b[8] = 1;
    b[9] = 1;
    b[10] = 1;
    b[11] = 1;
    sky_light_mask = b.to_ulong();
    std::cout << b << std::endl;
    long block_light_mask = 0;
    long empty_sky_light_mask = 0;
    empty_sky_light_mask = flipBits(empty_sky_light_mask);
    std::bitset<64> c(empty_sky_light_mask);
    c[8] = 0;
    c[9] = 0;
    c[10] = 0;
    c[11] = 0;
    empty_sky_light_mask = c.to_ulong();
    std::cout << c << std::endl;
    long empty_block_light_mask = 0;
    empty_block_light_mask = flipBits(empty_sky_light_mask);

    char_size sky_data_1 = chunk.sections[7].light;
    char_size sky_data_2 = chunk.sections[8].light;
    char_size sky_data_3 = chunk.sections[9].light;
    char_size sky_data_4 = chunk.sections[10].light;
    chunk_data_light chunk_data = 
    {
        minecraft::varint(0x27), x, z, 0x0a, 0x0, chunk,
        minecraft::varint(0), minecraft::varint(1), sky_light_mask, minecraft::varint(1), block_light_mask, 
        minecraft::varint(1), empty_sky_light_mask, minecraft::varint(1), empty_block_light_mask, minecraft::varint(4),
        {minecraft::varint(2048), sky_data_1},{minecraft::varint(2048), sky_data_2},
        {minecraft::varint(2048), sky_data_3},{minecraft::varint(2048), sky_data_4},
        minecraft::varint(0)
    };
    send_packet(chunk_data, user.sockfd); 
}

void send_world(User &user)
{
    user.loading = true;
    int z = user.chunk_pos.z;
    int x_ = user.chunk_pos.x;
    log(std::format("Chunk pos {}, {}", z, x_));
    std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> set_center_chunk =
    {
        minecraft::varint(0x54), minecraft::varint((unsigned long)(*(unsigned int *)&x_)), minecraft::varint((unsigned long)(*(unsigned int *)&z))
    };
    send_packet(set_center_chunk, user.sockfd);
    for (int y = z - user.render_distance; y <= z + user.render_distance; y++)
    {
        for (int x = x_ - user.render_distance; x <= x_ + user.render_distance; x++)
        {
            send_chunk(user, x, y);
        }
    }
    user.loading = false;
}

void system_chat(minecraft::string message)
{
    std::tuple<minecraft::varint, minecraft::string_tag, bool> system_chat = 
    {
        minecraft::varint(0x6C), minecraft::string_tag(message.str), false
    };
    send_everyone(system_chat);
}

void system_chat_unique(minecraft::string message, int sock)
{
    std::tuple<minecraft::varint, minecraft::string_tag, bool> system_chat = 
    {
        minecraft::varint(0x6C), minecraft::string_tag(message.str), false
    };
    send_packet(system_chat, sock);
}

void system_chat(message_locale msg)
{
    for (auto user: users)
    {
        auto locale_msg = msg.messages.find(user.second.language);
        if (locale_msg == msg.messages.end())
        {
            log(std::format("Couldnt find message for language {}", user.second.language), INFO);
            std::tuple<minecraft::varint, minecraft::string_tag, bool> system_chat = 
            {
                minecraft::varint(0x6C), minecraft::string_tag(msg.default_message), false
            };
            send_packet(system_chat, user.second.sockfd);
        }
        else
        {
            log(std::format("Found message for language {}", user.second.language), INFO);
            std::tuple<minecraft::varint, minecraft::string_tag, bool> system_chat = 
            {
                minecraft::varint(0x6C), minecraft::string_tag(locale_msg->second), false
            };
            send_packet(system_chat, user.second.sockfd);
        }
    }

}

void place_block(std::int32_t x, std::int32_t y, std::int32_t z, int block_id)
{
    float chunk_pos_x = floor(x/16.0f);
    float chunk_pos_z = floor(z/16.0f);
    minecraft::chunk &placed_chunk = find_chunk((int)chunk_pos_x, (int)chunk_pos_z);
    int in_chunk_x = rem_euclid(x, 16);
    int in_chunk_y = rem_euclid(y, 16);
    int in_chunk_z = rem_euclid(z, 16);

    placed_chunk.place_block(in_chunk_x, y, in_chunk_z, block_id);
    log(std::format("Placed {} in chunk {}, {}, section {} in x {} y {} z {}",block_id ,chunk_pos_x, chunk_pos_z, (y + 64)/16 ,rem_euclid(x, 16), rem_euclid(y, 16), z));
}



void update_visible_chunks()
{
    for (auto user: users)
    {
        if (user.second.state == PLAY && user.second.loading == false)
        {
            position pos = user.second.pos;
            chunk_pos_ curr_pos = {(int)(floor(pos.x/16.0f)), (int)(floor(pos.z/16.0f))};
            chunk_pos_ prev_pos = user.second.chunk_pos;
            //log(std::format("curr pos x {} z {}", curr_pos.x, curr_pos.z));
            if (curr_pos.x == prev_pos.x && curr_pos.z == prev_pos.z)
            {
                users.find(user.second.sockfd)->second.chunk_pos = curr_pos;
                continue;
            }
            if (curr_pos.z > prev_pos.z)
            {
                std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> set_center_chunk =
                {
                    minecraft::varint(0x54), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.x)), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.z))
                };
                send_packet(set_center_chunk, user.second.sockfd);
                int z = (curr_pos.z + user.second.render_distance) - 1;
                for (int x = (user.second.render_distance * -1) + curr_pos.x; x < user.second.render_distance + curr_pos.x; x++)
                {
                    send_chunk(user.second, x, z);
                }
                users.find(user.second.sockfd)->second.chunk_pos = curr_pos;
            }
            else if (curr_pos.z < prev_pos.z)
            {
                std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> set_center_chunk =
                {
                    minecraft::varint(0x54), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.x)), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.z))
                };
                send_packet(set_center_chunk, user.second.sockfd);
                int z = (curr_pos.z - user.second.render_distance) + 1;
                for (int x = (user.second.render_distance * -1) + curr_pos.x; x < user.second.render_distance + curr_pos.x; x++)
                {
                    send_chunk(user.second, x, z);		
                }
                users.find(user.second.sockfd)->second.chunk_pos = curr_pos;
            }
            if (curr_pos.x > prev_pos.x)
            {
                std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> set_center_chunk =
                {
                    minecraft::varint(0x54), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.x)), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.z))
                };
                send_packet(set_center_chunk, user.second.sockfd);
                int x = (curr_pos.x + user.second.render_distance) - 1;
                for (int z = (user.second.render_distance * -1) + curr_pos.z; z < user.second.render_distance + curr_pos.z; z++)
                {
                    send_chunk(user.second, x, z);		
                }
                users.find(user.second.sockfd)->second.chunk_pos = curr_pos;
            }
            else if (curr_pos.x < prev_pos.x)
            {
                std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> set_center_chunk =
                {
                    minecraft::varint(0x54), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.x)), minecraft::varint((unsigned long)(*(unsigned int *)&curr_pos.z))
                };
                send_packet(set_center_chunk, user.second.sockfd);
                int x = (curr_pos.x - user.second.render_distance) + 1;
                for (int z = (user.second.render_distance * -1) + curr_pos.z; z < user.second.render_distance + curr_pos.z; z++)
                {
                    send_chunk(user.second, x, z);	
                }
                users.find(user.second.sockfd)->second.chunk_pos = curr_pos;
            }
            log("************SENT CHUNKS************", INFO);
        }
    }
}

void disconnect_user(int current_fd)
{
    netlib::disconnect_server(current_fd, epfd);
    auto current_user = users.find(current_fd)->second;
    if (current_user.state == PLAY)
    {
        system_chat(minecraft::string(std::format("{} disconnected", current_user.name)));
        current_user.to_file();
        connected_users--;
    }
    minecraft::uuid uuid = current_user.uuid;
    users.erase(current_fd);
    
    std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> remove_entity = 
    {
        minecraft::varint(0x42), minecraft::varint(1), minecraft::varint(current_fd)
    };
    std::tuple<minecraft::varint, minecraft::varint, minecraft::uuid> remove_info = 
    {
        minecraft::varint(0x3D), minecraft::varint(1), uuid
    };
    send_everyone(remove_entity);
    send_everyone(remove_info);
}

void disconnect_user(User current_user)
{
    netlib::disconnect_server(current_user.sockfd, epfd);
    if (current_user.state == PLAY)
    {
        system_chat(minecraft::string(std::format("{} disconnected", current_user.name)));
        current_user.to_file();
        connected_users--;
    }
    minecraft::uuid uuid = current_user.uuid;
    users.erase(current_user.sockfd);
    
    std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> remove_entity = 
    {
        minecraft::varint(0x42), minecraft::varint(1), minecraft::varint(current_user.sockfd)
    };
    std::tuple<minecraft::varint, minecraft::varint, minecraft::uuid> remove_info = 
    {
        minecraft::varint(0x3D), minecraft::varint(1), uuid
    };
    send_everyone(remove_entity);
    send_everyone(remove_info);
}

void update_keep_alive()
{
    for (auto &user: users)
    {
        if (user.second.state == PLAY && user.second.loading == false)
        {
            if (user.second.ticks_to_keep_alive > 0)
            {
                user.second.ticks_to_keep_alive--;
                //log(std::format("Ticks to keep alive {}", user.second.ticks_to_keep_alive));
            }
            else if (user.second.ticks_to_keep_alive == 0 && user.second.loading == false)
            {
                std::tuple<minecraft::varint, long> keep_alive = {minecraft::varint(0x26), 12324};
                send_packet(keep_alive, user.second.sockfd);
                user.second.ticks_to_keep_alive--;
            }
            else if (user.second.ticks_to_keep_alive < 0)
            {
                if (user.second.ticks_since_keep_alive > 600)
                {
                    disconnect_user(user.second);
                    continue;
                }
                else
                {
                    user.second.ticks_since_keep_alive++;
                }
            }
        }
    }
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
            send_packet(std::make_tuple(minecraft::varint(1), std::get<0>(ping)), user.sockfd);
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

            auto login_succ = std::make_tuple(minecraft::varint(0x02), user.uuid, minecraft::string(user.name), 
                    minecraft::varint(0x00),(unsigned char)true);
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
            auto language = languages.find(user.locale.substr(0, 2));
            if (language == languages.end())
            {
                user.language = "english";
            }
            else
                user.language = language->second;
            log(std::format("New user has {} language", user.language));
            user.render_distance = (int)std::get<1>(client_information);

            auto known_packs = std::make_tuple(minecraft::varint(0x0E), minecraft::varint(1),
                minecraft::string("minecraft"), minecraft::string("core"), minecraft::string("1.21"));
            auto brand = std::make_tuple(minecraft::varint(0x01), minecraft::string("minecraft:brand"), minecraft::string("Jelly"));
            send_packet(brand, user.sockfd);
            send_packet(known_packs, user.sockfd);
            registry_data(user);
            std::tuple<minecraft::varint> end_config = {minecraft::varint(0x03)};
            send_packet(end_config, user.sockfd);
        }
        if (pkt.id == 0x03)
        {
            user.state = PLAY;
            if (std::filesystem::exists(std::format("{}.dat", user.name)))
                user.from_file(std::format("{}.dat", user.name));
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

            auto commands = std::make_tuple(minecraft::varint(0x11), minecraft::varint(4),
                    std::make_tuple((char)0x00, minecraft::varint(3), minecraft::varint(1), minecraft::varint(2), minecraft::varint(3)),
                    std::make_tuple((char)0x01, minecraft::varint(1), minecraft::varint(2), minecraft::string("pronouns")),
                    std::make_tuple((char)0x02, minecraft::varint(0), minecraft::string("pronouns"), minecraft::varint(5), minecraft::varint(2)),
                    std::make_tuple((char)0x01, minecraft::varint(0), minecraft::string("overwhelmed")),
                    minecraft::varint(0));

            send_packet(commands, user.sockfd);
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
                        minecraft::string_tag(user_name)
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
			send_everyone_visible(spawn_entity_user, user.pos.x, user.pos.y, user.pos.z);
            message_locale conn_msg({{"english", std::format("{} connected", user.name)},
                                     {"spanish", std::format("{} se conectó", user.name)},
                                     {"catalan", std::format("{} s'ha connectat", user.name)}}, std::format("{} connected", user.name));
            system_chat(conn_msg);
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
                                minecraft::string_tag(name)
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
            auto set_time = std::make_tuple(minecraft::varint(0x64), (long)time_ticks, (long)(time_ticks%24000));
            send_packet(set_time, user.sockfd);
            auto set_effect = std::make_tuple(minecraft::varint(0x76), minecraft::varint(user.sockfd), minecraft::varint(15),
            minecraft::varint(1), minecraft::varint(240000), (char)0x04);
            send_packet(set_effect, user.sockfd);
            std::tuple<minecraft::varint, minecraft::varint, minecraft::varint> set_center_chunk =
            {
                minecraft::varint(0x54), minecraft::varint(0), minecraft::varint(0)
            };
            send_packet(set_center_chunk, user.sockfd);

            std::tuple<minecraft::varint, unsigned char, float> game_event = 
            {
                minecraft::varint(0x22), 13, 0.0f
            };
            send_packet(game_event, user.sockfd);
            std::tuple<minecraft::varint, float, bool> tick_state =
            {
                minecraft::varint(0x71), 20.0f, false
            };
            send_packet(tick_state, user.sockfd);
            send_chunk(user, user.chunk_pos.x, user.chunk_pos.z);
            std::thread send_world_th(send_world, std::ref(user));
            send_world_th.detach();  
            connected_users++; 
        }
    }
    if (user.state == PLAY)
    {
        if (pkt.id == 0x04)
        {
            std::tuple<minecraft::string> command;
            command = read_packet(command, pkt.data);
            std::string commands = std::get<0>(command).str;
            log(commands);
            if (commands.starts_with("pronouns"))
            {
                std::string command_contents = commands.substr(commands.find(' ') + 1);
                user.pronouns = command_contents;
                std::string user_name = std::format("{} [{}]", user.name, user.pronouns);
                std::tuple<minecraft::varint, char, minecraft::varint, minecraft::uuid,
                        minecraft::string, minecraft::varint, bool ,bool, minecraft::string_tag> info_update_head_user =
                        {
                            minecraft::varint(0x3E), 0x01 |0x08 | 0x20, minecraft::varint(1), user.uuid, 
                            minecraft::string(user.name), minecraft::varint(0), true, true,
                            minecraft::string_tag(user_name)
                        };
                send_everyone(info_update_head_user);
                if (user.locale.compare("en_us") == 0 || user.locale.compare("en_uk") == 0)
                	system_chat_unique(std::format("Your pronouns have changed to {}", command_contents), user.sockfd);
                else if (user.locale.compare("es_es") == 0)
                    system_chat_unique(std::format("Tus pronombres han cambiado a {}", command_contents), user.sockfd);
                else if (user.locale.compare("ca_es") == 0)
                    system_chat_unique(std::format("Els teus pronoms han canviat a {}", command_contents), user.sockfd);
            }
            if (commands.starts_with("overwhelmed"))
            {
                user.overwhelmed = true;
                if (user.pronouns.contains("/") == true)
                {
                    std::vector<std::string> res = tokenize(user.pronouns, '/');
                    if (res.size() > 0)
                    {
                        if (res[0].compare("they") == 0)
                            res[0] = "them";
                        else if (res[0].compare("she") == 0)
                            res[0] = "her";
                        else if (res[0].compare("he") == 0)
                            res[0] = "him";
                        send_chat(std::format("{} is overwhelmed and will not see the chat, please keep interaction with {} as low as possible", user.name, res[0]), "SERVER [it/its]");
                    }
                    else
                        send_chat(std::format("{} is overwhelmed and will not see the chat, please keep interaction with them as low as possible", user.name), "SERVER [it/its]");
                }
                else
                    send_chat(std::format("{} is overwhelmed and will not see the chat, please keep interaction with them as low as possible", user.name), "SERVER [it/its]");
            }
        }
        else if (pkt.id == 0x06)
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
            log(std::format("Moved to x {} y {} z {} on_ground {} (1)", user.pos.x, user.pos.y, user.pos.z, user.on_ground), INFO);
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
            log(std::format("Moved to x {} y {} z {} on_ground {} (2)", user.pos.x, user.pos.y, user.pos.z, user.on_ground), INFO);
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
        else if (pkt.id == 0x24)
        {
            std::tuple<minecraft::varint, long, char, minecraft::varint> player_action;
            player_action = read_packet(player_action, pkt.data);
            long val = std::get<1>(player_action);
            std::int32_t x = val >> 38;
            std::int32_t y = val << 52 >> 52;
            std::int32_t z = val << 26 >> 38;
            if (std::get<0>(player_action).num == 0)
            {
                place_block(x, y, z, 0);
                unsigned char anim = 0;
                std::tuple<minecraft::varint, minecraft::varint, unsigned char> entity_animation =
                {
                    minecraft::varint(0x03), minecraft::varint(user.sockfd), anim
                };
                send_everyone_except_user(entity_animation, user.sockfd);

                std::tuple<minecraft::varint, long, minecraft::varint> update_block =
                {
                    minecraft::varint(0x09), (long long)((((x & (unsigned long)0x3FFFFFF) << 38) | ((z & (unsigned long)0x3FFFFFF) << 12) | (y & (unsigned long)0xFFF))), minecraft::varint(0)
                };
                send_everyone_visible(update_block, x, y, z);
                std::tuple<minecraft::varint, minecraft::varint> awk_block =
                {
                    minecraft::varint(0x05), std::get<3>(player_action)
                };
                if (user.loading == false)
                    send_packet(awk_block, user.sockfd);
            }
            log("Removed block");
        }
        else if (pkt.id == 0x18)
        {
            std::tuple<long> keep_alive_response;
            keep_alive_response = read_packet(keep_alive_response, pkt.data);
            if (std::get<0>(keep_alive_response) == 12324)
            {
                user.ticks_to_keep_alive = 200;
                user.ticks_since_keep_alive = 0;
            }
            else
            {
                disconnect_user(user);
            }
        }
        else if (pkt.id == 0x2F)
        {
            std::tuple<short> set_held_item;
            set_held_item = read_packet(set_held_item, pkt.data);
            user.selected_inv = std::get<0>(set_held_item) + 36;
            auto set_equipment = std::make_tuple(minecraft::varint(0x5B), minecraft::varint(user.sockfd), (char)0, 
            minecraft::varint(1), minecraft::varint(user.inventory_item[user.selected_inv]), minecraft::varint(0), minecraft::varint(0));
            send_everyone_except_user(set_equipment, user.sockfd);
        }
        else if (pkt.id == 0x32)
        {
            std::tuple<short, minecraft::varint> set_slot;
            set_slot = read_packet(set_slot, pkt.data);
            pkt.data += 3;
            if (std::get<1>(set_slot).num > 0)
            {
                std::tuple<minecraft::varint> item_id;
                item_id = read_packet(item_id, pkt.data);
                log(std::format("Got {} items with id {}", std::get<1>(set_slot).num, std::get<0>(item_id).num));
                std::string name = items[std::get<0>(item_id).num]["name"];
                log(std::format("name is {}", name), INFO);
                user.inventory_item[std::get<0>(set_slot)] = std::get<0>(item_id).num;
                if (std::get<0>(set_slot) == user.selected_inv)
                {
                    auto set_equipment = std::make_tuple(minecraft::varint(0x5B), minecraft::varint(user.sockfd), (char)0, 
                    minecraft::varint(1), std::get<0>(item_id), minecraft::varint(0), minecraft::varint(0));
                    send_everyone_except_user(set_equipment, user.sockfd);
                }
                for (auto obj: blocks)
                {
                    std::string name2 = obj["name"];
                    if (name.compare(name2) == 0)
                    {
                        user.inventory[std::get<0>(set_slot)] = obj["defaultState"];
                        break;
                    }
                }
            }
            else if (std::get<1>(set_slot).num == 0)
            {
                user.inventory[std::get<0>(set_slot)] = 0;
            }
        }
        else if (pkt.id == 0x38)
        {
            std::tuple<minecraft::varint, long, minecraft::varint, float, float, float, bool, minecraft::varint> use_item_on;
            use_item_on = read_packet(use_item_on, pkt.data);
            long val = std::get<1>(use_item_on);
            std::int32_t x = val >> 38;
            std::int32_t y = val << 52 >> 52;
            std::int32_t z = val << 26 >> 38;
            log(std::format("x {} y {} z {}", x, y, z));
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
            
            for (auto us: users)
            {
                if (us.second.state == PLAY)
                {
                    if (check_collision(us.second.pos, (position){.x = (double)x, .y = (double)y, .z = (double)z}) == true)
                    {
                        log("Block collides with player!");
		                return;
                    }
                }
            }
            log(std::format("Placed block {} in position {}",user.inventory[user.selected_inv], user.selected_inv));
            place_block(x, y, z, user.inventory[user.selected_inv]);
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
                minecraft::varint(0x09), (long long)((((x & (unsigned long)0x3FFFFFF) << 38) | ((z & (unsigned long)0x3FFFFFF) << 12) | (y & (unsigned long)0xFFF))), minecraft::varint(user.inventory[user.selected_inv])
            };
            send_everyone_visible(update_block, x, y, z);
            
            std::tuple<minecraft::varint, minecraft::varint> awk_block =
            {
                minecraft::varint(0x05), std::get<7>(use_item_on)
            };
            if (user.loading == false)
                send_packet(awk_block, user.sockfd);
            log("Placed block");
        }
    }
}

void recv_thread()
{
    int events_ready = 0;
    epoll_event events[1024];
    char *main_buffer = (char *)calloc(4096, sizeof(char));
    char_size buff = {.data = main_buffer, .consumed_size = 0, .max_size = 4096, .start_data = main_buffer};

    while (true)
    {
        events_ready = epoll_wait(epfd, events, 1024, -1);
        if (events_ready == -1)
            log(std::format("Error! {}", strerror(errno)), ERROR);
        //log("Events ready ", events_ready);
        for (int i = 0; i < events_ready;i++)
        {
            int current_fd = events[i].data.fd;
            std::vector<packet> packets;
            while (true)
            {
                int status = recv(current_fd, buff.data, 4, 0);
                buff.start_data = buff.data;
                //log("status: ", status);
                if (status == -1 || status == 0)
                {
                    disconnect_user(current_fd);
                }

                minecraft::varint lenght = read_varint(buff.data);
                int index = status;
                int status2 = 0;
                status -= lenght.size;
                while (status < lenght.num && status < 4096)
                {
                    status2 = recv(current_fd, &buff.data[index], lenght.num - (4 - lenght.size), 0);
                    if (status2 == -1 || status2 == 0)
                        disconnect_user(current_fd);
                    index += status2;
                    status+= status2;
                }
                packets.push_back(process_packet(&buff, current_fd, status));
                int remaining_count;
                ioctl(current_fd, FIONREAD, &remaining_count);
                log(remaining_count, INFO);
                if (remaining_count == 0)
                    break;
            }
            //log("read ", packets.size(), " packets");
            for (auto pack: packets)
            {
                if (pack.id == 0x1 && pack.size == 9)
                {
                    execute_packet(pack, users.find(current_fd)->second);
                    continue;
                }
                users.find(current_fd)->second.tick_packets.push_back(pack);
            }
            memset(buff.start_data, 0, 4096);
            buff.data = buff.start_data;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-no_light_postprocessing") == 0)
            LIGHT_POSTPROCESSING = false;
    }
    using clock = std::chrono::system_clock;
    using ms = std::chrono::duration<double, std::milli>;
    srandom(time(NULL));
    s = random();
    if (std::filesystem::exists("server.properties") == false)
    {
        create_config();
        log("Config file created!", INFO);
    }
    load_config();
    int sock = netlib::init_server(SV_IP, SV_PORT);
    if (sock == -1)
    {
        log("server init failed!", ERROR);
        return -1;
    }
	if (std::filesystem::exists("logs") == false)
	{
		std::filesystem::create_directory("logs");
		log("Created logging folder!");
	}

	generate_file_name();
    epfd = epoll_create1(0);
    log(std::format("epfd is {}", epfd), INFO);
    std::thread accept_t(accept_th, sock);
    accept_t.detach();
    std::thread read_t(recv_thread);
    read_t.detach();
    for (int y = -12; y < 12; y++)
    {
        for (int x = -12; x < 12; x++)
        {
            find_chunk(x, y);
        }
    }

    for (int i = 0; i < 100; i++)
    {
        int x = (random()%(24 * 16) - (12 * 16));
        int z = (random()%(24 * 16) - (12 * 16));
        auto& ch = find_chunk(floor(x/16.0f), floor(z/16.0f));
        int y = ch.height[(rem_euclid(z, 16)*16) + rem_euclid(x, 16)];
        if (y < 64)
        {
            i--;
            continue;
        }
        place_block(x, y, z, 146);
        place_block(x, y + 1, z, 146);
        place_block(x, y + 2, z, 146);
        place_block(x, y + 3, z, 146);
        place_block(x + 1, y + 3, z, 146);
        place_block(x + 2, y + 3, z, 146);
        place_block(x + 2, y + 4, z, 146);
        place_block(x + 2, y + 5, z, 146);
        int diff = 4;
        int diff2 = 0;
        for (int i = y + 5; i < y + 10;i++)
        {
            if (i == y + 7)
            {
                diff++;
                diff2--;
            }
            if (i == y + 8)
            {
                diff--;
                diff2++;
            }
            if (i == y + 9)
            {
                diff--;
                diff2++;
            }
            for (int ii = (z - 2) + diff2; ii < z + diff; ii++)
            {
                for (int iii = x + diff2; iii < x + (diff + 2); iii++)
                {
                    int r = random()%100;
                    if (r < 80)
                        place_block(iii, i, ii, 404);
                }
            }
        }
    }
    
    log("world created", INFO);
    std::ifstream f("items.json");
    items = json::parse(f);
    f.close();
    std::ifstream fs("blocks.json");
    blocks = json::parse(fs);
    fs.close();
    int ticks_until_keep_alive = 200;
    while (true)
    {   
        const auto before = clock::now();
        for (auto &user: users)
        {
            for (int i = 0; i < user.second.tick_packets.size();i++)
            {
                packet pack = user.second.tick_packets[i];     
                log(std::format("Got a packet with id {} and size {}", pack.id, pack.size));
                execute_packet(pack, user.second);
                free(pack.start_data);
                user.second.tick_packets.erase(user.second.tick_packets.begin() + i);
            }
        }
        //tick_packets.clear();
        for (auto user: users)
        {
            update_players_position(user.second);
        }
        for (auto user: users)
        {
            user.second.prev_pos = user.second.pos;
        }
        update_visible_chunks();
        update_keep_alive();
        time_ticks++;
        const ms duration = clock::now() - before;
        //log(std::format("MSPT {}ms", duration.count()), INFO);
        if (duration.count() <= 50)
            std::this_thread::sleep_for(std::chrono::milliseconds(50) - duration);
    }
}
