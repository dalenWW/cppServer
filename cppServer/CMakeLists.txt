set(SRCS
    Channel.cc
    Epoll.cc
    EventLoop.cc
    EventLoopThread.cc
    EventLoopThreadPool.cc
    Http.cc
    Main.cc
    Server.cc
    Connection.cc
    Timer.cc
    Util.cc
    HttpServer.cc
)
include_directories(${PROJECT_SOURCE_DIR}/base)


add_executable(cppServer ${SRCS})
target_link_libraries(cppServer libserver_base)


add_subdirectory(base)

