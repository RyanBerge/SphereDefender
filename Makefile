# Makefile

SERVER = bin/SphereDefenderServer.exe
CLIENT = bin/Client.exe

SFML_INCLUDE = C:/Libraries/sfml/include
SFML_LIB = C:/Libraries/sfml/lib

OBJ_FILES := $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp))

SERVER_OBJ_FILES := $(OBJ_FILES)
SERVER_OBJ_FILES += $(patsubst src/Server/%.cpp,obj/Server/%.o,$(wildcard src/Server/*.cpp))

CLIENT_OBJ_FILES := $(OBJ_FILES)
CLIENT_OBJ_FILES += $(patsubst src/Client/%.cpp,obj/Client/%.o,$(wildcard src/Client/*.cpp))

SERVER_INCLUDE := include/Server
CLIENT_INCLUDE := include/Client

REBUILDABLES = $(OBJ_FILES) $(LINK_TARGET) obj/*.d obj/Server/*.d obj/Client/*.d

CXXFLAGS += -MMD
CXXFLAGS += -Wall

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_OBJ_FILES)
	g++ -g -o $@ $^ -I$(INCLUDE) -I$(SERVER_INCLUDE) -I$(SFML_INCLUDE) -L$(SFML_LIB) -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

$(CLIENT): $(CLIENT_OBJ_FILES)
	g++ -g -o $@ $^ -I$(INCLUDE) -I$(CLIENT_INCLUDE) -I$(SFML_INCLUDE) -L$(SFML_LIB) -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

obj/%.o: src/%.cpp
	g++ -g -std=c++17 $(CXXFLAGS) -c -o $@ -Iinclude -I$(SFML_INCLUDE) $<

obj/Server/%.o: src/Server/%.cpp $(OBJ_FILES)
	g++ -g -std=c++17 $(CXXFLAGS) -c -o $@ -Iinclude -I$(SERVER_INCLUDE) -I$(SFML_INCLUDE) $<

obj/Client/%.o: src/Client/%.cpp $(OBJ_FILES)
	g++ -g -std=c++17 $(CXXFLAGS) -c -o $@ -Iinclude -I$(CLIENT_INCLUDE) -I$(SFML_INCLUDE) $<

clean :
	rm -f $(REBUILDABLES)

-include $(OBJ_FILES:.o=.d)
-include $(SERVER_OBJ_FILES:.o=.d)
-include $(CLIENT_OBJ_FILES:.o=.d)
