#include <bitset>
#include <iostream>

int main()
{
    long aa = 1927182;
    std::bitset<64> fullBitset(aa);
    
    std::string string = fullBitset.to_string();

    long x = (long)std::bitset<26>(string.substr(0, 26)).to_ulong();
    long y = (long)std::bitset<26>(string.substr(25, 26)).to_ulong();
    long z = (long)std::bitset<26>(string.substr(51, 12)).to_ulong();
    std::cout << x << y << z << std::endl;
}