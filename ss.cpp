// ss.cpp - stepping stone program listens for requests and
//          transfers file via addresses in chainfile
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

#include "steps.h"

using namespace std;

const bool VERBOSE = true;
const int MAX_CHARS = 255;
const int MAX_URL_SIZE = MAX_CHARS;
const int BACKLOG = 1;
const int PORT = 55333;
const string default_filename("index.html");
const string test_file("https://www.wikipedia.org/portal/wikipedia.org/assets/img/Wikipedia-logo-v2.png");




// FUNCTIONS //////////////////////////////////////////////
int start_listening (int portreq);
string get_ip (void);

int packetize (string msg, void* data);

int retrieve_file (string filename);
int read_request (int connectionfd);
int thread_request (FileRequest *req);
void* process_request (void *request);

int get_file (string fn);
int chunkify_file (void);
int transmit_file (void);

int step_to_next (FileRequest *req);

int main (int argc, char* argv[]) {

    cout << "Starting stepping stone..." << endl;

    int opt = 0;
    string portval(to_string(PORT));

    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                portval = optarg;
                break;
            case '?':
                if (optopt == 'p') {
                    cerr << "Option -" << optopt << " requires an argument." << endl;
                } else {
                    cerr << "Unknown option " << optopt << endl;
                }

            default:
                cerr << "getopt error: default ... aborting!" << endl;
                abort();
        }
    }

    
    cout << "Testing... " << endl;
    
    cout << "testing file retrieve... " << endl;
    //retrieve_file("http://csb.stanford.edu/class/public/pages/sykes_webdesign/05_simple.html");
    //retrieve_file("https://www.wikipedia.org/portal/wikipedia.org/assets/img/Wikipedia-logo-v2.png");


    char hostname[MAX_CHARS];

    gethostname(hostname, MAX_CHARS);
    cout << hostname << " " << portval <<  endl;


    start_listening(atoi(portval.c_str()));
    
    return 0;
}


int start_listening (int portreq) {
    
    string port(to_string(portreq)); 
    struct addrinfo hints, *res, *p;
    int sockfd, connectedfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;       // use the IP of this machine

    int status = getaddrinfo(NULL, port.c_str(), &hints, &res);
    if (status == -1) {
        cerr << "start_listening error: getaddrinfo" << endl;
        return 2;
    }

    
    int usable_addresses = 0;
    for (p = res; p != NULL; p = p->ai_next) {
        usable_addresses++;
        
        //cout << "Address info : ";
        char ipaddrstr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, &((struct sockaddr_in *)(res->ai_addr))->sin_addr, ipaddrstr, INET6_ADDRSTRLEN); 
        //cout << ipaddrstr << endl;
    } 
    //cout << "getaddrinfo usable addresses : " << usable_addresses << endl;

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (status == -1) {
        cerr << "start_listening error: socket" << endl;
        return 2;
    }

    status = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        close(sockfd);
        cerr << "start_listening error: bind" << endl;
        return 2;
    }
    
    status = listen(sockfd, BACKLOG);
    if (status == -1) {
        close(sockfd);
        cerr << "start_listening error: listen" << endl;
        return 2;
    }
 
    cout << "Waiting for a connection on " << get_ip() << " port " << port << endl;

    struct sockaddr_storage peeraddr;
    socklen_t peeraddrsize = sizeof(peeraddr);

    connectedfd = accept(sockfd, (struct sockaddr *)&peeraddr, &peeraddrsize);
    if (status == -1) {
        cerr << "start_listening error: accept" << endl;
        return 2;
    } else {
        char peeraddrstr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*) &peeraddr)->sin_addr, peeraddrstr, INET6_ADDRSTRLEN);
        //cout << "Good connection!" << endl;
        //cout << "Connection from : " << peeraddrstr << endl;
        cout << "Received connection..." << endl;

        read_request(connectedfd);
    }
    
    freeaddrinfo(res);

    close(sockfd);
    close(connectedfd);
    return 0;
}


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


int read_request(int connectionfd) {

    cout << "Reading new file request..." << endl;


    // receive URL 
    string requested_url(MAX_URL_SIZE, '0');
     

    // receive chainfile
    vector<string> chainlist;

    FileRequest req(requested_url, chainlist.size(), chainlist);


    thread_request(&req);

    return 0;
}

int thread_request(FileRequest *req) {

    cout << "Threading new file request..." << endl;
   
    //pthread_attr_t *attributes = NULL;
    //pthread_attr_init(attributes);

    pthread_t thread = 0;
    
    //pthread_create(&thread, attributes, &process_request, req); 
    pthread_create(&thread, NULL, &process_request, req); 
    pthread_join(thread, NULL);

    //pthread_attr_destroy(attributes);

    return 0;
}

void* process_request(void *request) {

    cout << "Processing new file request..." << endl;

    vector<string> chainlist;

    if (chainlist.empty()) {                                 // get file if empty
        get_file(test_file);       
    } else {                                                 // otherwise, select next ss
        step_to_next((FileRequest*)request);
    }


    return 0;
}

int get_file (string fn) {

    retrieve_file(fn);

    chunkify_file();

    transmit_file();

    return 0;
}
 
int retrieve_file(string filename) {

    string command("wget ");
    command += filename;
    system(command.c_str());

    return 0;
}


int chunkify_file () {

    return 0;
}


int transmit_file () {

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
