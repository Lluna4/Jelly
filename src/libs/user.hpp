#pragma once
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include "tokenize.hpp"
#include "logging.hpp"
#include "utils.hpp"

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
struct chunk_pos
{
    int x, y;
};

class User
{
    public:
        User() 
        {
            pos = {.x = 0.0f, .y = 64.0f, .z = 0.0f, .yaw = 0.0f, .pitch = 0.0f};
            chunk_p = {.x = 0, .y = 0};
            pronouns = "they/them";
            state = 0;
            on_ground = true;
            sneaking = true;
        }

        User(std::string uname, int socket)
            :uname_(uname), socket_(socket)
        {
            uuid_.generate(uname_);
            pos = {.x = 0.0f, .y = 64.0f, .z = 0.0f, .yaw = 0.0f, .pitch = 0.0f};
            chunk_p = {.x = 0, .y = 0};
            pronouns = "they/them";
            state = 0;
            on_ground = true;
            sneaking = true;
        }

        minecraft::uuid get_uuid()
        {
            return uuid_;
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
            chunk_p = {.x = (int)floor(p.x/16.0f), .y = (int)floor(p.z/16.0f)};
        }

        struct chunk_pos get_chunk_position()
        {
            return chunk_p;
        }

        struct position get_position()
        {
            return pos;
        }

        bool get_on_ground()
        {
            return on_ground;
        }
        
        void set_on_ground(bool set)
        {
            on_ground = set;
        }
        void set_uuid(minecraft::uuid uuid)
        {
            uuid_ = uuid;
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

        void set_sneaking(bool sneak)
        {
            sneaking = sneak;
        }

        bool get_sneaking()
        {
            return sneaking;
        }

        void to_file()
        {
            std::ofstream file(uname_);
            file << uuid_.buff << std::endl;
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
                    uuid_.uuid = line;
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
                    chunk_p = {.x = (int)(pos.x/16), .y = (int)(pos.z/16)};
                    break;
                case 4:
                    pronouns = line;
                    break;
                }
                index++;
            }
        }

    private:
        minecraft::uuid uuid_;
        std::string uname_;
        int socket_;
	    std::string locale;
	    int render_distance;
        struct position pos;
        std::string pronouns;
        int state;
        struct chunk_pos chunk_p;
        bool on_ground;
        bool sneaking;
};
