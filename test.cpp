#include "packet_manager.hpp"

int main()
{
    char buffer[10];
    char *end = buffer + 10;
    char aaa[] = "asasasaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    int ret = safe_memcpy(buffer, end, aaa, 999);
    log("Return: " , ret);
    log("Result: ", buffer);
}