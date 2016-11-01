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
int usage (char* argv[]);
string clean_url(string url);

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

    string url;
    int non_opt_args = 0;
    for (int index = optind; index < argc; index++) {
        //cout << "Non-option argument: " << argv[index] << endl;
        url = argv[index];
        non_opt_args++;
    }
    if (non_opt_args != 1) {                         // check for URL arg
        cout << "Non optional arguments: " << non_opt_args << endl;
        usage(argv);
    }

    url = clean_url(url);
    cout << "Requesting file: " << url << endl;

    // process chainfile, convert to common delimeter 
    cout << "Using chainfile: " << chainfile << endl;
   
    // read chainlist from file 
    string chainlist_str = read_chainfile(chainfile);
    //cout << chainlist_str << endl;

    // parse and convert
    vector<string> chainlist = parse_chainlist(chainlist_str);
    chainlist = convert_delimiter(chainlist, IPPORT_FILE_DELIM, IPPORT_DELIM);
    
    cout << "Chainlist is: " << endl; 
    print_chainlist(chainlist);

    // repack and print 
    chainlist_str = pack_chainlist(chainlist);
    //cout << chainlist_str << endl;
   
    // select next stepping stone 
    cout << "Selecting stepping stone... ";

    //cout << "Size of chainlist: " << chainlist.size() << endl;
    vector<string> ss = pick_rand_ss(&chainlist);    
    //cout << "Size of chainlist: " << chainlist.size() << endl;

    /*
    if (!ss.empty()) {
        cout << ss[0] << " : " << ss[1] << endl;
    } else {
        cout << "NONE!" << endl;
    }
    */

    //cout << "About to repack chainlist...do we get this far?" << endl;
    // stringify chainlist after stepping stone removal above
    chainlist_str = pack_chainlist(chainlist);
    

    cout << "Connecting to next stepping stone..." << endl;
    
    int ssfd = connect_to_ss(ss);

    // send url length and chainlist length
    send_short(ssfd, url.size());
    send_short(ssfd, chainlist_str.size());

    send_string(ssfd, url);
    if (chainlist_str.size() > 0) {
        send_string(ssfd, chainlist_str);
    }

    if (VERBOSE) {  
        cout << "URL and chainlist sent!" << endl;
    }

    FileRequest req(url, chainlist.size(), chainlist, 0);
    wait_for_file(&req, ssfd);

    return 0;
}

string clean_url (string url) {

    if (url.find('/') == string::npos) { 
        url += "/index.html";
    }    
    if (url.find('/') == url.size() - 1) {
        url += "index.html";
    }
    return url;
}

int usage (char* argv[]) {
    cout << "Usage: " << argv[0] << " [-c CHAINFILE] URL" << endl; 
    exit(0);
    return 0;
}


