NAME = webserv

CC = c++

SRCPATH = src

HEADERPATH = include

FLAGS = -std=c++98 -Wall -Wextra -Werror

# Config
CONFIG_SRC_DIR = $(SRCPATH)/config
CONFIG_SRC = GlobalConfig.cpp ServerConfig.cpp Config.cpp
CONFIG_HEADER_DIR = $(HEADERPATH)/config
CONFIG_HEADER = GlobalConfig.hpp ServerConfig.hpp Config.hpp

# Server
SERVER_SRC_DIR = $(SRCPATH)/server
SERVER_SRC = Server.cpp HttpRequest.cpp HttpResponse.cpp Connection.cpp \
FileHandler.cpp utils.cpp SessionManager.cpp
SERVER_HEADER_DIR = $(HEADERPATH)/server
SERVER_HEADER = Server.hpp HttpRequest.hpp HttpResponse.hpp Connection.hpp \
FileHandler.hpp utils.hpp SessionManager.hpp

# CGI
CGI_SRC_DIR = $(SRCPATH)/cgi
CGI_SRC = CgiHandler.cpp
CGI_HEADER_DIR = $(HEADERPATH)/cgi
CGI_HEADER = CgiHandler.hpp

# Http
HTTP_SRC_DIR = $(SRCPATH)/http
HTTP_SRC = HttpStatus.cpp
HTTP_HEADER_DIR = $(HEADERPATH)/http
HTTP_HEADER = HttpStatus.hpp

# Network
NETWORK_SRC_DIR = $(SRCPATH)/network
NETWORK_SRC = IOMultiplexer.cpp
NETWORK_HEADER_DIR = $(HEADERPATH)/network
NETWORK_HEADER = IOMultiplexer.hpp

# Utility
UTILITY_SRC_DIR = $(SRCPATH)/utility
UTILITY_SRC = Logger.cpp FileSystem.cpp Cache.cpp SignalSystem.cpp PathUtils.cpp
UTILITY_HEADER_DIR = $(HEADERPATH)/utility
UTILITY_HEADER = Logger.hpp FileSystem.hpp Cache.hpp SignalSystem.hpp PathUtils.hpp \
ResourceDeleters.hpp TResourceGard.hpp SignalSystem.hpp

# Main source
MAIN_SRC = main.cpp

SRC = $(addprefix $(CONFIG_SRC_DIR)/, $(CONFIG_SRC)) \
$(addprefix $(HTTP_SRC_DIR)/, $(HTTP_SRC)) \
$(addprefix $(SERVER_SRC_DIR)/, $(SERVER_SRC)) \
$(addprefix $(CGI_SRC_DIR)/, $(CGI_SRC)) \
$(addprefix $(NETWORK_SRC_DIR)/, $(NETWORK_SRC)) \
$(addprefix $(UTILITY_SRC_DIR)/, $(UTILITY_SRC)) \
$(addprefix $(SRCPATH)/, $(MAIN_SRC))

HEADER = $(addprefix $(CONFIG_HEADER_DIR)/, $(CONFIG_HEADER)) \
$(addprefix $(HTTP_HEADER_DIR)/, $(HTTP_HEADER)) \
$(addprefix $(NETWORK_HEADER_DIR)/, $(NETWORK_HEADER)) \
$(addprefix $(UTILITY_HEADER_DIR)/, $(UTILITY_HEADER)) \
$(addprefix $(CGI_HEADER_DIR)/, $(CGI_HEADER)) \
$(addprefix $(SERVER_HEADER_DIR)/, $(SERVER_HEADER))

OBJPATH = obj

OBJ = $(patsubst $(SRCPATH)/%.cpp,$(OBJPATH)/%.o,$(SRC))

all: $(NAME)

$(NAME): $(OBJPATH) $(LIBFT) $(OBJ) $(HEADER)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJPATH):
	mkdir -p $(OBJPATH)
	mkdir -p $(OBJPATH)/config
	mkdir -p $(OBJPATH)/http
	mkdir -p $(OBJPATH)/server
	mkdir -p $(OBJPATH)/cgi
	mkdir -p $(OBJPATH)/network
	mkdir -p $(OBJPATH)/utility

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp $(HEADER)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf $(OBJPATH)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: re
	./$(NAME)

debug: fclean
	$(MAKE) FLAGS="$(FLAGS) -g"
	valgrind --leak-check=full ./$(NAME) config/default.conf

release: fclean
	$(MAKE) FLAGS="$(FLAGS) -O3 -march=native -mtune=native"


test-all: all
	@bash ./tests/run_all.sh
	@echo "\n============================================"
	@echo "ALL TESTS COMPLETED"
	@echo "============================================\n"

.PHONY: all clean fclean run re debug test-all release
