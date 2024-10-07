#pragma once
#include <sys/socket.h>
#include <concepts>
#include <cstring>
#include <tuple>
#include "utils.hpp"
#include "world_gen.hpp"

#define write_comp_pkt(size, ptr, t) const_for<size>([&](auto i){write_var<std::tuple_element_t<i.value, decltype(t)>>::call(&ptr, std::get<i.value>(t));});

struct char_size
{
    char *data;
    int consumed_size;
    int max_size = 1024;
    char *start_data;
};

template <typename T>
void write_type(char *v, T value)
{
    switch (sizeof(T))
    {
        case 2:
        {
            uint16_t conv = htobe16((*(uint16_t *)&value));
            std::memcpy(v, &conv, sizeof(T));
            break;
        }
        case 4:
        {
            uint32_t conv = htobe32((*(uint32_t *)&value));
            std::memcpy(v, &conv, sizeof(T));
            break;
        }
        case 8:
        {
            uint64_t conv = htobe64((*(uint64_t *)&value));
            std::memcpy(v, &conv, sizeof(T));
            break;
        }
        default:
            std::memcpy(v, &value, sizeof(T));
    }
}

template <int size, typename T>
void write_array(char *v, T value)
{
    std::memcpy(v, value, size);
}


template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

template<typename T>
concept IsChar = std::same_as<T, char *>;

template<typename T>
concept IsPointer = std::is_pointer_v<T>;

template<typename T>
struct write_var
{
    static void call(char_size *v, T value) requires (arithmetic<T>)
    {
        if (v->consumed_size >= v->max_size || v->consumed_size + sizeof(T) >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
        }
        write_type<T>(v->data, value);
        v->data += sizeof(T);
        v->consumed_size += sizeof(T);
    }
};

template<typename ...T>
struct write_var<std::tuple<T...>>
{
    static void call(char_size *v, std::tuple<T...> value)
    {
        constexpr std::size_t size = std::tuple_size_v<decltype(value)>;
        write_comp_pkt(size, *v, value);
    }
};

template<>
struct write_var<minecraft::varint>
{
    static void call(char_size *v, minecraft::varint value)
    {
        if (v->consumed_size >= v->max_size || v->consumed_size + 5 >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
        }
        std::string dest;
        size_t size = WriteUleb128(dest, value.num);
        std::memcpy(v->data, dest.c_str(), size);
        v->data += size;
        v->consumed_size += size;
    }
};

template<>
struct write_var<minecraft::string>
{
    static void call(char_size *v, minecraft::string value)
    {
        while (v->consumed_size >= v->max_size || v->consumed_size + value.len >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
            v->data = v->start_data + v->consumed_size;
        }
        std::string a;
        size_t size = write_string(a, value.str);
        memcpy(v->data, a.c_str(), value.len + size);
        v->data += value.len + size;
        v->consumed_size += value.len + size;
    }
};

template<>
struct write_var<minecraft::uuid>
{
    static void call(char_size *v, minecraft::uuid value)
    {
        while (v->consumed_size >= v->max_size || v->consumed_size + value.len >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
            v->data = v->start_data + v->consumed_size;
        }
        memcpy(v->data, value.buff, 16);
        v->data += 16;
        v->consumed_size += 16;
    }
};

template<>
struct write_var<minecraft::chunk_rw>
{
    static void call(char_size *v, minecraft::chunk_rw value)
    {
        while (v->consumed_size >= v->max_size || v->consumed_size + value.size() >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
            v->data = v->start_data + v->consumed_size;
        }
        
        auto chun = value;
        for (int x = 0; x < chun.chunks.size(); x++)
        {
            chun.chunks[x].block_count = htobe16(*(uint16_t*)&chun.chunks[x].block_count);
            std::memcpy(v->data, &chun.chunks[x].block_count, sizeof(short));
            v->data += sizeof(short);
            minecraft::paletted_container_rw cont = chun.chunks[x].blocks;

            std::memcpy(v->data, &cont.bits_per_entry, sizeof(unsigned char));
            v->data += sizeof(unsigned char);
            std::string dest;
            size_t size = WriteUleb128(dest, cont.palette_data_entries.num);
            std::memcpy(v->data, dest.c_str(), size);
            v->data += size;
            for (int i = 0; i < cont.block_ids.size(); i++)
            {
                dest.clear();
                size = WriteUleb128(dest, cont.block_ids[i].num);
                std::memcpy(v->data, dest.c_str(), size);
                v->data += size;
            }   
            dest.clear();
            size = WriteUleb128(dest, cont.data_lenght.num);
            std::memcpy(v->data, dest.c_str(), size);
            v->data += size;

            for (int i = 0; i < cont.block_indexes.size(); i++)
            {
                unsigned long conv = htobe64((*(uint64_t *)&cont.block_indexes[i]));
                std::memcpy(v->data, &conv, sizeof(unsigned long));
                v->data += sizeof(unsigned long);
            }
            
            minecraft::paletted_container_rw cont2 = chun.chunks[x].biome;

            std::memcpy(v->data, &cont2.bits_per_entry, sizeof(unsigned char));
            v->data += sizeof(unsigned char);
            //send_varint(fd, cont2.palette_data_entries.num);
            dest.clear();
            size = WriteUleb128(dest, cont2.block_ids[0].num);
            std::memcpy(v->data, dest.c_str(), size);
            v->data += size;

            dest.clear();
            size = WriteUleb128(dest, cont2.data_lenght.num);
            std::memcpy(v->data, dest.c_str(), size);
            v->data += size;

            for (int i = 0; i < cont2.block_indexes.size(); i++)
            {
                std::memcpy(v->data, &cont2.block_indexes[i], sizeof(long));
                v->data += sizeof(long);
            }
        }
        v->consumed_size += value.size();
    }
};

template<>
struct write_var<minecraft::string_tag>
{
    static void call(char_size *v, minecraft::string_tag value)
    {
        std::string buf;
        char id = 0x08;
        char comp = 0x0a;
        std::string text = "text";
        short lenght = 4;
        char zero = 0;
        short len = value.len;
        short len2 = lenght;
        lenght = htobe16(*(uint16_t*)&lenght);
        value.len = htobe16(*(uint16_t*)&value.len);
        
        std::memcpy(v->data, &comp, sizeof(char));
        v->data += sizeof(char);
        v->consumed_size += sizeof(char);
        std::memcpy(v->data, &id, sizeof(char));
        v->data += sizeof(char);
        v->consumed_size += sizeof(char);
        std::memcpy(v->data, &lenght, sizeof(short));
        v->data += sizeof(short);
        v->consumed_size += sizeof(short);
        std::memcpy(v->data, text.c_str(), len2);
        v->data += len2;
        v->consumed_size += len2;
        std::memcpy(v->data, &value.len, sizeof(short));
        v->data += sizeof(short);
        v->consumed_size += sizeof(short);
        std::memcpy(v->data, value.string.c_str(), len);
        v->data += len;
        v->consumed_size += len;
        std::memcpy(v->data, &zero, sizeof(char));
        v->data += sizeof(char);
        v->consumed_size += sizeof(char);
    }
};


template <typename Integer, Integer ...I, typename F> constexpr void const_for_each(std::integer_sequence<Integer, I...>, F&& func)
{
    (func(std::integral_constant<Integer, I>{}), ...);
}

template <auto N, typename F> constexpr void const_for(F&& func)
{
    if constexpr (N > 0)
        const_for_each(std::make_integer_sequence<decltype(N), N>{}, std::forward<F>(func));
}    

template<typename ...T>
int send_packet(std::tuple<T...> packet, int sock)
{
    char *buffer = (char *)malloc(1024 * sizeof(char));
    char *start_buffer = buffer;
    std::string buf;
    size_t size_ = 0;
    constexpr std::size_t size = std::tuple_size_v<decltype(packet)>;
    char_size buff = {buffer, 0, 1024, start_buffer};
    write_comp_pkt(size, buff, packet);
    
    char *final_buffer = (char *)calloc(buff.consumed_size + size_, sizeof(char));
    size_ = WriteUleb128(buf, buff.consumed_size);
    std::memcpy(final_buffer, buf.c_str(), size_);
    std::memcpy(&final_buffer[size_], buff.start_data, buff.consumed_size);

    int ret = send(sock, final_buffer, buff.consumed_size + size_, 0);
    std::println("Sent {}B", ret);
    free(buff.start_data);
    free(final_buffer);
    
    return ret;
}
