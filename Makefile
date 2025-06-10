NAME	= webserv
CPP	= c++
CPPFLAG	= -Wall -Wextra -Wall -std=c++98 -pedantic
# CPPFLAG += -g -fsanitize=address
INC_DIR	= include
SRCS	= src/main.cpp \
	  src/config/Config.cpp \
	  src/config/ServerContext.cpp \
	  src/validator/Validator.cpp \
	  src/config/ConfigTokenizer.cpp \
	  src/config/ConfigParser.cpp \
	  src/config/Token.cpp

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