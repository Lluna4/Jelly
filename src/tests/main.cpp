#include "../libs/comp_time_read.hpp"
#include "../libs/comp_time_write.hpp"
#include <random>

int main()
{
    double lower = 0;
    double upper = 10000;
    std::uniform_real_distribution<double> dist(lower, upper);

    // Use std::mt19937 and seed it properly with std::random_device
    std::random_device rd;
    std::mt19937 rnd(rd());  // Seed only once with random_device

    char *buffer = (char *)calloc(1024, sizeof(char));
    char *buffer2 = (char *)calloc(1024, sizeof(char));
    char_size buff = {.data = buffer, .consumed_size = 0, .max_size = 1024, .start_data = buffer};
    char_size buff2 = {.data = buffer2, .consumed_size = 0, .max_size = 1024, .start_data = buffer2};
    std::tuple<double, double, double, bool> packet;
    std::tuple<double, double, double, bool> packet2;
    
    for (int i = 0; i < 20; i++)
    {
        // Generate random numbers using the same engine in every iteration
        double p1 = dist(rnd); 
        double p2 = 64; 
        double p3 = dist(rnd);

        packet = {p1, p2, p3, true};
        packet2 = {p1, p2, p3, false};
        constexpr std::size_t size = std::tuple_size_v<decltype(packet)>;
        constexpr std::size_t size2 = std::tuple_size_v<decltype(packet2)>;
        write_comp_pkt(size, buff, packet);
        write_comp_pkt(size, buff2, packet2);

        std::tuple<double, double, double, bool> pkt;
        std::tuple<double, double, double, bool> pkt2;
        pkt = read_packet(pkt, buff.start_data);
        pkt2 = read_packet(pkt2, buff2.start_data);

        std::println("First values  x {} y {} z {} bool {}", std::get<0>(pkt), std::get<1>(pkt), std::get<2>(pkt), std::get<3>(pkt));
        std::println("Second values x {} y {} z {} bool {}", std::get<0>(pkt2), std::get<1>(pkt2), std::get<2>(pkt2), std::get<3>(pkt2));
    }

    // Don't forget to free the allocated buffers
    free(buffer);
    free(buffer2);
}
