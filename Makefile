NAME	= webserv
CPP	= c++
CPPFLAG	= -Wall -Wextra -Wall -std=c++98 -pedantic
# CPPFLAG += -g -fsanitize=address
INC_DIR	= include
SRCS	= src/main.cpp \
	  src/config/config.cpp \
	  src/config/context/serverContext.cpp \
	  src/config/context/locationContext.cpp \
	  src/config/context/documentRootConfig.cpp \
	  src/config/validator.cpp \
	  src/config/tokenizer.cpp \
	  src/config/parser.cpp \
	  src/config/token.cpp

OBJS	= $(SRCS:.cpp=.o)

%.o: %.cpp
	$(CPP) $(CPPFLAG) -I$(INC_DIR) -c $< -o $@

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAG) -I$(INC_DIR) $(OBJS) -o $(NAME)

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all re clean fclean