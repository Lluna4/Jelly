#pragma once
#include <string>
#include <ctime>
#include <print>
#include <iostream>
#include <chrono>
#if __has_include (<format>) && __cplusplus >= 202002L
	#include <format>
#else
	#include <stdio.h>
	#define NO_FORMAT
	#pragma warn("No <format>")
#endif

enum SCALE
{
	INFO,
	WARN,
	ERROR
};

std::string file_name;

void generate_file_name()
{
	file_name = std::format("logs/log_{:%Od-%Om-%Oy_%OH-%OM-%OS}.txt", std::chrono::system_clock::now());
	std::println("File name {}", file_name);
}

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
	return std::format("{:%OH:%OM:%OS}", std::chrono::system_clock::now());
}

void write_to_file(std::string log_msg)
{
	std::ofstream file;
	file.open(file_name, std::ios::out | std::ios::app);
	file << log_msg << std::endl;
	file.close();
}

template<typename T>
void log(T value)
{
	std::string log_msg = std::format("[{}] {}", get_time(), value);
    std::println("[{}] {}", get_time(), value);
	write_to_file(log_msg);
}

template<typename T>
void log(T value, int scale)
{
	switch (scale)
	{
	case INFO:
	{
		std::string log_msg = std::format("[{}] INFO: {}", get_time(), value);
		std::println("[{}] INFO: {}", get_time(), value);
		write_to_file(log_msg);
		break;
	}
	case WARN:
	{
		std::string log_msg = std::format("[{}] WARN: {}", get_time(), value);
		std::println("\e[0;33m[{}] WARN: {}\e[0m", get_time(), value);
		write_to_file(log_msg);
	}
	case ERROR:
	{
		std::string log_msg = std::format("[{}] ERROR: {}", get_time(), value);
		std::println("\e[0;31m[{}] ERROR: {}\e[0m", get_time(), value);
		write_to_file(log_msg);
	}
	default:
		break;
	}

}
