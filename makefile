server: chat_room.cpp structHeader.cpp Protocol.pb.cc
	g++ chat_room.cpp structHeader.cpp Protocol.pb.cc -o bin/chat_room -lpthread -lprotobuf
	
client: client.cpp structHeader.cpp Protocol.pb.cc
	g++ client.cpp structHeader.cpp Protocol.pb.cc -o bin/client -lpthread -lprotobuf
