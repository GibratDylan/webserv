NAME = webserv

CC = c++

FLAGS = -std=c++98 -Wall -Wextra -Werror

SRCPATH = src

HEADERPATH = include

# config sources
CONFIG_SRC_DIR = $(SRCPATH)/config
CONFIG_SRC = GlobalConfig.cpp ServerConfig.cpp Config.cpp
CONFIG_HEADER_DIR = $(HEADERPATH)/config
CONFIG_HEADER = GlobalConfig.hpp ServerConfig.hpp Config.hpp

# Http
HTTP_SRC_DIR = $(SRCPATH)/http
HTTP_SRC = HttpStatus.cpp
HTTP_HEADER_DIR = $(HEADERPATH)/http
HTTP_HEADER = HttpStatus.hpp

# Main source
MAIN_SRC = main.cpp

# Main header
MAIN_HEADER = webserv.hpp

# Combine all sources with their paths
SRC = $(addprefix $(CONFIG_SRC_DIR)/, $(CONFIG_SRC)) \
$(addprefix $(HTTP_SRC_DIR)/, $(HTTP_SRC)) \
$(addprefix $(SRCPATH)/, $(MAIN_SRC))

HEADER = $(addprefix $(CONFIG_HEADER_DIR)/, $(CONFIG_HEADER)) \
$(addprefix $(HTTP_HEADER_DIR)/, $(HTTP_HEADER)) \
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
	mkdir -p $(OBJPATH)/http

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
	$(MAKE) FLAGS="$(FLAGS) -DDEBUG=1 -g"
	valgrind --leak-check=full ./$(NAME) config/test.conf

# Test configurations valides
test-valid: re
	@echo "\n============================================"
	@echo "Testing VALID configurations"
	@echo "============================================\n"
	@echo "--- Testing valid_minimal.conf ---"
	-./$(NAME) config/valid_minimal.conf
	@echo "\n--- Testing valid_basic.conf ---"
	-./$(NAME) config/valid_basic.conf
	@echo "\n--- Testing valid_multiple_servers.conf ---"
	-./$(NAME) config/valid_multiple_servers.conf
	@echo "\n--- Testing valid_complex.conf ---"
	-./$(NAME) config/valid_complex.conf
	@echo "\n--- Testing valid_with_comments.conf ---"
	-./$(NAME) config/valid_with_comments.conf
	@echo "\n--- Testing valid_whitespace.conf ---"
	-./$(NAME) config/valid_whitespace.conf
	@echo "\n--- Testing default.conf ---"
	-./$(NAME) config/default.conf
	@echo "\n============================================"
	@echo "Valid configuration tests COMPLETED"
	@echo "============================================\n"

# Test configurations avec erreurs
test-errors: re
	@echo "\n============================================"
	@echo "Testing ERROR configurations"
	@echo "============================================\n"
	@echo "--- Testing error_missing_semicolon.conf ---"
	-./$(NAME) config/error_missing_semicolon.conf
	@echo "\n--- Testing error_unclosed_brace.conf ---"
	-./$(NAME) config/error_unclosed_brace.conf
	@echo "\n--- Testing error_extra_brace.conf ---"
	-./$(NAME) config/error_extra_brace.conf
	@echo "\n--- Testing error_duplicate_port.conf ---"
	-./$(NAME) config/error_duplicate_port.conf
	@echo "\n--- Testing error_duplicate_location.conf ---"
	-./$(NAME) config/error_duplicate_location.conf
	@echo "\n--- Testing error_unknown_directive.conf ---"
	-./$(NAME) config/error_unknown_directive.conf
	@echo "\n--- Testing error_empty_directive.conf ---"
	-./$(NAME) config/error_empty_directive.conf
	@echo "\n============================================"
	@echo "Error configuration tests COMPLETED"
	@echo "============================================\n"

# Test toutes les configurations
test-all: test-valid test-errors
	@echo "\n============================================"
	@echo "ALL TESTS COMPLETED"
	@echo "============================================\n"

# Test avec valgrind
test-valgrind: fclean
	@$(MAKE) FLAGS="$(FLAGS) -DDEBUG=1 -g"
	@echo "\n============================================"
	@echo "Testing with Valgrind"
	@echo "============================================\n"
	@echo "--- Testing valid_basic.conf ---"
	valgrind --leak-check=full --show-leak-kinds=all ./$(NAME) config/valid_basic.conf
	@echo "\n--- Testing valid_complex.conf ---"
	valgrind --leak-check=full --show-leak-kinds=all ./$(NAME) config/valid_complex.conf
	@echo "\n--- Testing error_missing_semicolon.conf (should fail gracefully) ---"
	-valgrind --leak-check=full --show-leak-kinds=all ./$(NAME) config/error_missing_semicolon.conf
	@echo "\n--- Testing error_duplicate_port.conf (should fail gracefully) ---"
	-valgrind --leak-check=full --show-leak-kinds=all ./$(NAME) config/error_duplicate_port.conf
	@echo "\n============================================"
	@echo "Valgrind tests COMPLETED"
	@echo "============================================\n"

.PHONY: all clean fclean run re debug test-valid test-errors test-all test-valgrind
