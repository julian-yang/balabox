#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <iterator>

#include <neon/ne_request.h>
#include <neon/ne_session.h>

#include "fcgi_stdio.h"
#include <jsoncpp/json/json.h>

#include "http_helper.h"

//using namespace cgicc;
using namespace std;
#define BLOCK_SIZE 512

//forwards the HTTP request to the actual Block Server
//TODO: change host & query to the correct values when finalized
int storeToBlockServer(string hash, string block) {
    //make request here.
    //string host = "127.0.0.1";
    string path = "/block_store?hash=" + hash;
    string responseContentType = "";
    string response = "";
    string responseCode = "";
    
    HttpHelper::sendHttpRequest(HttpHelper::block_ip, path, "POST", block, responseContentType, response, responseCode);
    printf("Status: %s\r\n", responseCode.c_str());
    printf("Content-Type:  %s\r\n\r\n", responseContentType.c_str());
    printf("%s", response.c_str());

    return 0;
}

//TODO: refactor Roger's file_fetch hashBlock #'s so that this code uses it too

int main(void)
{
    int count = 0;
    while(FCGI_Accept() >= 0) {
        // NOTE: The actual request body is piped into stdin:
        
        //code from http://stackoverflow.com/questions/10129085/read-from-stdin-write-to-stdout-in-c
        string response_body = "";
        char buffer[BLOCK_SIZE];
        while(!feof(stdin)) {
            size_t bytes = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
            response_body.append(buffer, bytes);
            //fwrite(buffer, bytes, sizeof(char), stdout);
        }
        
        string query_string = (getenv("QUERY_STRING"));
        string hash;
        HttpHelper::getQueryParam(query_string, "hash", hash);
        //printf("hash: %s\n", hash.c_str());
        storeToBlockServer(hash, response_body);
        
    }

	return 0;
}


