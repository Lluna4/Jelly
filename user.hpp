#include <uuid_v4/uuid_v4.h>
#include <uuid_v4/endianness.h>

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