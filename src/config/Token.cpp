#include "Token.hpp"

Token::Token(const std::string& text, int line)
	: text_(text), lineNumber_(line) {
		setType_(text);
	}

Token::~Token() {}

std::string	Token::getText() const {
	return (this->text_);
}

int	Token::getLineNumber() const {
	return (this->lineNumber_);
}

TokenType	Token::getType() const {
	return (this->type_);
}

void		Token::setType_(const std::string& text) {
	if (text == "root")
		this->type_ = ROOT;
	else if (text == "server")
		this->type_ = SERVER;
	else if (text == "location" || text == "location_back")
		this->type_ = LOCATION;
	else if (text == "error_page")
		this->type_ = ERR_PAGE;
	else if (text == "listen")
		this->type_ = LISTEN;
	else if (text == "server_name")
		this->type_ = SERVER_NAME;
	else if (text == "allow_method")
		this->type_ = METHOD;
	else if (text == "index")
		this->type_ = INDEX;
	else if (text == "client_max_body_size")
		this->type_ = MAX_SIZE;
	else if (text == "autoindex")
		this->type_ = AUTOINDEX;
	else if (text == "is_cgi")
		this->type_ = IS_CGI;
	else if (text == "return")
		this->type_ = RETURN;
	else if (text == "{" || text == "}")
		this->type_ = BRACE;
	else
		this->type_ = VALUE;
}
