#ifndef STEPS_H_INCLUDED 
#define STEPS_H_INCLUDED

#include <vector>
#include <string>

// CONSTANTS //////////////////////////////////////////////
const bool VERBOSE = true;
const int MAX_CHARS = 255;
const int MAX_URL_SIZE = MAX_CHARS;
const int BACKLOG = 1;
const int PORT = 55333;
const std::string default_filename("index.html");
const std::string test_file("https://www.wikipedia.org/portal/wikipedia.org/assets/img/Wikipedia-logo-v2.png");
const std::string DEFAULT_CHAINFILE("chaingang.txt");

// DATA ///////////////////////////////////////////////////
class FileRequest {

    std::string url;
    int num_ss;
    std::vector<std::string> chainlist;

    public:
        FileRequest (std::string new_url, int new_num_ss, std::vector<std::string> new_chainlist) {
            url = new_url;
            num_ss = new_num_ss;
            chainlist = new_chainlist;
        }

};

#endif
