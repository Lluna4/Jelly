#include <iostream>
#include <string>
#include <map>
#include <any>
#include <typeinfo>
#include <stdexcept>

class PacketWrapper {
public:
    // Constructor to initialize the map
    PacketWrapper(const std::map<std::string, std::any>& pkt) : pkt(pkt) {}

    // Templated get function to perform casting
    template<typename T>
    T get(const std::string& key) const {
        auto it = pkt.find(key);
        if (it != pkt.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast& e) {
                throw std::runtime_error("Bad any_cast for key: " + key + ", expected type: " + typeid(T).name());
            }
        } else {
            throw std::runtime_error("Key not found: " + key);
        }
    }

private:
    std::map<std::string, std::any> pkt;
};

// Dummy pkt_read function for demonstration
std::map<std::string, std::any> pkt_read(/* packet type */) {
    // Dummy implementation
    std::map<std::string, std::any> pkt;
    pkt["X"] = 1.0;
    pkt["Y"] = 2.0;
    pkt["Z"] = 3.0;
    pkt["Ground"] = true;
    return pkt;
}

int main() {
    // Assuming pkt_read function returns the packet as a map
    auto raw_pkt = pkt_read(/* packet data */);

    // Wrap the raw packet map
    PacketWrapper pkt(raw_pkt);

    // Use the wrapper to access and automatically cast values
    try {
        // Explicitly specify the type when using the get method
        double x = pkt.get<double>("X");
        double y = pkt.get<double>("Y");
        double z = pkt.get<double>("Z");
        bool ground = pkt.get<bool>("Ground");

        std::cout << "X: " << x << "\nY: " << y << "\nZ: " << z << "\nGround: " << std::boolalpha << ground << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

