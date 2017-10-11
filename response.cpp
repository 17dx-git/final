#include <sstream>
#include <string>
#include <cstring>
#include <fstream>

//#include <iostream>
//#include "query.h"


void HeaderCreate(std::stringstream& ss,
            const char *status_code, 
            int content_Length){
    ss << "HTTP/1.0 "<< status_code <<"\r\n"
       << "Content-Type: text/html\r\n"
       << "Content-Length: " << content_Length << "\r\n"; 
}

void HeaderAddField(std::stringstream& ss,
            const char *field){
    ss << field << "\r\n"; 
}

void HeaderFinalize(std::stringstream& ss){
    ss << "\r\n"; 
}

void CreateBody(std::stringstream& ss,
            const char *body){
    ss << body; 
}

bool HexToDec(char hex, unsigned char & dec){
    if (hex >= '0' && hex <= '9'){
        dec = hex - '0';
    } else if (hex >= 'A' && hex <= 'F'){
        dec = 10 + hex - 'A';
    } else if (hex >= 'a' && hex <= 'f'){
        dec = 10 + hex - 'a';        
    } else return false;
    
    return true;
}

bool Hex2ToDec(char hexL, char hexR, char & dec){
    unsigned char decL;
    unsigned char decR;
        
    if (!HexToDec(hexL, decL)) return false;
    if (!HexToDec(hexR, decR)) return false;
    
    dec = decL*16 + decR;
    
    return true;
}


bool decodeFile(std::string& file){
    size_t fsize=file.size();
    std::size_t pos = file.find("%"); //find:  % hexL hexR 
    while(  pos != std::string::npos){
        if ( file.size() < pos+2 ) return false;
        
        char hexL = file[pos+1];
        char hexR = file[pos+2];
        
        char decodedChar;        
        if ( ! Hex2ToDec( hexL, hexR,
                          decodedChar )  ) return false;

        file[pos] = decodedChar; //replace char '%' on decodedChar 
        file.erase(pos + 1, 2) ; // erase hexL and hexR 
        
        pos = file.find("%", pos+1);
       
    }
    return true;
}

bool extactFile(const std::string& query,size_t& pos, std::string& file){
    size_t qsize = query.size();
    if ( qsize <= pos) return false;
    
        
    while (pos < qsize &&  query[pos]==' '  ) ++pos; //skipt space
    
    if ( qsize == pos) return false; // in query was only space
        
    
    if (query[pos] !='/') return false;  //path start with "/"
    
    ++pos; // skipt "/"
    size_t pos_start = pos;
    
    while (pos < qsize   ) {
        //skipt file path
        if (query[pos] ==' ' 
           || query[pos] =='\n'
           || query[pos] =='\r'
           || query[pos] =='?'){
           break; 
        }        
        ++pos;  
    }; 
    
    //if (pos_start != pos ){
        
    file = query.substr(pos_start, pos-pos_start);
    
    //skipt params to end URI (file&params)
    while (pos < qsize &&  query[pos] !=' '  ) ++pos; 
    
    return true;
}

bool HTTP_VersExist(const std::string& query, size_t & pos){
    size_t qsize=query.size();
    
    while (pos < qsize &&  query[pos]==' '  ) ++pos; //skipt space
    
    if (qsize - pos  < strlen("HTTP/1.0") ) return false;
    
    
    
    bool HTTP_OK= query[pos+0] == 'H'  
               && query[pos+1] == 'T'
               && query[pos+2] == 'T'
               && query[pos+3] == 'P' 
               && query[pos+4] == '/' ;
    
    if (! HTTP_OK) return false;  
    
    pos = pos+5;
    
    
    bool HTTP_1_0 =  query[pos+0] == '1'   
                 &&  query[pos+1] == '.'
                 &&  query[pos+2] == '0' ;  
                 
    bool HTTP_0_9 =  query[pos+0] == '0'   
                 &&  query[pos+1] == '.'
                 &&  query[pos+2] == '9' ;                   

        
    if ( HTTP_1_0 || HTTP_0_9 ) return true; 
    
    return false;
}


bool MethodIsGET(const std::string& query, size_t & pos){
    
    if (query.size() < 4) return false;
    
    if (   query[0] == 'G' 
        && query[1] == 'E'
        && query[2] == 'T'
        && query[3] == ' ' ) {
           pos=4;
           return true; 
        }
    return false;
}

void CreateResponse200(std::ifstream& is, std::stringstream& response){
      is.seekg (0, std::ios::end);
      int content_Length = is.tellg();
      is.seekg (0, std::ios::beg);
     
      char * body = new char [content_Length];
      
      is.read (body,content_Length);
            
      HeaderCreate(response, "200 OK", content_Length);
      HeaderFinalize(response);
      
      CreateBody(response, body);
      delete [] body;    
}

void CreateResponse400(std::stringstream& response){       
        const char * body=
               "<title>Final Server **Bad Request**</title>\n"
               "<h1>Sorry, you request not valid</h1>\n"; 
             
        int content_Length = strlen(body);
        HeaderCreate(response, "400 Bad Request", content_Length );
        HeaderFinalize(response);
        CreateBody(response, body );
}

void CreateResponse404(std::stringstream& response){       
        const char * body =
                "<title>Final Server **page not found**</title>\n"
                "<h1>Sorry, page not exists</h1>\n"; 
             
        int content_Length = strlen(body);
        HeaderCreate(response, "404 Not Found", content_Length );
        HeaderFinalize(response);
        CreateBody(response, body );
}

void CreateResponse501(std::stringstream& response){       
        const char * body =
                "<title>Final Server **error query**</title>\n"
                "<h1>Sorry, method not implemented </h1>\n"; 
             
        int content_Length = strlen(body);
        HeaderCreate(response, "501 Not Implemented", content_Length );
        HeaderAddField(response,"Allow: GET");
        HeaderFinalize(response);
        
        CreateBody(response, body );
}


void CreateResponse(const std::string& query, std::stringstream& response){
    size_t pos=0;
    if (! MethodIsGET(query, pos) ) {
        CreateResponse501(response);
        return ;
    }
    
       
    std::string file;    
    if (! extactFile(query, pos, file) ) {
        CreateResponse400(response);
        return ;
    }
       
    if (! decodeFile(file) ) {
        CreateResponse400(response);
        return ;
    }
    
    //std::cout << file << std::endl;
    
    if (! HTTP_VersExist(query, pos) ) {
        CreateResponse400(response);
        return ;
    }
    
        
    std::ifstream is;
    
    if (file == "") file = "index.html";
    
    is.open (file.c_str(), std::ios::binary );
    if ( is.is_open() ){ 
        CreateResponse200(is, response) ;
        is.close();  
    }
    else{        
        CreateResponse404(response);
    }    
}


/*int main()
{    
    std::string query = "GET /CMake%20Lists.txt?f=1&h=2  HTTP/1.0\r\n";
    std::stringstream response;
    CreateResponse(query, response);
    
    std::cout << response.str();
    return 0;
}*/


/*int main()
{    
    std::string partial = "GET /CMake%20Lists.txt?f=1&h=2  HTTP/1.0\r\n\r\n"
                          "GET /query.h  HTTP/1.0\r\n"
                          "Content-Length: 5\r\n\r\n12345"
                          "GET /2.txt  HTTP/1.0\r\n";
       
    std::string query;
    int state = getOneQuery(partial, query);
    
    while(state != STATE_WAIT_DATA ){
        std::stringstream response;
        if ( state == STATE_BAD_REQUEST){
            CreateResponse400(response);
        } else{ //STATE_READY_QUERY
            CreateResponse(query, response);
        }
        
        std::cout << query << std::endl;
        
        std::cout << response.str();
        
        state = getOneQuery(partial, query);
    }
    
    return 0;
}*/
