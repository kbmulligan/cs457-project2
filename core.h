#ifndef STEPS_H_INCLUDED 
#define STEPS_H_INCLUDED

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

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

const int MAX_CHUNK_SIZE = 65000;

class Chainlist;
class FileRequest;

// DATA ///////////////////////////////////////////////////
class FileRequest {

    std::string url;
    int num_ss;
    std::vector<std::string> chainlist;
    int sfd;

    public:
        FileRequest (std::string new_url, int new_num_ss, 
                     std::vector<std::string> new_chainlist, int socketfd) {
            url = new_url;
            num_ss = new_num_ss;
            chainlist = new_chainlist;
            sfd = socketfd;
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

        std::string get_url () {
            return url;
        }

        std::vector<std::string> * get_chainlist_ref () {
            return &chainlist;
        }

        int get_socket () {
            return sfd;
        }

};

class Chainlist {

    std::vector<std::string> chainlist;

    public:
        Chainlist (std::vector<std::string> newlist) {
            chainlist = newlist;
        }

        std::vector<std::string> get_chainlist () {
            return chainlist;
        }

        std::string get_chainlist_str () {
            std::string list;
            return list;
        }
};

class Chunk {

    char *data;
    int size;

    public:
        Chunk (void* raw_data, int sizeofdata) {
            size = sizeofdata;             // copy size
            data = (char *)malloc(size);   // allocate memory
            memcpy(data, raw_data, size);  // copy data
        }

        ~Chunk () {
            free(data);
        }

        int get_size () {
            return size;
        }        

        char* get_data () {
            return data;
        }

};

class FileTarget {

    std::string filename;
    std::vector<Chunk> chunks;
    std::ifstream ifs;

    public:
        FileTarget (std::string fn) {
            std::cout << "Create File object..." << std::endl;
            filename = fn;
            ifs.open(filename, std::ios::in | std::ios::binary); 
            std::cout << "Filename: " << filename << std::endl;

            if (ifs.good()) {
                std::cout << "File is good..." << std::endl;
            } else {
                std::cout << "File is bad..." << std::endl;
            }
        }

        ~FileTarget () {
            ifs.close();
        }
        
        std::vector<Chunk> get_chunks () {
            return chunks;
        }

        int get_num_chunks () {
            return chunks.size();
        }

        void chunkify () {
            ;
        }

        int get_size () {
            int size = 0;
            ifs.seekg(0, ifs.end);
            size = ifs.tellg();
            return size;
        }
};      



// FUNCTIONS //////////////////////////////////////////////
std::string get_ip (void);

int retrieve_file (std::string filename);
int read_request (int connectionfd);

short read_short (int connectionfd);
int send_short (int connectionfd, short data);
std::string read_string (int connectionfd, int string_len);
int send_string (int connectionfd, std::string str);

int step_to_next (FileRequest *req);

std::vector<std::string> pick_rand_ss(std::vector<std::string> chainlist);
std::vector<std::string> parse_socketpair (std::string raw_data, char delim);
std::vector<std::string> parse_chainlist (std::string raw_chain);
std::string read_chainfile (std::string filename);
std::vector<std::string> convert_delimiter (std::vector<std::string> chainlist, char old_delim, char new_delim);
std::string pack_chainlist (std::vector<std::string> vec_chain);
std::vector<std::string> pick_rand_ss (std::vector<std::string> *chainlist);

int connect_to_ss (std::vector<std::string> ss);

int wait_for_file (int sfd);

#endif
