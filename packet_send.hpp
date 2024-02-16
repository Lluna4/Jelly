#pragma once
#define VARINT_MAX 1
#define VARLONG_MAX 10
#define STRING_MAX 19
#include "user.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "stdlib.h"
#include <cstring>
#include "utils.hpp"
#include <string>
#include "logging.hpp"

int PKT_LOGIN_PLAY[]= {sizeof(int), sizeof(bool), VARINT_MAX, VARINT_MAX, STRING_MAX, VARINT_MAX, VARINT_MAX, VARINT_MAX, sizeof(bool), sizeof(bool), sizeof(bool), VARINT_MAX, STRING_MAX, VARINT_MAX, STRING_MAX, sizeof(long), sizeof(unsigned char), sizeof(char), sizeof(bool), sizeof(bool), sizeof(bool), VARINT_MAX};
const int PKT_LOGIN_PLAY_SIZE = 22;

enum PKT_TYPES {LOGIN_PLAY = 0};
enum PKT_VARIATIONS {DEFAULT = 0};

void write_varint(char **ptr, unsigned long val)
{
    std::string buf;
    WriteUleb128(buf, val);
    strcpy(*ptr, buf.c_str());
}

void free_all(char **mat, int type)
{
    if (type == LOGIN_PLAY)
    {
        for (int i = 0; i < PKT_LOGIN_PLAY_SIZE; i++)
        {
            free(mat[i]);
        }
    }
}

void free_to_n(char **mat, int n)
{
    for (int i = 0; i < n; i++)
    {
        free(mat[i]);
    }
}

unsigned long calc_mat_len(int type)
{
    int ret = 0;

    if (type == LOGIN_PLAY)
    {
        for (int i = 0; i < PKT_LOGIN_PLAY_SIZE; i++)
        {
            ret += PKT_LOGIN_PLAY[i];
        }
    }
    ret = ret + 1;
    return ret;
}

void login_play_default(char **mat)
{
    bool b = false;
    long seed = 123456;
    unsigned char mode = 3;
    char prev = -1;
    int id = 0;
    memcpy(mat[0], &id, sizeof(int));
    memcpy(mat[1], &b, sizeof(bool));
    write_varint(&mat[2], 1);
    write_varint(&mat[3], strlen("minecraft:overworld"));
    strcpy(mat[4], "minecraft:overworld");
    write_varint(&mat[5], 1);
    write_varint(&mat[6], 1);
    write_varint(&mat[7], 1);
    memcpy(mat[8], &b, sizeof(bool));
    memcpy(mat[9], &b, sizeof(bool));
    memcpy(mat[10], &b, sizeof(bool));
    write_varint(&mat[11], strlen("minecraft:overworld"));
    strcpy(mat[12], "minecraft:overworld");
    write_varint(&mat[13], strlen("minecraft:overworld"));
    strcpy(mat[14], "minecraft:overworld");
    memcpy(mat[15], &seed, sizeof(bool));
    memcpy(mat[16], &mode, sizeof(unsigned char));
    memcpy(mat[17], &prev, sizeof(char));
    memcpy(mat[18], &b, sizeof(bool));
    memcpy(mat[19], &b, sizeof(bool));
    memcpy(mat[20], &b, sizeof(bool));
    write_varint(&mat[21], 0);
}

void alloc_and_send(int type, int variation, User user)
{
    unsigned long size = 0;
    unsigned long id = 0;
    if (type == LOGIN_PLAY)
    {
        id = 0x29;
        size = calc_mat_len(LOGIN_PLAY);
        char **mat = (char **)calloc(PKT_LOGIN_PLAY_SIZE, sizeof(char *));
        std::string buf;
        if (!mat)
        {
            log_err("Allocation error of size: ", PKT_LOGIN_PLAY_SIZE * sizeof(char *));
            free(mat);
            return;
        }
        for (int i = 0; i < PKT_LOGIN_PLAY_SIZE; i++)
        {
            mat[i] = (char *)calloc(PKT_LOGIN_PLAY[i], sizeof(char));
            if (!mat[i])
            {
                log_err("Allocation error of size: ", PKT_LOGIN_PLAY[i] * sizeof(char));
                free_to_n(mat, i);
                free(mat);
                return;
            }
        }
        if (variation == DEFAULT)
        {
            login_play_default(mat);
        }
        WriteUleb128(buf, size);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        buf.clear();
        WriteUleb128(buf, id);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        for (int i = 0; i < PKT_LOGIN_PLAY_SIZE; i++)
        {
            send(user.get_socket(), mat[i], PKT_LOGIN_PLAY[i], 0);
        }
        free_all(mat, LOGIN_PLAY);
        free(mat);
    }
}
