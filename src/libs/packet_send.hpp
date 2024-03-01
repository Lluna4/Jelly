#pragma once
#define VARINT_MAX 1
#define VARLONG_MAX 10
#define STRING_MAX 19
#define POSITION_SIZE 8
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

int PKT_SET_SPAWN[] = {POSITION_SIZE, sizeof(float)};
const int PKT_SET_SPAWN_SIZE = 2;

int PKT_SYNC_POS[] = {sizeof(double), sizeof(double), sizeof(double), sizeof(float), sizeof(float), sizeof(char), VARINT_MAX};
const int PKT_SYNC_POS_SIZE = 7;

int PKT_GAME_EVENT_13[] = {sizeof(unsigned char), sizeof(float)};
const int PKT_GAME_EVENT_13_SIZE = 2;

int PKT_GAME_EVENT_3[] = {sizeof(unsigned char), sizeof(float)};
const int PKT_GAME_EVENT_3_SIZE = 2;

enum PKT_TYPES {LOGIN_PLAY = 0, SET_SPAWN = 1, SYNC_POS = 2, GAME_EVENT_13 = 3, GAME_EVENT_3 = 4};
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
    else if (type == SET_SPAWN)
    {
        for (int i = 0; i < PKT_SET_SPAWN_SIZE; i++)
        {
            ret += PKT_SET_SPAWN[i];
        }  
    }
    else if (type == SYNC_POS)
    {
        for (int i = 0; i < PKT_SYNC_POS_SIZE; i++)
        {
            ret += PKT_SYNC_POS[i];
        }  
    }
    else if (type == GAME_EVENT_13)
    {
        for (int i = 0; i < PKT_GAME_EVENT_13_SIZE; i++)
        {
            ret += PKT_GAME_EVENT_13[i];
        }  
    }
    ret = ret + 1;
    return ret;
}

void login_play_default(char **mat)
{
    bool b = false;
    long seed = 0;
    unsigned char mode = 3;
    char prev = 1;
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
    memcpy(mat[15], &seed, sizeof(long));
    memcpy(mat[16], &mode, sizeof(unsigned char));
    memcpy(mat[17], &prev, sizeof(char));
    memcpy(mat[18], &b, sizeof(bool));
    memcpy(mat[19], &b, sizeof(bool));
    memcpy(mat[20], &b, sizeof(bool));
    write_varint(&mat[21], 0);
}

void set_spawn_default(char **mat)
{
    long x, y, z = 0;
    y = 200;
    long long res = ((x & 0x3FFFFFF) << 38) | ((z & 0x3FFFFFF) << 12) | (y & 0xFFF);
    float angle = 0.0f;
    memcpy(mat[0], &res, sizeof(long long));
    memcpy(mat[1], &angle, sizeof(float));
}

void sync_pos(char **mat, User user)
{
    struct position pos = user.get_position();
    
    char flag = 0;
    unsigned long id = 0;

    memcpy(mat[0], &pos.x, sizeof(double));
    memcpy(mat[1], &pos.y, sizeof(double));
    memcpy(mat[2], &pos.z, sizeof(double));
    memcpy(mat[3], &pos.yaw, sizeof(float));
    memcpy(mat[4], &pos.pitch, sizeof(float));
    memcpy(mat[5], &flag, sizeof(char));
    write_varint(&mat[6], id);
}

void game_event_13(char **mat)
{
    unsigned char event = 13;
    float val = 0.0f;
    memcpy(mat[0], &event, sizeof(unsigned char));
    memcpy(mat[1], &val, sizeof(float));
}

void game_event_3(char **mat)
{
    unsigned char event = 3;
    float val = 0.0f;
    memcpy(mat[0], &event, sizeof(unsigned char));
    memcpy(mat[1], &val, sizeof(float));
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
    else if (type == SET_SPAWN)
    {
        id = 0x54;
        size = calc_mat_len(SET_SPAWN);
        char **mat = (char **)calloc(PKT_SET_SPAWN_SIZE, sizeof(char *));
        std::string buf;
        if (!mat)
        {
            log_err("Allocation error of size: ", PKT_SET_SPAWN_SIZE * sizeof(char *));
            free(mat);
            return;
        }
        for (int i = 0; i < PKT_SET_SPAWN_SIZE; i++)
        {
            mat[i] = (char *)calloc(PKT_SET_SPAWN[i], sizeof(char));
            if (!mat[i])
            {
                log_err("Allocation error of size: ", PKT_SET_SPAWN[i] * sizeof(char));
                free_to_n(mat, i);
                free(mat);
                return;
            }
        }
        if (variation == DEFAULT)
        {
            set_spawn_default(mat);
        }
        WriteUleb128(buf, size);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        buf.clear();
        WriteUleb128(buf, id);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        for (int i = 0; i < PKT_SET_SPAWN_SIZE; i++)
        {
            send(user.get_socket(), mat[i], PKT_SET_SPAWN[i], 0);
        }
        free_all(mat, SET_SPAWN);
        free(mat);
    }
    else if (type == SYNC_POS)
    {
        id = 0x3E;
        size = calc_mat_len(SYNC_POS);
        char **mat = (char **)calloc(PKT_SYNC_POS_SIZE, sizeof(char *));
        std::string buf;
        if (!mat)
        {
            log_err("Allocation error of size: ", PKT_SYNC_POS_SIZE * sizeof(char *));
            free(mat);
            return;
        }
        for (int i = 0; i < PKT_SYNC_POS_SIZE; i++)
        {
            mat[i] = (char *)calloc(PKT_SYNC_POS[i], sizeof(char));
            if (!mat[i])
            {
                log_err("Allocation error of size: ", PKT_SYNC_POS[i] * sizeof(char));
                free_to_n(mat, i);
                free(mat);
                return;
            }
        }
        if (variation == DEFAULT)
        {
            sync_pos(mat, user);
        }
        WriteUleb128(buf, size);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        buf.clear();
        WriteUleb128(buf, id);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        for (int i = 0; i < PKT_SYNC_POS_SIZE; i++)
        {
            send(user.get_socket(), mat[i], PKT_SYNC_POS[i], 0);
        }
        free_all(mat, SYNC_POS);
        free(mat);
    }
    else if (type == GAME_EVENT_13)
    {
        id = 0x20;
        size = calc_mat_len(GAME_EVENT_13);
        char **mat = (char **)calloc(PKT_GAME_EVENT_13_SIZE, sizeof(char *));
        std::string buf;
        if (!mat)
        {
            log_err("Allocation error of size: ", PKT_GAME_EVENT_13_SIZE * sizeof(char *));
            free(mat);
            return;
        }
        for (int i = 0; i < PKT_GAME_EVENT_13_SIZE; i++)
        {
            mat[i] = (char *)calloc(PKT_GAME_EVENT_13[i], sizeof(char));
            if (!mat[i])
            {
                log_err("Allocation error of size: ", PKT_GAME_EVENT_13[i] * sizeof(char));
                free_to_n(mat, i);
                free(mat);
                return;
            }
        }
        if (variation == DEFAULT)
        {
            game_event_13(mat);
        }
        WriteUleb128(buf, size);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        buf.clear();
        WriteUleb128(buf, id);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        for (int i = 0; i < PKT_GAME_EVENT_13_SIZE; i++)
        {
            send(user.get_socket(), mat[i], PKT_GAME_EVENT_13[i], 0);
        }
        free_all(mat, GAME_EVENT_13);
        free(mat);
    }
    else if (type == GAME_EVENT_3)
    {
        id = 0x20;
        size = calc_mat_len(GAME_EVENT_13);
        char **mat = (char **)calloc(PKT_GAME_EVENT_13_SIZE, sizeof(char *));
        std::string buf;
        if (!mat)
        {
            log_err("Allocation error of size: ", PKT_GAME_EVENT_13_SIZE * sizeof(char *));
            free(mat);
            return;
        }
        for (int i = 0; i < PKT_GAME_EVENT_13_SIZE; i++)
        {
            mat[i] = (char *)calloc(PKT_GAME_EVENT_13[i], sizeof(char));
            if (!mat[i])
            {
                log_err("Allocation error of size: ", PKT_GAME_EVENT_13[i] * sizeof(char));
                free_to_n(mat, i);
                free(mat);
                return;
            }
        }
        if (variation == DEFAULT)
        {
            game_event_3(mat);
        }
        WriteUleb128(buf, size);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        buf.clear();
        WriteUleb128(buf, id);
        send(user.get_socket(), buf.c_str(), buf.length(), 0);
        for (int i = 0; i < PKT_GAME_EVENT_13_SIZE; i++)
        {
            send(user.get_socket(), mat[i], PKT_GAME_EVENT_13[i], 0);
        }
        free_all(mat, GAME_EVENT_13);
        free(mat);
    }
}
