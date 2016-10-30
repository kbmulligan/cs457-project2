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

int read_request (int connectionfd);
int thread_request (FileRequest *req);
void* process_request (void *request);

int capture_file (string fn);
int chunkify_file (string filename);
int transmit_file (string fn, int sfd);
int delete_local_file (string fn);

short int read_short (int connectionfd);
int send_short (int connectionfd, short data);

string local_filename (string filename);



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
    if (connectedfd == -1) {
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
    string requested_url = read_string(connectionfd, length_url);

    // receive chainfile
    vector<string> chainlist;
    if (length_chainlist > 0) {
        string chainlist_str = read_string(connectionfd, length_chainlist);
    }

    // build request
    FileRequest req(requested_url, chainlist.size(), chainlist, connectionfd);

    // start new thread
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

    cout << "FileRequest.url = " << req->get_url() << endl;
    string url(req->get_url());

    if (req->get_chainlist_ref()->empty()) {     // get file if no more steping stones 
        cout << "Chainlist was empty...getting file... " << endl; 
        retrieve_file(url);       
    } else {                                     // otherwise, select next ss
        cout << "Chainlist was non-empty...stepping... " << endl; 
        int ssfd = step_to_next(req);
        wait_for_file(ssfd);
    }

    string fn = local_filename(url);             // strips url to filename only

    chunkify_file(fn);

    transmit_file(fn, req->get_socket());

    delete_local_file(fn);
    

    return 0;
}

int chunkify_file (string fn) {

    cout << "Chunking file: " << fn << "(" << fn.size() << ")" << endl;

    FileTarget package(fn);
    cout << "File size: " << package.get_size() << endl;

    return 0;
}


int transmit_file (string fn, int sfd) {
    cout << "Transmitting file: " << fn << " on socket: " << sfd << endl;

    int packetsize = 10;
    send_short(sfd, packetsize);

    return 0;
}

int delete_local_file (string fn) {
    cout << "Deleting file: " << fn << endl;
    string command = "rm " + fn;
    system(command.c_str());
    return 0;
}

// strips first part of URL, leaving only the filename with extension
string local_filename (string filename) {
    int index = filename.rfind("/"); 
    string fn = filename.substr( index + 1, filename.size() - (index + 2) );
    return fn; 
}
