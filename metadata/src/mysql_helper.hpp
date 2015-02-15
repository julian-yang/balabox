#ifndef MYSQL_HELPER_HPP

/* basic c++ headers */
#include <vector>
#include <string>

/* mysql connector headers: DEFINE THESE LAST OR ELSE */
#include <my_global.h>
#include <mysql.h>

/**
 * This class defines basic functionality to interact with the metadata
 * mysql database
 */
class MySQLHelper {
public:

    /**
     * Opens a connection to the local mysql database using default parameters
     * @return 0 on success, nonzero on failure
     */
    int
    connect();

    /**
     * Opens a connection to a mysql database with user provided parameters
     * @return 0 on success, nonzero on failure
     */
     int
     connect(const std::string& host, unsigned int port, const std::string& db, const std::string& user, 
        const std::string& pass);

    /**
     * Closes connection to the local mysql database
     * @return 0 on success, nonzero on failure
     * @note connect() must have been previously called
     */
    int 
    close();

    /**
     * Finds supplied block hashes that are NOT in the database
     * @param user_hashes vector of user supplied hashes  
     * @param missing_hashes vector of user hashes that are NOT in the database
     * @return 0 on success, non-zero on failure
     */
    int 
    getMissingBlockHashes(const std::vector<std::string>& userHashes, 
        std::vector<std::string>& missingHashes);

    /**
     * Commits an update for a file to the database
     * @param userId user id string
     * @param filename name of file to commit changes to
     * @param hashes vector of block hashes for the file
     * @return 0 on success, non-zero on failure 
     * @note a user should have checked to make sure their blocks have been uploaded
     */
    int
    updateFileData(const std::string& userId, const std::string& filename, 
        const std::vector<std::string>& hashes); 

    /**
     * Retrieves block hashes for a given user's file
     * @param userId user id string
     * @param filename file to retrieve block hashes for
     * @param hashes reference to vector to fill with the requested file's hashes 
     * @return 0 on success, non-zero on failure
     */
    int
    getFileBlockList(const std::string& userId, const std::string& filename, 
        std::vector<std::string>& hashes);

private:
    MYSQL *m_conn;
};

#endif /* MYSQL_HELPER_HPP */