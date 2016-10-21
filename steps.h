#ifndef STEPS_H_INCLUDED 
#define STEPS_H_INCLUDED

#include <vector>
#include <string>

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
