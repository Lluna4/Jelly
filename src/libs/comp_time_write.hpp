#pragma once
#include <sys/socket.h>
#include <concepts>
#include <cstring>
#include <tuple>
#include <bitset>
#include "utils.hpp"
#include "chunks.hpp"

#define write_comp_pkt(size, ptr, t) const_for<size>([&](auto i){write_var<std::tuple_element_t<i.value, decltype(t)>>::call(&ptr, std::get<i.value>(t));});

struct char_size
{
    char *data;
    int consumed_size;
    int max_size = 1024;
    char *start_data;
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
struct write_var<char_size>
{
    static void call(char_size *v, char_size value)
    {
        while (v->consumed_size >= v->max_size || v->consumed_size + value.consumed_size >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
            v->data = v->start_data + v->consumed_size;
        }
        std::memcpy(v->data, value.data, value.consumed_size);
        v->data += value.consumed_size;
        v->consumed_size += value.consumed_size;
    }
};

template<typename T>
struct write_var<std::vector<T>>
{
    static void call(char_size *v, std::vector<T> value)
    {
        for (auto val : value)
        {
            std::tuple<T> va = val;
            constexpr std::size_t size = std::tuple_size_v<decltype(va)>;
            write_comp_pkt(size, *v, va);
        }
    }
};

template<>
struct write_var<minecraft::bitset>
{
    static void call(char_size *v, minecraft::bitset value)
    {
        while (v->consumed_size >= v->max_size || v->consumed_size + (value.size.num * 8) >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->max_size + 1024);
            v->max_size += 1024;
            v->data = v->start_data + v->consumed_size;
        }
        std::tuple<minecraft::varint> head = {value.size};
        constexpr std::size_t size = std::tuple_size_v<decltype(head)>;
        write_comp_pkt(size, *v, head);
        for (auto val: value.bits)
        {
            unsigned long new_long = val.to_ulong();
            std::tuple<unsigned long> long_enc = {new_long};
            constexpr std::size_t size_ = std::tuple_size_v<decltype(long_enc)>;
            write_comp_pkt(size_, *v, long_enc);
        }
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
struct write_var<minecraft::chunk>
{
    static void call(char_size *v, minecraft::chunk value)
    {
        char *serialised_chunk = (char *)malloc(50000 * sizeof(char));
        char_size a = {.data = serialised_chunk, .consumed_size = 0, .max_size = 50000, .start_data = serialised_chunk};
        for (int i = 0; i < value.sections.size(); i++)
        {
            if (value.sections[i].blocks.type == SINGLE_VALUED)
            {
                std::tuple<short, unsigned char, minecraft::varint, minecraft::varint, char_size> data
                {
                    value.sections[i].block_count, value.sections[i].blocks.bits_per_entry,
                    value.sections[i].blocks.value, value.sections[i].blocks.data_size,
                    (char_size){.data = value.sections[i].blocks.data.get(), .consumed_size = 0,
                        .max_size = 1, .start_data = value.sections[i].blocks.data.get()}
                };
                constexpr std::size_t size = std::tuple_size_v<decltype(data)>;
                
                write_comp_pkt(size, a, data);
            }
            else if (value.sections[i].blocks.type == INDIRECT)
            {
                std::tuple<short, unsigned char, minecraft::varint, std::vector<minecraft::varint>, minecraft::varint> data
                {
                    value.sections[i].block_count, value.sections[i].blocks.bits_per_entry,
                    minecraft::varint(value.sections[i].blocks.palette.size()), value.sections[i].blocks.palette,
                    value.sections[i].blocks.data_size
                };
                constexpr std::size_t size = std::tuple_size_v<decltype(data)>;
               
                write_comp_pkt(size, a, data);
                char *long_dat = (char *)calloc(9, sizeof(char));
                char long_index = 0;
                for (int x = 0; x < 4096; x++)
                {
                    if (long_index == 8)
                    {
                        long new_long = 0;
                        std::memcpy(&new_long, long_dat, sizeof(long));
                        std::tuple<long> long_enc = {new_long};
                        constexpr std::size_t size_ = std::tuple_size_v<decltype(long_enc)>;
                        write_comp_pkt(size_, a, long_enc);
                        memset(long_dat, 0, sizeof(long));
                        long_index = 0;
                    }
                    long_dat[long_index] = value.sections[i].blocks.data[x];
                    long_index++;
                }
                long new_long = 0;
                std::memcpy(&new_long, long_dat, sizeof(long));
                std::tuple<long> long_enc = {new_long};
                constexpr std::size_t size_ = std::tuple_size_v<decltype(long_enc)>;
                write_comp_pkt(size_, a, long_enc);
                free(long_dat);
            }
            if (value.sections[i].biome.type == SINGLE_VALUED)
            {
                std::tuple<unsigned char, minecraft::varint, minecraft::varint, char_size> data
                {
                    value.sections[i].biome.bits_per_entry,
                    value.sections[i].biome.value, value.sections[i].biome.data_size,
                    (char_size){.data = value.sections[i].biome.data.get(), .consumed_size = 0,
                        .max_size = 1, .start_data = value.sections[i].biome.data.get()}
                };
                constexpr std::size_t size = std::tuple_size_v<decltype(data)>;
                
                write_comp_pkt(size, a, data);
            }
            else if (value.sections[i].biome.type == INDIRECT)
            {
                std::tuple<unsigned char, minecraft::varint, std::vector<minecraft::varint>, minecraft::varint, char_size> data
                {
                    value.sections[i].biome.bits_per_entry,minecraft::varint(value.sections[i].biome.palette.size()), 
                    value.sections[i].biome.palette,value.sections[i].biome.data_size,(char_size){.data = value.sections[i].biome.data.get(), 
                    .consumed_size = (int)value.sections[i].biome.data_size.num * 8,
                        .max_size = (int)value.sections[i].biome.data_size.num * 8 + 1, .start_data = value.sections[i].biome.data.get()}
                };
                constexpr std::size_t size = std::tuple_size_v<decltype(data)>;
               
                write_comp_pkt(size, a, data);
            }
        }
        if (v->consumed_size >= v->max_size || v->consumed_size + a.consumed_size >= v->max_size)
        {
            v->start_data = (char *)realloc(v->start_data, v->consumed_size + a.consumed_size + 1024);
            v->max_size = v->consumed_size + a.consumed_size + 1024;
            v->data = v->start_data + v->consumed_size;
        }
        std::tuple<minecraft::varint> chunk =
        {
            minecraft::varint(a.consumed_size), 
        };
        constexpr std::size_t size_ = std::tuple_size_v<decltype(chunk)>;
        write_comp_pkt(size_, *v, chunk);
        memcpy(v->data, a.start_data, a.consumed_size);
        v->consumed_size += a.consumed_size;
        v->data += a.consumed_size;
        free(a.start_data);
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
        std::memcpy(v->data, value.str.c_str(), len);
        v->data += len;
        v->consumed_size += len;
        std::memcpy(v->data, &zero, sizeof(char));
        v->data += sizeof(char);
        v->consumed_size += sizeof(char);
    }
};




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
    
    char *final_buffer = (char *)calloc(buff.max_size, sizeof(char));
    size_ = WriteUleb128(buf, buff.consumed_size);
    std::memcpy(final_buffer, buf.c_str(), size_);
    std::memcpy(&final_buffer[size_], buff.start_data, buff.consumed_size);

    int ret = send(sock, final_buffer, buff.consumed_size + size_, 0);
    std::println("Sent {}B", ret);
    free(buff.start_data);
    free(final_buffer);
    
    return ret;
}
