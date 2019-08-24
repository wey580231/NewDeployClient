cmake_minimum_required(VERSION 3.7)
project(NewDeployClient)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        DeployFiles.cpp
        HandleCourse.cpp
        HandleDisk.cpp
        md5.cpp
        MyLog.cpp
        MyThread.cpp
        MyTime.cpp
        NewDeployClient.cpp
        RecvCommand.cpp
        RecvServiceMulticast.cpp
        ScanFiles.cpp
        SendHeart.cpp
        stringz.cpp
        Unicode_UTF.cpp global.cpp
        socket.cpp
        RUtil.cpp
        )

find_package(Threads)

add_executable(NewDeployClient ${SOURCE_FILES} )
target_link_libraries(NewDeployClient ${CMAKE_THREAD_LIBS_INIT})