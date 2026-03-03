NAME = webserv

CC = c++

FLAGS = -std=c++98 -Wall -Wextra -Werror

SRCPATH = src

HEADERSPATH = include

# Parser config sources
PARSER_SRC_DIR = $(SRCPATH)/parser_config
PARSER_SRC = 
PARSER_HEADER_DIR = $(HEADERSPATH)/parser_config
PARSER_HEADER = 

# Main source
MAIN_SRC = main.c

# Main header
MAIN_HEADER = webserv.hpp

# Combine all sources with their paths
SRC = $(addprefix $(PARSER_SRC_DIR)/, $(PARSER_SRC)) \
$(addprefix $(SRCPATH)/, $(MAIN_SRC))

HEADER = $(addprefix $(PARSER_HEADER_DIR)/, $(PARSER_HEADER)) \
$(addprefix $(HEADERPATH)/, $(MAIN_HEADER))

OBJPATH = obj

# Create object file names from source files
OBJ = $(SRC:$(SRCPATH)/%.c=$(OBJPATH)/%.o)

all: $(NAME)

$(NAME): $(OBJPATH) $(LIBFT) $(OBJ) $(HEADER)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

# Create object directories
$(OBJPATH):
	mkdir -p $(OBJPATH)
	mkdir -p $(OBJPATH)/parser_config

$(OBJPATH)/%.o: $(SRCPATH)/%.c $(HEADER)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf $(OBJPATH)

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C $(LIBFTPATH) fclean

re: fclean all

run: re
	./$(NAME)

debug: re
	./webserv -D DEBUG=1 config/default.conf

.PHONY: all clean fclean run re debug
