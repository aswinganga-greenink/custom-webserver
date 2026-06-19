#include "httprequest.hpp"
#include <sstream>

bool HttpRequest::parse(const std::string& raw_request){
    size_t request_line_end = raw_request.find("\r\n");
    if(request_line_end == std::string::npos){
        return false;
    }

    std::string request_line = raw_request.substr(0, request_line_end);


    std::istringstream iss(request_line);

    if (!(iss >> method >> uri >> version)) {
        return false;
    }

    if (version.length() < 5 || version.substr(0, 5) != "HTTP/") {
        return false; 
    }

    size_t header_start = request_line_end + 2;

    while( header_start < raw_request.size()) {
        size_t header_end = raw_request.find("\r\n", header_start);

        if(header_end == std::string::npos){
            break;
        }

        std::string line = raw_request.substr(header_start, header_end - header_start);


        if (line.empty()){
            break;
        }

        size_t colon_pos = line.find(':');
        if ( colon_pos != std::string::npos ) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            if( !value.empty() && value[0] == ' '){
                value.erase(0, 1);
            }
            headers[key] = value;
        }

        header_start = header_end + 2;


    }

    return true;

}