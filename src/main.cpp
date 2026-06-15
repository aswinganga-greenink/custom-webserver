#include <iostream>
#include "server.hpp"


int main(){
    std::cout << "Booting web server..." << std::endl;

    Server my_server_(8000);

    my_server_.start_server();

    return 0;

}