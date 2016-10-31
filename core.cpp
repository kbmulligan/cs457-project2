// core.cpp - core of the stepping stone program listens,
//          transfers file via addresses in chainlist
// Author: K. Brett Mulligan
// Date: Oct 2016
// CSU - Comp Sci
// CS457 - Networks
// Dr. Indrajit Ray
// License: MIT
// References : Beej's Guide to Socket Programming
//              https://beej.us/guide/bgnet/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>

#include "core.h"

using namespace std;


// FUNCTIONS //////////////////////////////////////////////


string get_ip () {
 
    string ip;

    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    char hostname[64];

    gethostname(hostname, 64);
    
    //cout << "Getting hostname : " << hostname << endl;
    status = getaddrinfo(hostname,  NULL, &hints, &res); 
    if (status != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return string("ERROR: check cerr");
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        string ipver;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = string("IPv4");
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = string("IPv6");
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        //cout << ipver << " : " << ipstr << endl;

    }

    freeaddrinfo(res);

    ip = string(ipstr);

    return ip;
}

short read_short (int connectionfd) { 
    short data = 0;
    int bytes_received = recv(connectionfd, &data, sizeof(data), 0);
    if (bytes_received == -1) {
        cout << "read_short: recv returned -1" << endl;
    }
    return ntohs(data);
}

int send_short (int connectionfd, short data) { 
    short net_data = htons(data);
    int bytes_sent = send(connectionfd, &net_data, sizeof(net_data), 0);
    if (bytes_sent == -1) {
        cout << "send_short: send returned -1" << endl;
    }
    return 0; 
}

// reads string_legnth bytes of data from socket connectionfd and returns it as string
string read_string (int connectionfd, int string_length) {
   
    cout << "Reading string... of length: " << string_length << " on socket: " << connectionfd << endl; 

    char buffer[string_length];
    int status = read(connectionfd, buffer, string_length); 
    if (status < string_length) {
       cout << "read_string didn't read it all! read: " << status << " bytes" << endl; 
    }
    string new_string(buffer);
    return new_string;
}

int send_string (int connectionfd, string str) {

    cout << "Sending string... of length: " << str.size() << " on socket: " << connectionfd << endl; 

    int flags = 0;
    char string_data[str.size()];
    strcpy(string_data, str.c_str()); // copy str into string_data location

    int status = send(connectionfd, &string_data, str.size(), flags);
    int size = str.size();
    if ( status == -1 || (status != size) ) {
        cout << "send_string: send failed, status: " << status << endl;
    }
    return 0;
}

string pack_chainlist (vector<string> vec_chain) {
    string chainlist = "";
    
    if (!vec_chain.empty()) {
        chainlist += vec_chain[0];

        for (unsigned int i = 1; i < vec_chain.size(); i++) {
            chainlist += CHAINLIST_DELIM;
            chainlist += vec_chain[i]; 
        }
    } else {
        cout << "Empty chainlist" << endl;
    }
    return chainlist;
}

// takes raw string of ip's and ports delimited by commas and converts it to a vector to strings
vector<string> parse_chainlist (string raw_chain) {

    vector<string> chainlist;

    stringstream streamlist(raw_chain);
    char delim = CHAINLIST_DELIM;

    string item;

    while(getline(streamlist, item, delim)) {
        chainlist.push_back(item);
    }

    return chainlist;
}

// takes ip:port and splits ip from port 
vector<string> parse_socketpair (string raw_data, char delim) {

    vector<string> socketpair;

    stringstream streamlist(raw_data);

    string item;

    //cout << "Iterating through socket pair..." << endl;

    while(getline(streamlist, item, delim)) {
        socketpair.push_back(item);
    }
 
    cout << "parse_socketpair: " << socketpair[0] << " " << socketpair[1] << endl;    
    return socketpair;
}

vector<string> convert_delimiter (vector<string> chainlist, char old_delim, char new_delim) {
   
    vector<string> chainlist_new; 
    for (unsigned int i = 0; i < chainlist.size(); i++) {
        vector<string> pair = parse_socketpair(chainlist[i], old_delim);
        string new_chainitem(pair[0] + new_delim + pair[1]);
        chainlist_new.push_back(new_chainitem);
    }
    
    return chainlist_new;
}

string read_chainfile (string filename) {

    string chainlist;

    ifstream chainfile(filename.c_str());

    string num_steps;
    getline(chainfile, num_steps);
    int chainlist_size = atoi(num_steps.c_str());

    string line;
    int chain_items = 0;
    while (getline(chainfile, line)) {
        chainlist += line;
        chainlist += CHAINLIST_DELIM;
        chain_items++;
    }
    chainlist = chainlist.substr(0, chainlist.size() - 1);             // remove last delim char

    if (chain_items != chainlist_size) {
        cout << "read_chainfile error: sizes don't match" << endl;
    }
    
    return chainlist;
}

// downloads url "filename" to current working directory
int retrieve_file(string filename) {

    cout << "Retrieving file..." << endl;
    string command("wget ");
    command += filename;
    system(command.c_str());

    return 0;
}

// returns socket file descriptoer to next stepping stone selected
int step_to_next(FileRequest *req) {

    cout << "Stepping to next stepping stone..." << endl;

    // select next stepping stone 
    cout << "Selecting next ss..." << endl;

    //cout << "Size of chainlist: " << chainlist.size() << endl;
    vector<string> ss = pick_rand_ss(req->get_chainlist_ref());    
    //cout << "Size of chainlist: " << chainlist.size() << endl;



    //cout << "About to repack chainlist...do we get this far?" << endl;



    // stringify chainlist after stepping stone removal above
    string chainlist_str = pack_chainlist(*(req->get_chainlist_ref()));
    
    cout << "Connecting..." << endl;
    
    int ssfd = connect_to_ss(ss);

    string url = req->get_url();

    // send url length and chainlist length
    send_short(ssfd, url.size());
    send_short(ssfd, chainlist_str.size());

    send_string(ssfd, url);
    if (chainlist_str.size() > 0) {
        send_string(ssfd, chainlist_str);
    }

    cout << "URL and chainlist sent!" << endl;


    

    return ssfd;
}

int packetize (string url, vector<string> *chainlist, char* data) {

    uint16_t url_len = htons(url.size());
    uint16_t chainlist_len = htons(chainlist->size());

    if (VERBOSE) {
       cout << "url_len (network): " << url_len << endl;
       cout << "chainlist_len (network): " << chainlist_len << endl;
    }

    // manually copy header info and message to data
    memcpy(data, &url_len, sizeof(url_len));
    memcpy(&(data[2]), &chainlist_len, sizeof(chainlist_len));
    memcpy(&(data[4]), url.c_str(), strlen(url.c_str()));


    // NOT COMPLETE



    return 0;
}

// selects a stepping stone from the chain list and returns a string vector of [ip, port]
vector<string> pick_rand_ss (vector<string> *chainlist) {
  
    cout << "Assigning stepping stone from last item in chainlist..." << endl;
    string ss(chainlist->back()); 		// PICKS THE FIRST ONE FOR NOW
    cout << ss << endl;
    
    cout << "Removing stepping stone from chainlist..." << endl; 
    chainlist->pop_back();
    cout << "Done." << endl;

    return parse_socketpair(ss, IPPORT_DELIM);
}

// connects to stepping stone ss, return socket file descriptor ---  MUST BE CLOSED LATER!!!
int connect_to_ss (vector<string> ss) {

    string ip = ss[0];
    string port = ss[1];

    if (VERBOSE) {
        cout << "Connecting to..." << endl;
        cout << "SERVER : " << ip << endl;
        cout << "PORT   : " << port << endl;
    }

    struct addrinfo hints, *res;
    int sockfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    if (status == -1) {
        cerr << "connect_to_ss error: getaddrinfo" << endl;
        return 2;
    }
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (status == -1) {
        cerr << "connect_to_ss error: socket" << endl;
        return 2;
    }

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        close(sockfd);
        cerr << "connect_to_ss error: connect failed" << endl;
        return 2;
    }

    // done with this addrinfo
    freeaddrinfo(res);
    
    // check if everything's good to go, then start comm
    if (!sockfd) {
        cerr << "connect_to_ss error: sockfd == NULL" << endl;
    }
    
    return sockfd;
}


// waits for packets sent in the format [SIZE, DATA...]
int wait_for_file (int sfd) {

    cout << "Waiting for file..." << endl;

    // data buffer for whole file
    int filesize = 0;

    vector<Chunk> chunks;                           // this is where we stick the data

    short packet_size = read_short(sfd);
    while (packet_size > 0) {
        
        cout << "Reading packet data..." << endl;

        filesize += packet_size;                     // adjust filesize by packetsize

        char *buffer = (char *)malloc(packet_size);  // allocate memory and read
        int bytes_read = read(sfd, buffer, packet_size);
        if (bytes_read < packet_size) {
            cout << "wait_for_file didn't read all of packet!" << endl;
        }

        Chunk new_chunk(buffer, packet_size);        // make new chunk
        chunks.push_back(new_chunk);                 // append to vector

        packet_size = read_short(sfd);               // prep for next round
    } 

    for (unsigned int i = 0; i < chunks.size(); i++) {
        filesize += chunks[i].get_size();
    }

    cout << "Chunks read: " << chunks.size() << endl;
    cout << "Total file size: " << filesize << endl;

    write_file(chunks); 

    return 0;
}

int write_file (vector<Chunk> chunks) {

    cout << "Writing file... " << endl;

    fstream ofile("package.file", ios::binary | ios::out | ios::app);

    if ( !ofile.is_open() ) {
        cout << "File is not open...probably error opening file" << endl;
    } else {
        for (unsigned int i = 0; i < chunks.size(); i++) {
            ofile.write(chunks[i].get_data(), chunks[i].get_size());
        }
    }

    ofile.close();
     
    return 0;
}

// given a socket, waits to receive 16-bit field indicating number of bytes,
// then reads that number of bytes from the socket
// returns 0 on success, -1 on not enough data received
int receive_packet (int sfd) {

    

    return 0;
}
