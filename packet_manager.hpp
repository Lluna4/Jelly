#include "user.hpp"
#include "logging.hpp"
#include <sys/socket.h>
#include <vector>
#include <stdlib.h>

std::size_t ReadUleb128(const char* addr, unsigned long* ret) 
{
  unsigned long result = 0;
  int shift = 0;
  std::size_t count = 0;
  

  	while (1) {
    unsigned char byte = *reinterpret_cast<const unsigned char*>(addr);
    addr++;
    count++;

    result |= (byte & 0x7f) << shift;
    shift += 7;

    if (!(byte & 0x80)) break;
  }

  *ret = result;

  return count;
}

struct packet
{
    unsigned long id;
    unsigned long lenght;
    std::string data;
};

void proc_pkt(char *buffer, std::vector<packet> &packets)
{
    unsigned long size = 0;
    unsigned long id = 0;
    std::size_t steps = 0;
    char *pkt;
    struct packet ret;
    char *b;
    while (*buffer != '\0')
    {
        steps = ReadUleb128(buffer, &size);
        buffer = buffer + steps;
        b = buffer;
        pkt = (char *)calloc(size + 1, sizeof(char));
        steps = ReadUleb128(buffer, &id);
        buffer = buffer + steps;
        std::memcpy(pkt, buffer, (size));
        ret = {
            id,
            size,
            pkt
        };
        packets.push_back(ret);
        log("New packet");
        log("Id ", id);
        log("Size ", size);
        log("Data ", pkt);
        free(pkt);
        buffer = b + (size);
    }
}

void pkt_manager(int sock, std::vector<packet> &packets)
{
    char buffer[1025];
    char *pkt;
    int id = 0;
    char *ptr = buffer; //pointer to the start of every packet, used for iterating
    std::string data;
    struct packet ret;
    int status = 0;
    while (1)
    {
        status = recv(sock, buffer, 1024, 0);
        if (status == -1)
            break;
        ptr = buffer;
        proc_pkt(ptr, packets);
    }
}