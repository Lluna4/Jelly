#include <print>
#include <bitset>

int main()
{
    unsigned long val = ((10 & (unsigned long)0x3FFFFFF) << 38) | ((15 & (unsigned long)0x3FFFFFF) << 12) | (64 & (unsigned long)0xFFF);
    //val = be64toh(val);
    std::int32_t x = val >> 38;
    std::int32_t y = val << 52 >> 52;
    std::int32_t z = val << 26 >> 38;	

    std::println("{},{},{}", x, y, z);
}