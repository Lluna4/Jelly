#pragma once
#include <string>
#include <ctime>
#include <cstring>
#include <vector>
#if __has_include (<format>) && __cplusplus >= 202002L
	#include <format>
#else
	#include <stdio.h>
	#define NO_FORMAT
	#pragma warn("No hay <format>")
#endif

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


std::string get_time()
{
	time_t tiempo;
	time(&tiempo);
	struct tm* a = (tm*)malloc(1 * sizeof(tm));
	localtime_r(&tiempo, a);
    std::string h = ft_itoa(a->tm_hour);
    std::string min = ft_itoa(a->tm_min);
    std::string sec = ft_itoa(a->tm_sec);
	#ifndef NO_FORMAT
	if (h.length() == 1)
		h = std::format("{}{}", "0", h);
	if (min.length() == 1)
		min = std::format("{}{}", "0", min);
    if (sec.length() == 1)
		sec = std::format("{}{}", "0", sec);
	std::string ret = std::format("{}:{}:{}", h, min, sec);
	#else
	char buffer[20] = {0};
	if (h.length() == 1)
	{
		sprintf(buffer, "%i%s", '0', h.c_str());
		h = buffer;
	}
	if (min.length() == 1)
	{
		sprintf(buffer, "%i%s", '0', min.c_str());
		min = buffer;
	}
	if (h.length() == 1)
	{
		sprintf(buffer, "%i%s", '0', sec.c_str());
		sec = buffer;
	}
	sprintf(buffer, "%s:%s:%s", h.c_str(), min.c_str(), sec.c_str());
	std::string ret = buffer;
	#endif
    
    free(a);
	return ret;
}

template<typename T>
void log(T value)
{
    std::cout << "[" << get_time() << "] " << value << std::endl;
}

template<typename T>
void log(T value, T value2)
{
    std::cout << "[" << get_time() << "] " << value << value2 << std::endl;
}

template<typename T>
void log_err(T value)
{
    std::cout <<"\x1B[91m" << "[" << get_time() << "] " << value << "\033[0m\t\t" << std::endl;
}

template<typename T, typename B>
void log_err(T value, B value2)
{
    std::cout <<"\x1B[91m" << "[" << get_time() << "] " << value << value2 <<"\033[0m\t\t" << std::endl;
}

template<typename T, typename B>
void log(T value, B value2, T value3)
{
    std::cout << "[" << get_time() << "] " << value << value2 << value3 << std::endl;
}

template<typename T, typename B>
void log(T value, B value2)
{
    std::cout << "[" << get_time() << "] " << value << value2 << std::endl;
}

void log_header()
{
	std::cout << "[" << get_time() << "] ";
}