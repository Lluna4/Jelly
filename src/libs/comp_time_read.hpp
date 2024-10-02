#pragma once

#include <concepts>
#include <cstring>
#include <tuple>
#include <stdlib.h>
#include "utils.hpp"

#define read_comp_pkt(size, ptr, t) const_for_<size>([&](auto i){std::get<i.value>(t) = read_var<std::tuple_element_t<i.value, decltype(t)>>::call(&ptr);});


template <typename T>
T read_type(char *v)
{
    T a;

    std::memcpy(&a, v, sizeof(T));
    switch (sizeof(T))
    {
        case 2:
            a = be16toh(a);
            break;
        case 4:
            a = be32toh(a);
            break;
        case 8:
            a = be64toh(a);
            break;
    }
    return a;
}

template<>
float read_type<float>(char *v)
{
    return read_float(v);
}

template<>
double read_type<double>(char *v)
{
    return read_double(v);
}

minecraft::varint read_varint(char *v)
{
    minecraft::varint a;

    a.size = ReadUleb128(v, &a.num);

    return a;
}


template<typename T>
struct read_var
{
    static T call(char** v)
    {
        T ret = read_type<T>(*v);
        *v += sizeof(T);
        return ret;
    }
};

template<>
struct read_var<minecraft::varint>
{
    static minecraft::varint call(char **v)
    {
        minecraft::varint ret = read_varint(*v);
        *v += ret.size;
        return ret;
    }
};

template<>
struct read_var<minecraft::string>
{
    static minecraft::string call(char **v)
    {
        minecraft::string ret; 
        ret.len = read_string(*v, ret.str);
        *v += ret.len;
        return ret;
    }
};


template <typename Integer, Integer ...I, typename F> constexpr void const_for_each_(std::integer_sequence<Integer, I...>, F&& func)
{
    (func(std::integral_constant<Integer, I>{}), ...);
}

template <auto N, typename F> constexpr void const_for_(F&& func)
{
    if constexpr (N > 0)
        const_for_each_(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
}

template<typename ...T>
std::tuple<T...> read_packet(std::tuple<T...> packet, char *buffer)
{
    constexpr std::size_t size = std::tuple_size_v<decltype(packet)>;
    read_comp_pkt(size, buffer, packet);
    return packet;
}