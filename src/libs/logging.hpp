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
	return std::format("{:%OH:%OM:%OS}", std::chrono::system_clock::now());
}

template<typename T>
void log(T value)
{
    std::println("[{}] {}", get_time(), value);
}

template<typename T>
void log(T value, T value2)
{
    std::println("[{}] {}{}", get_time(), value, value2);
}

template<typename T>
void log_err(T value)
{
    std::println("\x1B[91m[{}] {}\033[0m\t\t", get_time(), value);
}

template<typename T, typename B>
void log_err(T value, B value2)
{
    std::println("\x1B[91m[{}] {}{}\033[0m\t\t", get_time(), value, value2);
}

template<typename T, typename B>
void log(T value, B value2, T value3)
{
    std::println("[{}] {}{}{}", get_time(), value, value2, value3);
}

template<typename T, typename B>
void log(T value, B value2)
{
    std::println("[{}] {}{}", get_time(), value, value2);
}

void log_header()
{
    std::print("[{}] ", get_time());
}
