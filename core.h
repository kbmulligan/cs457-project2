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
const char IPPORT_DELIM = ':';
const char IPPORT_FILE_DELIM = ' ';
const char CHAINLIST_DELIM = ',';

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
        
        void print (void) {
            std::cout << "--- FILE REQUEST ---" << std::endl;
            std::cout << "URL: " << url << std::endl;
            std::cout << "Stepping stones: " << num_ss << std::endl;
            for (unsigned int i = 0; i < chainlist.size(); i++) {
                std::cout << chainlist[i] << std::endl;
            }
            std::cout << std::endl;
        }
};

// FUNCTIONS //////////////////////////////////////////////
std::string get_ip (void);

int retrieve_file (std::string filename);
int read_request (int connectionfd);

short read_short (int connectionfd);
int send_short (int connectionfd, short data);

int step_to_next (FileRequest *req);

std::vector<std::string> pick_rand_ss(std::vector<std::string> chainlist);
std::vector<std::string> parse_socketpair (std::string raw_data, char delim);
std::vector<std::string> parse_chainlist (std::string raw_chain);
std::string read_chainfile (std::string filename);
std::vector<std::string> convert_delimiter (std::vector<std::string> chainlist, char old_delim, char new_delim);
std::string pack_chainlist (std::vector<std::string> vec_chain);
std::vector<std::string> pick_rand_ss (std::vector<std::string> chainlist);

int connect_to_ss (std::vector<std::string> ss);


#endif