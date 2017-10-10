#include "query.h"
#include <sstream>
#include <cstring>


bool get_ContentLength(std::string& s, 
             size_t start, size_t& ContentLength){
     size_t str_size=s.size();
     
     while (start < str_size &&  s[start]==' '  ) ++start; //skipt space
     
     if (str_size == start) return false; //empty str
     
     size_t pos = start;
     for (; pos < str_size; ++pos){
         if (s[pos] < '0' || s[pos] > '9') break;
     }
     
    
     if (pos == start) return false; //not number
     
     std::string res = s.substr(start, pos - start);
     std::stringstream ss;
     ss << res;
     ss >> ContentLength;             
     return true;                 
}

int getOneQuery(std::string& partial,
                std::string& query ){
                          
     size_t posFin = partial.find("\r\n\r\n"); 
     if (  posFin == std::string::npos ) return STATE_WAIT_DATA;
               
     size_t sizeHeader = posFin +4;
    
     query = partial.substr(0, sizeHeader);
    
     size_t posField = query.find("Content-Length:");
    
     //дополнительных данных нет
     //удаляем запрос из буффера
     if ( posField == std::string::npos) {
        partial.erase(0, sizeHeader);
        return STATE_READY_QUERY;
     };
    
     //есть дополнительные данные Entity-Body
     posField = posField + strlen("Content-Length:");
     size_t ContentLength;
     if (!get_ContentLength(partial, 
             posField, ContentLength)) {
         //чтобы не зацикливаться на плохом запросе
         //удалим его.
         partial.erase(0, sizeHeader); 
         return STATE_BAD_REQUEST;           
     }
        
     //удаляем из буффера запрос и дополнительные данные    
     if ( partial.size() > sizeHeader + ContentLength){
        partial.erase(0, sizeHeader + ContentLength);        
        return STATE_READY_QUERY;
     }          
        
     return STATE_WAIT_DATA;
               
}

