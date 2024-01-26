#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <format>

enum req {STATUS = 1, LOGIN = 2};

static int	ft_intlen(int n)
{
	int	ret;

	ret = 1;
	while (n >= 10)
	{
		ret++;
		n = n / 10;
	}
	return (ret);
}

static std::string ft_make_ret(int n, int sign)
{
	int		len;

	len = ft_intlen(n) + sign;
	len--;
    std::string ret(len + 1, '\0');
	while (len >= 0)
	{
		ret[len] = (n % 10) + '0';
		n = n / 10;
		len--;
	}
	if (sign == 1)
		ret[0] = '-';
	return (ret);
}

std::string	ft_itoa(int n)
{
    std::string ret;
	int		sign;

	sign = 0;
	if (n == -2147483648)
	{
		ret = "-2147483648";
		return (ret);
	}
	if (n < 0)
	{
		n *= -1;
		sign = 1;
	}
	return (ft_make_ret(n, sign));
}


std::size_t ReadUleb128(const char* addr, unsigned long* ret) {
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

char *read_string(char *addr, unsigned long len)
{
    char *ret = (char *)calloc(len + 1, sizeof(char));
    int pos = 0;
    while (pos < len)
    {
        ret[pos] = *addr;
        pos++;
        addr++;
    }
    return ret;
}

char *packet_login_success(boost::uuids::uuid uuid, char *uname)
{
    char *ret = (char*)calloc(100, sizeof(char));
    char **iter;
    int pos = 0;
    //std::string ret;
    ret[pos] = strlen(uname) + 19;
    ret[pos + 1] = 0x02;
    iter = &ret;
    pos = pos + 2;
    strlcpy(&ret[pos], (char *)uuid.data, strlen((char *)uuid.data));
    pos = pos + 16;
    ret[pos] = strlen(uname);
    pos++;
    strlcpy(&ret[pos], uname, strlen(uname) + 1);
    return ret;
}

int main() 
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    unsigned long host_len = 0;
    unsigned long prot = 0;
    unsigned long port = 0;
    boost::uuids::uuid uuid;
    std::string packet;

    struct sockaddr_in addr = {
        AF_INET,
        htons(8081), // Use a specific port, e.g., 8080
        0
    };

    bind(s, (struct sockaddr*)&addr, sizeof(addr));

    listen(s, 10);
    char buffer[1024] = {0};
    char *buff;
    //char *uuid;
    int client_fd = accept(s, 0, 0);
    recv(client_fd, buffer, 100, 0);
    //std::cout << buffer << std::endl;
    char *a = buffer + 2;
    ReadUleb128(a, &prot);
    a += 2;
    ReadUleb128(a, &host_len);
    a++;
    std::cout << prot << std::endl;
    std::cout << host_len << std::endl;
    buff = read_string(a, host_len);
    std::cout << buff << std::endl;
    a = a + host_len;
    a = a + 2;
    if ((int)*a == LOGIN)
    {
        a += 3;
        ReadUleb128(a, &host_len);
        a++;
        std::cout << host_len << std::endl;
        buff = read_string(a, host_len);
        std::cout << buff << std::endl;
        a = a + host_len;
        std::memcpy(uuid.data, a, 16);
        std::cout << a << std::endl;
        char *pack = packet_login_success(uuid, buff);
        std::cout << pack << std::endl;
        send(client_fd, pack, strlen(pack), 0);
        recv(client_fd, buffer, 100, 0);
        std::cout << buffer << std::endl;
    }
}