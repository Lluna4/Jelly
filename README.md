# Jelly
A simple 1.21/1.21.1 minecraft server for linux and macos made in c++23(using some c++26 features) thats in development!
## Build
To build the project you have to simply create the build directory `mkdir build && cd build` configure cmake `cmake ..` and then build the project `cmake --build .`, the project has dependencies with [nlohmann/json](https://github.com/nlohmann/json) and [packets](https://github.com/Mansitoh/Minecraft-DataRegistry-Packet-Generator) and openssl so please put nlohmann_json and Minecraft-DataRegistry-Packet-Generator in the same directory as the project
## Features
This project aims to be a simple minecraft server for me to learn but also to be as inclusive as possible, for now it has pronouns built in that can be changed with `/pronouns <pronouns>` but i also plan to provide support for plural systems in the future \
`/overwhelmed` is another command used when the user is overwhelmed, it mutes all the chat and its planned to make a harder version that makes so other people cant see you and you cant interact with you \
The project has simple threaded world generation with trees based on one noise map and an spline with all the noise values and block height
## Dependencies
I'm really glad that some amazing people created an awesome c++ library for json, without them this project would be impossible \
And also thank you nlohmann for making [nlohmann/json](https://github.com/nlohmann/json) <3 \
Thank you Reputeless for making a really simple and good perlin noise lib [Reputeless/PerlinNoise](https://github.com/Reputeless/PerlinNoise) <3 \
Thank you @mansitoh for the packets at [packets](https://github.com/Mansitoh/Minecraft-DataRegistry-Packet-Generator)

