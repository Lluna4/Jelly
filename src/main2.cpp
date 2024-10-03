#include "libs/netlib.h"
#include "libs/utils.hpp"
#include <vector>
#include <map>
#include <thread>
#include <any>
#include "libs/packet_processing.hpp"
#include <chrono>
#include <errno.h>
#include "../nlohmann_json/json.hpp"
#include "libs/comp_time_write.hpp"
#include "libs/comp_time_read.hpp"

int epfd;
std::vector<packet> tick_packets;

using json = nlohmann::json;

enum states
{
    HANDSHAKE,
    STATUS,
    LOGIN,
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
            pos = {0.0f, 0.0f, 0.0f};
            angle = {0.0f, 0.0f};
            on_ground = true;
            sneaking = false;
        }
        
        int state;
        minecraft::uuid uuid;
        std::string name;
        std::string pronouns;
        std::string locale;
        position pos;
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
	std::string base64 = std::format("data:image/png;base64,{}", base64_encode("img.png"));
    char *buffer = (char *)malloc(1024 * sizeof(char));
    char *start_buffer = buffer;
	response ={
		{"version", {
			{"name", "1.20.4"},
			{"protocol", 765}
		}},
		{"players", {
			{"max", 20},
			{"online", 1}
		}},
		{"description", {
			{"text", "Hiiiii! :3"}
		}},
		{"favicon", base64}
	};
	response_str = response.dump();
	std::cout << response_str << std::endl;
    minecraft::string str = (minecraft::string){.len = response_str.length(), .string = response_str};
    std::tuple<minecraft::varint, minecraft::varint, minecraft::string> status_pkt = 
        {minecraft::varint(str.len + 1), minecraft::varint(0), str};
    constexpr std::size_t size = std::tuple_size_v<decltype(status_pkt)>;
    write_comp_pkt(size, buffer, status_pkt);
    send(user.sockfd, start_buffer, 1024, 0);
}

void recv_thread()
{
    int events_ready = 0;
    epoll_event events[1024];
    char *main_buffer = (char *)calloc(1024, sizeof(char));

    while (true)
    {
        events_ready = epoll_wait(epfd, events, 1024, 20);
        if (events_ready == -1)
            log("Error! ", strerror(errno));
        //log("Events ready ", events_ready);
        for (int i = 0; i < events_ready;i++)
        {
            int current_fd = events[i].data.fd;
            int status = recv(current_fd, main_buffer, 1024, 0);
            if (status == -1 || status == 0)
            {
                netlib::disconnect_server(current_fd, epfd);
            }
            std::vector<packet> packets = process_packet(main_buffer, current_fd);
            log("read ", packets.size(), " packets");
            for (auto pack: packets)
                tick_packets.push_back(pack);
            memset(main_buffer, 0, 1024);
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
                constexpr std::size_t size = std::tuple_size_v<decltype(handshake)>;
                read_comp_pkt(size, pkt.data, handshake);
                log(std::format("Got a handshake packet, version: {}, address {}, port {}, state {}", 
                std::get<0>(handshake).num, std::get<1>(handshake).string, std::get<2>(handshake), std::get<3>(handshake).num));
                user.state = std::get<3>(handshake).num;
                break;
        }
    }
    if (user.state == STATUS)
    {

    }
}

int main()
{
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

    while (true)
    {   
        for (auto pack: tick_packets)
        {
            log(std::format("Got a packet with id {} and size {}", pack.id, pack.size));
            auto user_ = users.find(pack.sock);
            if (user_ == users.end())
                continue;
            execute_packet(pack, user_->second);
            
        }
        tick_packets.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
