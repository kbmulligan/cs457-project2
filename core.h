#ifndef STEPS_H_INCLUDED 
#define STEPS_H_INCLUDED

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

// CONSTANTS //////////////////////////////////////////////
const bool VERBOSE = false;
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
const int MAX_FAILS = 100;
const int MAX_CHUNK_SIZE = 1024;

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
    //std::vector<Chunk> chunks;
    std::ifstream ifs;
    char *data;
    int chunk_size;

    public:
        FileTarget (std::string fn) {

            filename = fn;
            ifs.open(filename.c_str(), std::fstream::in | std::fstream::binary); 

            if (VERBOSE) {
                std::cout << "Create File object..." << std::endl;
                std::cout << "Filename: " << filename << std::endl;
            }

            if (ifs.good()) {
            
                if (VERBOSE) {
                    std::cout << "File is good..." << std::endl;
                }

                chunk_size = std::min(MAX_CHUNK_SIZE, get_size());
                read_file();                                       // get into memory

            } else {
                std::cout << "File is bad..." << std::endl;
            }
        }

        ~FileTarget () {
            ifs.close();
            free(data);
        }
        
        std::string get_filename () {
            return filename;
        }


        void print () {
            std::cout << "--- FileTarget ---" << std::endl;
            std::cout << "Filename: " << filename << std::endl;
            std::cout << "File stream good: " << ifs.good() << std::endl;
            std::cout << "File size: " << get_size() << std::endl;
            std::cout << "File chunk size: " << chunk_size << std::endl;
            std::cout << "File chunks: " << get_num_chunks() << std::endl;
            
            //for (unsigned int i = 0; i < chunks.size(); i++) {
            //    std::cout << "Chunk " << i << ": size " << chunks[i].get_size() << std::endl;
            //}
        }

        /*
        void chunkify () {

            std::cout << "Chunking file: " << filename << "(" << get_size() << " bytes)" << std::endl;
            
            while (ifs.tellg() < get_size()) {
                
                std::cout << "Adding chunk..." << std::endl;

                char *buffer = (char *)malloc(MAX_CHUNK_SIZE);
                ifs.read(buffer, MAX_CHUNK_SIZE); 

                chunks.push_back( Chunk(buffer, ifs.gcount()) );                 // add chunk
                

                //free(buffer);

                std::cout << "Adding chunk...complete" << std::endl;

                std::cout << "ifs.tellg() : " << ifs.tellg() << std::endl;
            }

            print();
        }
        */

        void read_file() {
            int size = get_size();
            data = (char *)malloc(size);
            ifs.read(data, size); 
        }

        int get_num_chunks () {
            int num_chunks = get_size() / chunk_size;
            if (num_chunks * chunk_size < get_size()) {
                num_chunks++;
            }
            return num_chunks;
        }

        int get_chunk_size(int chunk) {                    // chunk size different for last chunk
            int this_chunk_size = chunk_size;

            if (chunk >= get_num_chunks() - 1) {
                this_chunk_size = get_size() - ((get_num_chunks() - 1) * chunk_size);
            }

            return this_chunk_size;
        }

        int get_size () {
            int loc = ifs.tellg();     // record current marker location

            ifs.seekg(0, ifs.end);     // seek all the way to the EOF
            int size = ifs.tellg();

            ifs.seekg(loc);            // reset get marker
            return size;
        }

        char* get_data () {
            return data;
        }
};      



// FUNCTIONS //////////////////////////////////////////////
std::string get_ip (void);

int retrieve_file (std::string filename);
int read_request (int connectionfd);

short read_short (int connectionfd);
int send_short (int connectionfd, short data);
long read_long (int connectionfd);
int send_long (int connectionfd, long data);
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

int wait_for_file (FileRequest* req, int sfd);

int receive_packet (int sfd);

int write_file (char* buffer, int buffer_size, std::string fn);
int transmit_packet (int sfd, char* data, int size);

std::string local_filename (std::string filename);
void print_chainlist(std::vector<std::string> chain);

#endif
