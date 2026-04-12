NAME = webserv

CC = c++

SRCPATH = src

HEADERPATH = include

FLAGS = -std=c++98 -Wall -Wextra -Werror

# Config
CONFIG_SRC_DIR = $(SRCPATH)/config
CONFIG_SRC = GlobalConfig.cpp ServerConfig.cpp Config.cpp ConfigParser.cpp DirectiveRegistry.cpp
CONFIG_HEADER_DIR = $(HEADERPATH)/config
CONFIG_HEADER = GlobalConfig.hpp ServerConfig.hpp Config.hpp ConfigParser.hpp DirectiveRegistry.hpp IDirectiveHandler.hpp

# Server
SERVER_SRC_DIR = $(SRCPATH)/server
SERVER_SRC = Server.cpp HttpRequest.cpp Connection.cpp \
utils.cpp SessionManager.cpp
SERVER_HEADER_DIR = $(HEADERPATH)/server
SERVER_HEADER = Server.hpp HttpRequest.hpp Connection.hpp \
utils.hpp SessionManager.hpp

# CGI
CGI_SRC_DIR = $(SRCPATH)/cgi
CGI_SRC = CgiExecutor.cpp CgiResponseParser.cpp CgiHandler.cpp
CGI_HEADER_DIR = $(HEADERPATH)/cgi
CGI_HEADER = CgiExecutor.hpp CgiResponseParser.hpp CgiHandler.hpp

# Http
HTTP_SRC_DIR = $(SRCPATH)/http
HTTP_SRC = HttpResponse.cpp HttpParser.cpp HttpRequestValidator.cpp HttpRouter.cpp HttpResponseBuilder.cpp
HTTP_HEADER_DIR = $(HEADERPATH)/http
HTTP_HEADER = HttpResponse.hpp HttpParser.hpp HttpRequestValidator.hpp HttpRouter.hpp HttpResponseBuilder.hpp

# Application
APPLICATION_SRC_DIR = $(SRCPATH)/application
APPLICATION_SRC = StaticFileHandler.cpp DirectoryListingHandler.cpp UploadHandler.cpp DeleteHandler.cpp RequestProcessor.cpp
APPLICATION_HEADER_DIR = $(HEADERPATH)/application
APPLICATION_HEADER = IRequestHandler.hpp StaticFileHandler.hpp DirectoryListingHandler.hpp UploadHandler.hpp DeleteHandler.hpp RequestProcessor.hpp

# Network
NETWORK_SRC_DIR = $(SRCPATH)/network
NETWORK_SRC = IOMultiplexer.cpp TcpSocket.cpp SocketManager.cpp ClientConnection.cpp
NETWORK_HEADER_DIR = $(HEADERPATH)/network
NETWORK_HEADER = IOMultiplexer.hpp TcpSocket.hpp SocketManager.hpp ClientConnection.hpp

# Utility
UTILITY_SRC_DIR = $(SRCPATH)/utility
UTILITY_SRC = Logger.cpp FileSystem.cpp Cache.cpp SignalSystem.cpp PathUtils.cpp MimeTypeResolver.cpp
UTILITY_HEADER_DIR = $(HEADERPATH)/utility
UTILITY_HEADER = Logger.hpp FileSystem.hpp Cache.hpp SignalSystem.hpp PathUtils.hpp \
MimeTypeResolver.hpp ResourceGuard.hpp ResourceDeleters.hpp TResourceGard.hpp SignalSystem.hpp

# Main source
MAIN_SRC = main.cpp

SRC = $(addprefix $(CONFIG_SRC_DIR)/, $(CONFIG_SRC)) \
$(addprefix $(HTTP_SRC_DIR)/, $(HTTP_SRC)) \
$(addprefix $(APPLICATION_SRC_DIR)/, $(APPLICATION_SRC)) \
$(addprefix $(SERVER_SRC_DIR)/, $(SERVER_SRC)) \
$(addprefix $(CGI_SRC_DIR)/, $(CGI_SRC)) \
$(addprefix $(NETWORK_SRC_DIR)/, $(NETWORK_SRC)) \
$(addprefix $(UTILITY_SRC_DIR)/, $(UTILITY_SRC)) \
$(addprefix $(SRCPATH)/, $(MAIN_SRC))

HEADER = $(addprefix $(CONFIG_HEADER_DIR)/, $(CONFIG_HEADER)) \
$(addprefix $(HTTP_HEADER_DIR)/, $(HTTP_HEADER)) \
$(addprefix $(APPLICATION_HEADER_DIR)/, $(APPLICATION_HEADER)) \
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
	mkdir -p $(OBJPATH)/application
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

test-all: re
	@bash ./tests/run_all.sh
	@echo "\n============================================"
	@echo "ALL TESTS COMPLETED"
	@echo "============================================\n"

# Lint / static analysis (optional tools)
# - Requires clang-tidy and a compilation database (compile_commands.json)
# - The compilation database can be generated with Bear ("bear -- make re")
tidy-db:
	@bash ./tools/clang_tidy.sh --build-db

tidy:
	@bash ./tools/clang_tidy.sh --auto-build-db --changed

tidy-all:
	@bash ./tools/clang_tidy.sh --auto-build-db --all

tidy-fix:
	@bash ./tools/clang_tidy.sh --auto-build-db --changed --fix

tidy-fix-all:
	@bash ./tools/clang_tidy.sh --auto-build-db --all --fix

.PHONY: all clean fclean run re debug test-all release tidy-db tidy tidy-all tidy-fix tidy-fix-all
