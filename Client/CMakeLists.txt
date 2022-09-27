cmake_minimum_required(VERSION 3.12)
project(client_tcp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

add_executable(client-tcp Client/Client.cpp
        Client/Client.h "File Handler/FileReader.cpp" "File Handler/FileReader.h"
        "File Handler/FileWriter.cpp" "File Handler/FileWriter.h"
        Sender/Sender.cpp Sender/Sender.h Packet/Packet.h
        Receiver/Receiver.cpp Receiver/Receiver.h Utils/constants.h
        Packet/PacketHandler.cpp Packet/PacketHandler.h
        clientMain.cpp
        SR/SR_Receiver.cpp SR/SR_Receiver.h)