/* c++ headers */
#include <stdlib.h>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
extern char ** environ;
#endif
#include "fcgio.h"
#include "fcgi_config.h" // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include "fcgi_stdio.h"
#include "leveldb/db.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unistd.h>

using namespace std;

// To check binary file: sudo curl --data-binary "@filename" --header "Host: www.balabox.com"

// Maximum number of bytes allowed to be read from stdin
static const unsigned long STDIN_MAX = 100000000;

/* 
	Parses request body into a vector<unsigned char>.
*/
vector<unsigned char> getRequestBody(const FCGX_Request &request)
{
	FCGX_Stream *in; 
    char * clenstr = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    unsigned long int clen = STDIN_MAX;

    vector<unsigned char> content;

    if (clenstr)
    {
        clen = strtoul(clenstr, &clenstr, 10);
        if (*clenstr)
        {
           
            //cout << "Status: 404\r\n\r\n";
            cerr << "Can't Parse \"CONTENT_LENGTH="
                 << FCGX_GetParam("CONTENT_LENGTH", request.envp)
                 << "\"\n";
            clen = STDIN_MAX;
        }

        // *always* put a cap on the amount of data that will be read
        if (clen > STDIN_MAX) clen = STDIN_MAX;

        int ch;

        for (unsigned long i = 0; i < clen; i++) {
        	ch = FCGX_GetChar(request.in);
        	content.push_back((unsigned char)ch);
        }

        clen = content.size();
    }
    else
    {
        // *never* read stdin when CONTENT_LENGTH is missing or unparsable
        clen = 0;
    }

    // Chew up any remaining stdin - this shouldn't be necessary
    // but is because mod_fastcgi doesn't handle it correctly.

    // ignore() doesn't set the eof bit in some versions of glibc++
    // so use gcount() instead of eof()...
    do cin.ignore(1024); while (cin.gcount() == 1024);

    return content;
}

void outputErrorMessage(const string& error) 
{
     cout << "Status: 400\r\n"
          << "Content-type: text/html\r\n"
          << "\r\n"
          << "<html><p>400 " << error << "</p></html>";
}

void outputSuccessMessage(const unordered_map<string, string>& info) {
	cout << "Status: 200\r\n"
         << "Origin: Block Server\r\n"
		 << "Content-type: text/html\r\n"
		 << "\r\n"
		 << "<html><p> 200 OK </p>\n"
         << "<p> Hash: " << info.find("hash")->second << "</p>\n"
         //<< "<p> Data: " << info.find("data")->second << "</p>\n"
         << "<p> IP: " << info.find("ip")->second << "</p>\n"
         << "</html>\n";
}

/** 
*   Returns ifconfig's eth0 information.
*/
string getEth0Info() {
    FILE *fp;
    char ipBuffer[64];
    string eth0;

    fp = popen("/sbin/ifconfig eth0", "r");

    while (fgets(ipBuffer, 64, fp) != NULL) {
        eth0 += ipBuffer;
    }

    pclose(fp);
    return eth0;
}

/**
*   Parses eth0 information for IP address.
*   @param ip: string containing IP address
*   Returns 0 upon success and nonzero otherwise.
*/
int getIPAddress(string& ip) {
    string eth0 = getEth0Info();
    string begin = "inet addr";

    // Verify that the inet addr exists
    int beginPos = eth0.find(begin);
    if (beginPos == string::npos) {
        return 1;
    }

    ip = eth0.substr(beginPos + begin.length() + 1, 13);
    return 0;
}

/*
  Parses given query string for block hash.
  Returns 0 upon success and nonzero otherwise.
*/
int getBlockHash(const string& param, string& blockHash) {
	// Verify that the parameter required is found
	int hashPos = param.find("hash");

	if (hashPos == string::npos) {
		return 1;
	}

	int equPos = param.find("=");
	blockHash = param.substr(equPos+1);
	
	return 0;
}

/* 
    Given a file's hash and the data that needs to be stored, 
    return a key-value pair suitable for leveldb storage.
*/
pair<string, string> constructKV(const string& hash, const vector<unsigned char>& fileData) {
    string value(fileData.begin(), fileData.end());

    if (value.size() != fileData.size())
        outputErrorMessage("Data loss in vector to string conversion");

    return make_pair(hash, value);
}

int main(void) {
	int count = 0;

    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    int status = FCGX_Init();
    status = FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0)
    {

    	// Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        #if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
        cin  = &cin_fcgi_streambuf;
        cout = &cout_fcgi_streambuf;
        cerr = &cerr_fcgi_streambuf;
        #else
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
        #endif

        // Although FastCGI supports writing before reading,
        // many http clients (browsers) don't support it (so
        // the connection deadlocks until a timeout expires!).

        // Get data from POST request
        vector<unsigned char> content = getRequestBody(request);
        unordered_map<string, string> successInfo;

        char* query_string = FCGX_GetParam("QUERY_STRING", request.envp);
        string errorMsg = "Invalid Input";

        // Invalid inputs
        if (query_string == nullptr) {
        	outputErrorMessage("No parameters");
        	continue;
        } 

        // Get blockhash from POST request
        string param = query_string;
        string blockHash;
        int getBlockHashSuccess = getBlockHash(param, blockHash);

        if (getBlockHashSuccess != 0) {
        	outputErrorMessage(errorMsg);
            continue;
        }

        // Open DB
        string dbName = "/var/www/html/mydb";
        leveldb::DB *db;
    	leveldb::Options options;
    	options.create_if_missing = true;
        leveldb::Status status;
        do {
            status = leveldb::DB::Open(options, "/var/www/html/mydb", &db);
            sleep(1);
        } while (!status.ok());
    	
    	leveldb::WriteOptions woptions;

    	// Insert into DB
    	pair<string, string> blockPair = constructKV(blockHash, content);
        do {
            status = db->Put(woptions, blockPair.first, blockPair.second);
            sleep(1);
        } while (!status.ok());
    	if (!status.ok()) {
    		outputErrorMessage(status.ToString());
            delete db;
    		continue;
    	}

        string ip;
        int getIPSuccess = getIPAddress(ip);
        if (getIPSuccess != 0) {
            outputErrorMessage("Unable to retrieve IP address");
            delete db;
            continue;
        }

        successInfo["hash"] = blockPair.first;
        successInfo["data"] = blockPair.second;
        successInfo["ip"] = ip;

        delete db;

    	outputSuccessMessage(successInfo);

    }

#if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
    cin  = cin_streambuf;
    cout = cout_streambuf;
    cerr = cerr_streambuf;
#else
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);
#endif

    return 0;
}

