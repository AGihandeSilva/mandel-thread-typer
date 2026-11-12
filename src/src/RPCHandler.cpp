#include <RPCHandler.h>

#include <iostream>
#include <sstream>
#include <fstream>

RPCHandler::RPCHandler(uint16_t port, MandelbrotWidget& widget) :
	m_port(port), m_widget(widget) {}

std::string RPCHandler::LoadStringFromFile(std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open " << path << std::endl;
        abort();
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    return sstr.str();
}
