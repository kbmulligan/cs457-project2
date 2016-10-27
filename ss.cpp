// ss.cpp - stepping stone program
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

short int read_short (int connectionfd);
int send_short (int connectionfd, short data);


int step_to_next (FileRequest *req);



// MAIN SS EXECUTION //////////////////////////////////////
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
    
    //cout << "testing file retrieve... " << endl;
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
    hints.ai_family = AF_INET;
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
        
        char ipaddrstr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, &((struct sockaddr_in *)(res->ai_addr))->sin_addr, ipaddrstr, INET6_ADDRSTRLEN); 
        cout << "Address info : ";
        cout << ipaddrstr << endl;
    } 
    cout << "getaddrinfo usable addresses : " << usable_addresses << endl;

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


int read_request(int connectionfd) {

    cout << "Reading new file request..." << endl;

    short int length_url = read_short(connectionfd);
    short int length_chainlist = read_short(connectionfd);

    cout << "URL len: " << length_url << endl;
    cout << "Chainlist len: " << length_chainlist << endl;

    // receive URL 
    string requested_url(MAX_URL_SIZE, '0');
    requested_url = read_string(connectionfd, length_url);

    // receive chainfile
    vector<string> chainlist;
    string chainlist_str = read_string(connectionfd, length_chainlist);

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
 
    FileRequest *req = reinterpret_cast<FileRequest *>(request);

    cout << req->get_url() << endl;

    vector<string> chainlist;

    if (chainlist.empty()) {                     // get file if no more steping stones 
        get_file(test_file);       
    } else {                                     // otherwise, select next ss
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
 


int chunkify_file () {

    return 0;
}


int transmit_file () {

    return 0;
}

