#include"EventLoop.h"
#include"Server.h"
#include"base/Logging.h"
#include <getopt.h>
#include <string>
#include"Httpserver.h"
#include<iostream>

int main(int argc,char* argv[])
{
      int numThread = 4;
      int port = 8000;
      std::string logPath =  "./server.log";

      int opt;
      const char *str = "t:l:p:";
      while ((opt = getopt(argc, argv, str))!= -1)
      {  
            switch (opt)
            {
                  case 't':
                  {
                        numThread = atoi(optarg);
                        break;
                  }
                  case 'l':
                  {
                        logPath = optarg;
                        if (logPath.size() < 2 || optarg[0] != '/')
                        {
                              printf("logPath should start with \"/\"\n");
                              abort();
                        }
                        break;      
                  }
                  case 'p':
                  {
                        port = atoi(optarg);
                        break;
                  }
                  default: break;
            }
      }
      Logger::setLogFileName(logPath);
      EventLoop mainLoop;
      HttpServer myHTTPServer(&mainLoop, port, numThread);
      // Server myServer(&mainLoop, port, numThread);      
      myHTTPServer.start();
      // myServer.start();
      mainLoop.loop();
      return 0;
}
