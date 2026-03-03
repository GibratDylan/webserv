NAME = webserv

HEADERS = #    include/minishell.h \
# 	include/lexer.h \
# 	include/parser.h \
# 	include/executor.h \
# 	include/builtins.h \
# 	include/resource_manager.h

CC = c++

FLAGS = -std=c++98 -Wall -Wextra -Werror

SRCPATH = src

# # Lexer sources
# LEXER_DIR = $(SRCPATH)/lexer
# LEXER_SRC = tokenizer.c \
# 	token_utils.c

# # Parser sources
# PARSER_DIR = $(SRCPATH)/parser
# PARSER_SRC = commands.c \
# 	commands_utils.c \
# 	syntax.c \
# 	parser_main.c \
# 	assign.c \
# 	utils.c \
# 	utils2.c \
# 	debug.c

# # Parser expansion sources
# EXPANSION_DIR = $(SRCPATH)/parser/expansion
# EXPANSION_SRC = expand.c \
# 	expand_split.c \
# 	dollar.c \
# 	wildcard.c \
# 	wildcard_utils.c \
# 	wildcard_matcher.c

# # Executor sources
# EXECUTOR_DIR = $(SRCPATH)/executor
# EXECUTOR_SRC = exec_all.c \
# 	exec_all_utils.c \
# 	exec_cmd.c \
# 	path.c \
# 	pipe_handler.c \
# 	exit_status.c

# # Executor redirections sources
# REDIRECTIONS_DIR = $(SRCPATH)/executor/redirections
# REDIRECTIONS_SRC = redirs.c \
# 	heredoc.c \
# 	heredoc_cleanup.c

# # Executor signals sources
# SIGNALS_DIR = $(SRCPATH)/executor/signals
# SIGNALS_SRC = signal_handler.c

# # Builtins sources
# BUILTINS_DIR = $(SRCPATH)/builtins
# BUILTINS_SRC = builtin_dispatcher.c \
# 	cd.c \
# 	echo.c \
# 	env.c \
# 	exit.c \
# 	export.c \
# 	pwd.c 

# # Resource manager sources
# RESOURCE_DIR = $(SRCPATH)/resource_manager
# RESOURCE_SRC = process_tracker.c \
# 	memory_tracker.c \
# 	error_handler.c

# Main source
MAIN_SRC = main.c

# Combine all sources with their paths
SRC = #   $(addprefix $(LEXER_DIR)/, $(LEXER_SRC)) \
# 	$(addprefix $(PARSER_DIR)/, $(PARSER_SRC)) \
# 	$(addprefix $(EXPANSION_DIR)/, $(EXPANSION_SRC)) \
# 	$(addprefix $(EXECUTOR_DIR)/, $(EXECUTOR_SRC)) \
# 	$(addprefix $(REDIRECTIONS_DIR)/, $(REDIRECTIONS_SRC)) \
# 	$(addprefix $(SIGNALS_DIR)/, $(SIGNALS_SRC)) \
# 	$(addprefix $(BUILTINS_DIR)/, $(BUILTINS_SRC)) \
# 	$(addprefix $(RESOURCE_DIR)/, $(RESOURCE_SRC)) \
# 	$(addprefix $(SRCPATH)/, $(MAIN_SRC))

OBJPATH = obj

# Create object file names from source files
OBJ = $(SRC:$(SRCPATH)/%.c=$(OBJPATH)/%.o)

all: $(NAME)

$(NAME): $(OBJPATH) $(LIBFT) $(OBJ) $(HEADERS)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

# Create object directories
$(OBJPATH):
	mkdir -p $(OBJPATH)
	mkdir -p $(OBJPATH)/lexer
	mkdir -p $(OBJPATH)/parser
	mkdir -p $(OBJPATH)/parser/expansion
	mkdir -p $(OBJPATH)/executor
	mkdir -p $(OBJPATH)/executor/redirections
	mkdir -p $(OBJPATH)/executor/signals
	mkdir -p $(OBJPATH)/builtins
	mkdir -p $(OBJPATH)/resource_manager

$(OBJPATH)/%.o: $(SRCPATH)/%.c $(HEADERS)
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

debug:
	
.PHONY: all clean fclean run re debug
