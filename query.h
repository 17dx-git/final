#include <string>


const int STATE_BAD_REQUEST = -3;
const int STATE_ERROR_WRITE = -2;
const int STATE_ERROR_READ = -1;
const int STATE_READY_QUERY = 0;
const int STATE_WAIT_DATA = 1;
const int STATE_CLOSE_SOCKET = 2;


bool get_ContentLength(std::string& s, 
             size_t start, size_t& ContentLength);
             
int getOneQuery(std::string& partial,
                std::string& query ) ;           