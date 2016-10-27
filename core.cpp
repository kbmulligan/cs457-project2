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

string pack_chainlist (vector<string> vec_chain) {
    string chainlist(vec_chain[0]);

    for (unsigned int i = 1; i < vec_chain.size(); i++) {
        chainlist += CHAINLIST_DELIM;
        chainlist += vec_chain[i]; 
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
vector<string> parse_socketpair (string raw_data) {

    vector<string> socketpair;

    stringstream streamlist(raw_data);
    char delim = IPPORT_DELIM;

    string item;

    while(getline(streamlist, item, delim)) {
        socketpair.push_back(item);
    }

    return socketpair;
}

string read_chainfile (string filename) {

    string chainlist;

    ifstream chainfile(filename.c_str());

    string line;
    while (getline(chainfile, line)) {
        chainlist += line;
        chainlist += CHAINLIST_DELIM;
    }
    chainlist.pop_back();             // remove last delim char
    
    return chainlist;
}

// downloads url "filename" to current working directory
int retrieve_file(string filename) {

    string command("wget ");
    command += filename;
    system(command.c_str());

    return 0;
}


int step_to_next(FileRequest *req) {

    cout << "Stepping to next stepping stone..." << endl;

    return 0;
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
