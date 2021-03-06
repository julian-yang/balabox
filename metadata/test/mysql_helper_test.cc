#include <stdio.h>
#include <string>
#include <iostream>
#include "mysql_helper.hpp"

#include <my_global.h>
#include <mysql.h>

using namespace std;

int test_file_list(MySQLHelper &h)
{
   cout << endl << "--FILE LIST TEST--"<< endl; 
   string uid = "steven";
   string filename = "testfile";
   vector<string> hashes;
   unsigned int version;
   if (h.getFileBlockList(uid,filename,hashes,version) != 0) {
       return -1;
   }
   for (unsigned int i = 0; i <hashes.size(); ++i) {
       cout << hashes[i] << endl;
   }
   cout << "version: " << version << endl;

    return 0;   
}

int test_file_update(MySQLHelper &h)
{
   cout << endl << "--FILE UPDATE TEST--"<< endl; 
   string uid = "steven";
   string filename = "newfile";
   vector<string> hashes;
   hashes.push_back("aaf02993af40bf0c8ab083519af47b0d3c5af5110b72d4a3eaea2df0c765264d");
   hashes.push_back("4a8d881b5d8f7fed33b1f5a6cd0e289ed6d801bd32dbb74bc3feeef8b2eceb3e");
   if (h.updateFileData(uid,filename,hashes,0) != 0) {
     return -1;
   }
   vector<string> retrieved_hashes;
   unsigned int version;
   if (h.getFileBlockList(uid,filename,hashes,version) != 0) {
     return -1;
   }

   for (unsigned int i = 0; i <retrieved_hashes.size(); ++i) {
       cout << hashes[i] << endl;
   } 

    return 0;
}

int test_missing_block_hashes(MySQLHelper &h)
{
    cout << endl << "--GET MISSING BLOCK HASH TEST--" << endl;
    vector<string> userHashes;
    userHashes.push_back("c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe");
    userHashes.push_back("9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
    // shouldnt be in test db
    userHashes.push_back("ffa63583dfa6706b87d284b86b0d693a161e4840aad2c5cf6b5d27c3b9621f7d");
    
    vector<string> missingHashes;
    if (h.getMissingBlockHashes(userHashes,missingHashes) != 0) {
        return -1;
    }
    if (missingHashes.size() != 1) {
        return -1;
    }

    cout << userHashes[0] << endl;
    return 0;
}

int test_get_recent_first_hashes(MySQLHelper &h)
{
    cout << endl << "--GET RECENT FIRST HASHES TEST--" << endl;
    vector<string> hashes;
    if (h.getRecentFirstHashes("steven", 4, hashes) != 0) {
        return -1;
    }

    for (unsigned int i = 0; i < hashes.size(); ++i) {
        cout << hashes[i] << endl;
    }
    return 0;
}
int test_get_user_file_names(MySQLHelper &h)
{
    cout << endl << "--GET USER FILE NAMES TEST--" << endl;
    vector<string> fileNames;
    vector<string> versions;
    if (h.getUserFileNames("steven", fileNames, versions) != 0) {
        return -1;
    }

    for (unsigned int i = 0; i < fileNames.size(); ++i) {
        cout << fileNames[i] << " " << versions[i] << endl;
    }
    return 0;
}

int test_get_caches(MySQLHelper &h)
{
    cout << endl << "--GET CACHES TEST--" << endl;
    vector<string> caches;
    if (h.getCaches("steven", 10, caches) != 0) {
        return -1;
    }

    for (unsigned int i = 0; i < caches.size(); ++i) {
        cout << caches[i] << endl;
    }
    return 0;
}
int test_remove_cache(MySQLHelper &h)
{
    cout << endl << "--REMOVE CACHE TEST--" << endl;
    vector<string> caches;
    if (h.removeCache("steven", "10.1.1.1") != 0) {
        return -1;
    }
    h.getCaches("steven",10,caches);
    if(caches.size() != 0) {
      cout << "Failed to remove steven -> 10.1.1.1 cache association!" << endl;
      return -1;
    }

    return 0;
}
int test_remove_file(MySQLHelper &h)
{
    string uid = "steven";
    string filename = "testfile";

    cout << endl << "--REMOVE FILE TEST--" << endl;
    if (h.removeFile(uid, filename) != 0) {
        return -1;
    }
    vector<string> fileHashes;
    unsigned int vers;
    h.getFileBlockList(uid,filename,fileHashes, vers);
    
    if(fileHashes.size() != 0) {
      cout << "Failed to remove " + uid + " filename: " + filename << endl;
      return -1;
    }

    return 0;
}
int test_add_cache(MySQLHelper& h)
{
  string uid = "testuser";
  string ipAddr = "127.0.0.1";

  cout << endl << "--ADD CACHE TEST--" << endl;
  
  if (h.addCache(uid, ipAddr)) {
    return -1;
  }

  vector<string> ipAddrs;
  h.getCaches(uid, 10, ipAddrs);
  if (ipAddrs.size() == 0) {
    cout << "Failed to add user-cache association!" << endl;
    return -1;
  }

  for (int i = 0; i < ipAddrs.size(); ++i) {
    cout << ipAddrs[i] << endl;
  }
  return 0;
}
int main(void)
{
    cout << "---MYSQL HELPER TEST---"<<endl<<endl;
    MySQLHelper h;
    int failed = 0;
    if (h.connect() == 0) {
        cout << "Successfully connected to mysql database!\n";
    }
    else {
        cout << "Failed to connect to mysql db!" << endl;
        failed++;
        goto end;
    }

    if(test_file_list(h) != 0) {
        printf("File list test failed!\n");
        failed++;
    }
    else {
        cout << "File list test succeeded"<<endl;
    }
    
    if(test_file_update(h) != 0) {
        cout << "file update test failed!" << endl;
        failed++;
    }
    else {
        cout << "file update test succeeded!" << endl;
    }

    if (test_missing_block_hashes(h) != 0) {
        cout << "get missing block hashes test failed!" << endl;
        failed++;
    }
    else {
        cout << "get missing block hashes test succeeded!" << endl;
    }    

    if (test_get_recent_first_hashes(h) != 0) {
        cout << "get recent first hashes test failed!" << endl;
        failed++;
    }
    else {
        cout << "get recent first hashes test suceeded!" << endl;
    }
    if (test_get_user_file_names(h) != 0) {
      failed++;
      cout << "Get User File Names Failed!" << endl;
    } 
    else {
        cout << "get user file names test suceeeded!" << endl;
    }
    if (test_get_caches(h) != 0) {
      failed++;
      cout << "get caches test failed!" << endl;
    } 
    else {
        cout << "get caches test suceeeded!" << endl;
    }

    if (test_remove_cache(h) != 0) {
      failed++;
      cout << "remove cache test failed!" << endl;
    } 
    else {
        cout << "remove cache test suceeeded!" << endl;
    }
    if (test_remove_file(h) != 0) {
      failed++;
      cout << "remove file test failed!" << endl;
    } 
    else {
        cout << "remove file test suceeeded!" << endl;
    }
    if (test_add_cache(h) != 0) {
      failed++;
      cout << "add cache test failed!" << endl;
    } 
    else {
        cout << "add cache test suceeeded!" << endl;
    }
    if(h.close() == 0) {
        cout << endl << "Successfully closed mysql connection!" << endl;
    }
    else {
        failed++;
        cout << "Failed to close mysql connection!" << endl;
    }
    
    end:
    if (failed == 0) {
        cout << endl << "---ALL TESTS PASSED---" << endl;
    }
    else {
        printf("\n---%d TESTS Failed---\n", failed);
    } 
    }
