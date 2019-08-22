#include"Http.h"
#include<iostream>
#include<sstream>
#include"base/Logging.h"

Http::Http()
    : parseState_(false),
      keepAlive_(true)
{
}

Http::~Http()
{
}

bool Http::parseHttpRequest(std::string &s)
{
    std::string crlf = "\r\n"; 
    std::string crlfcrlf = "\r\n\r\n";
    size_t prev = 0, next = 0, pos_colon;
	std::string key, value;
    bool parseFlag = false;

    if ((next = s.find(crlf, prev)) != std::string::npos)
    {
        std::string first_line = s.substr(prev, next - prev);
		prev = next;
		std::stringstream sstream(first_line);
		sstream >> (httpRequest_.method);
		sstream >> (httpRequest_.url);
		sstream >> (httpRequest_.version);
    }
    else
	{
        LOG << s;
		LOG << "Error in httpParse: http_request_line isn't complete!";
        parseFlag = false;
        s.clear();
        return parseFlag;
	}

    size_t pos_crlfcrlf = 0;
    if (( pos_crlfcrlf = s.find(crlfcrlf, prev)) != std::string::npos)
	{
		while (prev != pos_crlfcrlf)
        {
            next = s.find(crlf, prev + 2);
            pos_colon = s.find(":", prev + 2);
            key = s.substr(prev + 2, pos_colon - prev-2);
            value = s.substr(pos_colon + 2, next-pos_colon-2);
            prev = next;
            httpRequest_.header.insert(std::pair<std::string, std::string>(key, value));
        }
	}
    else
    {
        LOG << "Error in httpParse: http_request_header isn't complete!";
        parseFlag = false;
        s.clear();
        return parseFlag;
    }
    //parse http request body
	httpRequest_.body = s.substr(pos_crlfcrlf + 4);
    parseFlag = true;
    s.clear();
    return parseFlag;
}

void Http::httpHandle(std::string &responsecontext)
{
    if(httpRequest_.method == "GET")
    {
    }
    else if(httpRequest_.method == "POST")
    {
    }
    else
    {
        errorMsg_ = "Method Not Implemented";
        httpError(501,"Method Not Implemented",responsecontext);
        return;
    }

    size_t pos = httpRequest_.url.find("?");
    if(pos != std::string::npos)
    {
        path_ = httpRequest_.url.substr(0,pos);
        queryString_ = httpRequest_.url.substr(pos+1);
    }
    else
    {
        path_ = httpRequest_.url;
    }
        //keepalive判断处理
    auto iter = httpRequest_.header.find("Connection");
    if(iter != httpRequest_.header.end())
    {
        keepAlive_ = (iter->second == "Keep-Alive");
    }
    else
    {
        if(httpRequest_.version == "HTTP/1.1")
        {
            keepAlive_ = true;//HTTP/1.1默认长连接
        }
        else
        {
            keepAlive_ = false;//HTTP/1.0默认短连接
        }            
    }

    if(path_ == "/")
    {        
        path_ = "/index.html";
    }
    else if(path_ == "/hello")
    {
        std::string filetype("text/html");
        responseBody_ = ("hello world");
        responsecontext += httpRequest_.version + " 200 OK\r\n";
        responsecontext += "Server: NetServer/0.1\r\n";
        responsecontext += "Content-Type: " + filetype + "; charset=utf-8\r\n";
        if(iter != httpRequest_.header.end())
        {
            responsecontext += "Connection: " + iter->second + "\r\n";
        }
        responsecontext += "Content-Length: " + std::to_string(responseBody_.size()) + "\r\n";
        responsecontext += "\r\n";
        responsecontext += responseBody_;
        return;
    }
    else
    {
    }    
 
    path_.insert(0,".");
    FILE* fp = NULL;
    if((fp = fopen(path_.c_str(), "rb")) == NULL)
    {
        errorMsg_ = "Not Found";
        httpError(404,"Not Found",responsecontext);
        return;
    }
    else
    {
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        while(fread(buffer, sizeof(buffer), 1, fp) == 1)
        {
            responseBody_.append(buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        if(feof(fp))
        {
            responseBody_.append(buffer);
        }        
        else
        {
            std::cout << "error fread" << std::endl;
        }        	
        fclose(fp);
    }

    std::string filetype("text/html"); //固定为html
    responsecontext += httpRequest_.version + " 200 OK\r\n";
    responsecontext += "Server: WebServer/0.1\r\n";
    responsecontext += "Content-Type: " + filetype + "; charset=utf-8\r\n";
    if(iter != httpRequest_.header.end())
    {
        responsecontext += "Connection: " + iter->second + "\r\n";
    }
    responsecontext += "Content-Length: " + std::to_string(responseBody_.size()) + "\r\n";
    responsecontext += "\r\n";
    responsecontext += responseBody_;
}

void Http::httpError(const int err_num,const std::string errmsg,std::string &responsecontext)
{
    errorMsg_ = errmsg;
    std::string responsebody;
    responsebody += "<html><title>出错了</title>";
    responsebody += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>";
    responsebody += "<style>body{background-color:#f;font-size:14px;}h1{font-size:60px;color:#eeetext-align:center;padding-top:30px;font-weight:normal;}</style>";
    responsebody += "<body bgcolor=\"ffffff\"><h1>";
    responsebody += std::to_string(err_num) + " " + errorMsg_;
    responsebody += "</h1><hr><em> NetServer</em>\n</body></html>";

    std::string httpversion;
    if(httpRequest_.version.empty())
    {
        httpversion = "HTTP/1.1";
    }
    else
    {
        httpversion = httpRequest_.version;
    }

    responsecontext += httpversion + " " + std::to_string(err_num) + " " + errorMsg_ + "\r\n";
    responsecontext += "Server: NetServer/0.1\r\n";
    responsecontext += "Content-Type: text/html\r\n";
    responsecontext += "Connection: Keep-Alive\r\n";
    responsecontext += "Content-Length: " + std::to_string(responsebody.size()) + "\r\n";
    responsecontext += "\r\n";
    responsecontext += responsebody;
}