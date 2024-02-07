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
class User
{
    public:
        User() 
        {
            UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
            uuid = uuidGenerator.getUUID();
        }

        User(std::string uname, int socket)
            :uname_(uname), socket_(socket)
        {
            UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
            uuid = uuidGenerator.getUUID();
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

    private:
        UUIDv4::UUID uuid;
        std::string uname_;
        int socket_;
};
