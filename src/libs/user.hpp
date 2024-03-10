#pragma once
#include <random>
#include <chrono>
#include <iostream>
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
        }

        User(std::string uname, int socket)
            :uname_(uname), socket_(socket)
        {
            UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
            uuid = uuidGenerator.getUUID();
            pos = {.x = 0.0f, .y = 1000.0f, .z = 64.0f, .yaw = 0.0f, .pitch = 0.0f};
            pronouns = "they/them";
        }

        UUIDv4::UUID get_uuid()
        {
            return uuid;
        }

        std::string get_uname()
        {
            return uname_;
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

    private:
        UUIDv4::UUID uuid;
        std::string uname_;
        int socket_;
	    std::string locale;
	    int render_distance;
        struct position pos;
        std::string pronouns;
};
