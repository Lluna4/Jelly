#pragma once
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include "tokenize.hpp"
#include "logging.hpp"
#include <uuid_v4/uuid_v4.h>
#include <uuid_v4/endianness.h>

/*
struct uuid
{
	uint64_t first;
	uint64_t last;
};

uuid gen_uuid()
{
	uuid ret;
	typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
	uint32_t seed_val = std::chrono::system_clock::now().time_since_epoch().count();
	MyRNG rng;
	rng.seed(seed_val);
	std::uniform_int_distribution<uint64_t> uint_dist;
	ret.first = uint_dist(rng);
	ret.last = uint_dist(rng);
	std::cout << ret.first << std::endl;
	std::cout << ret.last << std::endl;
	return ret;
}*/

struct position
{
    double x, y, z;
    float yaw, pitch;
};

class User
{
    public:
        User() 
        {
            UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
            uuid = uuidGenerator.getUUID();
            pos = {.x = 0.0f, .y = 1000.0f, .z = 64.0f, .yaw = 0.0f, .pitch = 0.0f};
            pronouns = "they/them";
            state = 0;
        }

        User(std::string uname, int socket)
            :uname_(uname), socket_(socket)
        {
            UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
            uuid = uuidGenerator.getUUID();
            pos = {.x = 0.0f, .y = 1000.0f, .z = 64.0f, .yaw = 0.0f, .pitch = 0.0f};
            pronouns = "they/them";
            state = 0;
        }

        UUIDv4::UUID get_uuid()
        {
            return uuid;
        }

        std::string get_uname()
        {
            return uname_;
        }
        int get_state()
        {
            return state;
        }
        int get_socket()
        {
            return socket_;
        }

        std::string get_locale()
        {
            return locale;
        }
        
        int get_render_distance()
        {
            return render_distance;
        }

        void update_position(position p)
        {
            pos = p;
        }

        struct position get_position()
        {
            return pos;
        }

        void set_uuid(UUIDv4::UUID uuid_)
        {
            uuid = uuid_;
        }

        void set_uname(std::string uname)
        {
            uname_ = uname;
        }

        void set_socket(int socket)
        {
            socket_ = socket;
        }

        void set_locale(std::string locale_)
        {
            locale = locale_;
        }

        void set_render_distance(int render)
        {
            render_distance = render;
        }

        std::string get_pronouns()
        {
            return pronouns;
        }

        void set_pronouns(std::string pron)
        {
            pronouns = pron;
        }

        void set_state(int stat)
        {
            state = stat;
        }

        void to_file()
        {
            std::ofstream file(uname_);
            file << uuid.str() << std::endl;
            file << locale << std::endl;
            file << render_distance << std::endl;
            file << pos.pitch << "," << pos.yaw << "," << pos.x << "," << pos.y << "," << pos.z << std::endl;
            file << pronouns << std::endl;
            file.close();
        }

        void from_file(std::string filename)
        {
            std::ifstream file(filename);
            std::string line;
            int index = 0;
            std::vector<std::string> tokens;
            while (std::getline(file, line))
            {
                switch (index)
                {
                case 0:
                    uuid.fromStr(line.c_str());
                    break;
                case 1:
                    locale = line;
                    break;
                case 2:
                    render_distance = atoi(line.c_str());
                    break;
                case 3:
                    tokens = tokenize(line, ',');
                    if (tokens.size() != 5)
                    {
                        log_err("Invalid position in file!");
                        return;
                    }
                    pos.pitch = std::stof(tokens[0]);
                    pos.yaw = std::stof(tokens[1]);
                    pos.x = std::stod(tokens[2]);
                    pos.y = std::stod(tokens[3]);
                    pos.z = std::stod(tokens[4]);
                    break;
                case 4:
                    pronouns = line;
                    break;
                }
                index++;
            }
        }

    private:
        UUIDv4::UUID uuid;
        std::string uname_;
        int socket_;
	    std::string locale;
	    int render_distance;
        struct position pos;
        std::string pronouns;
        int state;
};
