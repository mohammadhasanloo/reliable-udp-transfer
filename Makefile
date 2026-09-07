CXX      := c++
CXXFLAGS := -std=c++14 -pthread -O2
BUILD    := build

SERVER_SRC := Server/serverMain.cpp Server/Server/Server.cpp \
              "Server/File Handler/FileReader.cpp" "Server/File Handler/FileWriter.cpp" \
              Server/Sender/Sender.cpp Server/Receiver/Receiver.cpp \
              Server/Packet/PacketHandler.cpp Server/SR/SR_Receiver.cpp Server/SR/SR_Sender.cpp

CLIENT_SRC := Client/clientMain.cpp Client/Client/Client.cpp \
              "Client/File Handler/FileReader.cpp" "Client/File Handler/FileWriter.cpp" \
              Client/Sender/Sender.cpp Client/Receiver/Receiver.cpp \
              Client/Packet/PacketHandler.cpp Client/SR/SR_Receiver.cpp

.PHONY: all server client clean

all: server client

server: | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $(BUILD)/transfer_server $(SERVER_SRC)

client: | $(BUILD)
	$(CXX) $(CXXFLAGS) -o $(BUILD)/transfer_client $(CLIENT_SRC)

$(BUILD):
	@mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
