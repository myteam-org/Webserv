NAME	= webserv
CPP	= c++
CPPFLAG	= -Wall -Wextra -Werror -std=c++98 
DBGFLAGS := -g
# CPPFLAG += -g -fsanitize=address
INC_DIR	= include
SRCS	= 	src/config/tokenizer.cpp \
	src/config/context/locationContext.cpp \
	src/config/context/documentRootConfig.cpp \
	src/config/context/serverContext.cpp \
	src/config/validator.cpp \
	src/config/config.cpp \
	src/config/testPrint.cpp \
	src/config/token.cpp \
	src/config/parser.cpp \
	src/utils/logger.cpp \
	src/utils/string.cpp \
	src/utils/ip.cpp \
	src/http/method.cpp \
	src/http/request/read/utils.cpp \
	src/http/request/read/body.cpp \
	src/http/request/read/chunked_body.cpp \
	src/http/request/read/header.cpp \
	src/http/request/read/header_parsing_utils.cpp \
	src/http/request/read/length_body.cpp \
	src/http/request/read/line.cpp \
	src/http/request/read/context.cpp \
	src/http/request/read/reader.cpp \
	src/http/request/parse/request_parser.cpp \
	src/http/request/request.cpp \
	src/http/handler/file/delete.cpp \
	src/http/handler/file/redirect.cpp \
	src/http/handler/file/static.cpp \
	src/http/handler/file/cgi_env.cpp \
	src/http/handler/file/cgi_execute.cpp \
	src/http/handler/file/cgi_parse.cpp \
	src/http/handler/file/cgi_target.cpp \
	src/http/handler/file/cgi.cpp \
	src/http/handler/file/upload.cpp \
	src/http/handler/file/fileOrCgi.cpp \
	src/http/handler/router/builder.cpp \
	src/http/handler/router/internal.cpp \
	src/http/handler/router/middleware/chain.cpp \
	src/http/handler/router/middleware/error_page.cpp \
	src/http/handler/router/middleware/logger.cpp \
	src/http/handler/router/registry.cpp \
	src/http/handler/router/router.cpp \
	src/http/mime.cpp \
	src/http/response/builder.cpp \
	src/http/response/response.cpp \
	src/http/response/response_headers.cpp \
	src/http/status.cpp \
	src/http/virtual_server.cpp \
	src/action/cgi_action.cpp \
	src/action/cgi_context.cpp \
	src/http/config/config_resolver.cpp \
	src/io/base/fileDescriptor.cpp \
	src/io/input/read/buffer.cpp \
	src/io/input/reader/fd.cpp \
	src/io/input/write/buffer.cpp \
	src/io/input/writer/fd.cpp \
	src/io/handler/CgiStdinHandler.cpp \
	src/io/handler/CgiStdoutHandler.cpp \
	src/io/handler/ListenerHandler.cpp \
	src/io/handler/ClientHandler.cpp \
	src/server/socket/connectionSocket.cpp \
	src/server/socket/serverSocket.cpp \
	src/server/socket/socketAddr.cpp \
	src/server/connection/connectionManager.cpp \
	src/server/connection/connection.cpp \
	src/server/epollEvent.cpp \
	src/server/fileDescriptor/fdRegistry.cpp \
	src/server/fileDescriptor/fdUtils.cpp \
	src/server/dispatcher/requestDispatcher.cpp \
	src/server/epollEventNotifier.cpp \
	src/server/resolver/endpointResolver.cpp \
	src/server/server.cpp \
	src/main.cpp

OBJS	= $(SRCS:.cpp=.o)

debug: CPPFLAG += $(DBGFLAGS)
debug: $(NAME)

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
