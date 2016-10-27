// awget.cpp - awget program requests filedownload from 
//             stepping stone server
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
#include <vector>

#include "core.h"

using namespace std;


// FUNCTIONS //////////////////////////////////////////////


int main (int argc, char* argv[]) {

    cout << "Starting awget..." << endl;

    int opt = 0;
    string chainfile(DEFAULT_CHAINFILE);

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                chainfile = optarg;
                break;
            case '?':
                if (optopt == 'c') {
                    cerr << "Option -" << optopt << " requires an argument." << endl;
                } else {
                    cerr << "Unknown option " << optopt << endl;
                }

            default:
                cerr << "getopt error: default ... aborting!" << endl;
                abort();
        }
    }

    
    cout << "Using chainfile: " << chainfile << endl;
    
    cout << "Testing... " << endl;

    read_chainfile(chainfile);
    
    return 0;
}
