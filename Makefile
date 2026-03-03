NAME = webserv

CC = c++

FLAGS = -std=c++98 -Wall -Wextra -Werror

SRCPATH = src

HEADERPATH = include

# Parser config sources
PARSER_SRC_DIR = $(SRCPATH)/config
PARSER_SRC = GlobalConfig.cpp ServerConfig.cpp LocationConfig.cpp
PARSER_HEADER_DIR = $(HEADERPATH)/config
PARSER_HEADER = GlobalConfig.hpp ServerConfig.hpp LocationConfig.hpp

# Main source
MAIN_SRC = main.cpp

# Main header
MAIN_HEADER = webserv.hpp

# Combine all sources with their paths
SRC = $(addprefix $(PARSER_SRC_DIR)/, $(PARSER_SRC)) \
$(addprefix $(SRCPATH)/, $(MAIN_SRC))

HEADER = $(addprefix $(PARSER_HEADER_DIR)/, $(PARSER_HEADER)) \
$(addprefix $(HEADERPATH)/, $(MAIN_HEADER))

OBJPATH = obj

# Create object file names from source files
OBJ = $(patsubst $(SRCPATH)/%.cpp,$(OBJPATH)/%.o,$(SRC))

all: $(NAME)

$(NAME): $(OBJPATH) $(LIBFT) $(OBJ) $(HEADER)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

# Create object directories
$(OBJPATH):
	mkdir -p $(OBJPATH)
	mkdir -p $(OBJPATH)/config

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

debug: clean
	$(MAKE) FLAGS="$(FLAGS) -DDEBUG=1"
	./$(NAME) config/test.conf

.PHONY: all clean fclean run re debug
