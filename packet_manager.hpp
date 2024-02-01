#include "user.hpp"
#include "logging.hpp"
#include <sys/socket.h>
#include <vector>
#include <stdlib.h>

int safe_memcpy(char *dest, char *dest_lim, char *src, int n)
{
    int ret = 0;
    while (dest != dest_lim && ret < n && *src != '\0')
    {
        *dest = *src;
        ret++;
        dest++;
        src++;
    }
    return ret;
}


void pkt_manager(int sock, std::vector<std::string> &packets)
{
    char buffer[1025];
    char *pkt;
    char *ptr = buffer; //pointer to the start of every packet, used for iterating
    std::string ret;
    int size = 0;
    while (1)
    {
        if (*ptr == '\0')
            recv(sock, buffer, 1024, 0);
            ptr = buffer;
        size = (int)*ptr;
        pkt = (char *)calloc(size + 1, sizeof(char));
        safe_memcpy(pkt, pkt + size, ptr, size);
        packets.push_back(strdup(pkt));
        free(pkt);
        ptr = ptr + size + 1;
    }
}