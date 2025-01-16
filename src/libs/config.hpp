#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "tokenize.hpp"
#include <format>

int SV_PORT = 25565;
std::string SV_IP = "0.0.0.0";
std::string motd = "A Minecraft Server";
std::string icon_path = "favicon.png";
int sv_render_distance = 8;

bool isNumber(std::string a)
{
	if (a.empty())
		return false;
	for (int i = 0; i < a.length(); i++)
	{
		if (isdigit(a[i]) == 0)
			return false;
	}
	return true;
}

bool check_ip(char* ip)
{
	std::string buff;
	std::vector<std::string> tokens;
	if (strcmp(ip, "localhost") == 0)
		return true;
	buff = ip;
	tokens = tokenize(ip, '.');
	if (tokens.size() != 4)
		return false;
	for (int i = 0; i < tokens.size(); i++)
	{
		if (isNumber(tokens[i]) == false)
			return false;
		else if (atoi(tokens[i].c_str()) > 255)
			return false;
	}
	return true;
}

void create_config()
{
    std::ofstream cfg("server.properties");
    cfg << "#Minecraft server config\n";
    cfg << "server-ip=\n";
    cfg << "server-port=25565\n";
    cfg << "motd=A Minecraft Server\n";
    cfg << "icon-path=favicon.png\n";
    cfg << "server-render-distance=8\n";
    cfg.close();
}

void load_config()
{
    std::ifstream infile("server.properties");
    std::string line;
    while (std::getline(infile, line))
    {
        if (line.starts_with("#") == false)
        {
            std::vector<std::string> tokens = tokenize(line, '=');
            if (tokens[0].compare("server-ip") == 0)
            {
                if (tokens.size() > 1 && check_ip((char *)tokens[1].c_str()) == true)
                    SV_IP = tokens[1];
            }
            else if (tokens[0].compare("server-port") == 0)
            {
                if (tokens.size() > 1 && isNumber(tokens[1]) == true)
                {
                    SV_PORT = atoi(tokens[1].c_str());
                }
            }
            else if (tokens[0].compare("motd") == 0)
            {
                if (tokens.size() > 1)
                {
                    motd = tokens[1];
                }
            }
            else if (tokens[0].compare("icon-path") == 0)
            {
                if (tokens.size() > 1)
                {
                    icon_path = tokens[1];
                }
            }
            else if (tokens[0].compare("server-render-distance") == 0)
            {
                if (tokens.size() > 1)
                {
                    sv_render_distance = std::atoi(tokens[1].c_str());
                }
            }
        }
    }
    infile.close();
}