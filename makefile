server: chat_room.cpp structHeader.cpp
	g++ chat_room.cpp structHeader.cpp -o bin/chat_room -lpthread
	
client: client.cpp structHeader.cpp
	g++ client.cpp structHeader.cpp -o bin/client -lpthread
