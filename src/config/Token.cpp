#include "Token.hpp"

Token::Token(const std::string& text, int line)
	: text_(text), lineNumber_(line) {
	}

Token::~Token() {}

std::string	Token::getText() const {
	return (this->text_);
}

int	Token::getLineNumber() const {
	return (this->lineNumber_);
}

TokenType	Token::getType() {
	return (type_);
}

void		Token::setType_(std::string& text) {
	if (text == "root")
		this->type_ = ROOT;
	if (text == "server")
		this->type_ = SERVER;
	if (text == "location" || text == "location_back")
		this->type_ = LOCATION;
	if (text == "error_page")
		this->type_ = ERR_PAGE;
	if (text == "listen")
		this->type_ = LISTEN;
	if (text == "server_name")
		this->type_ = SERVER_NAME;
	if (text == "allow_method")
		this->type_ = METHOD;
	if (text == "index")
		this->type_ = INDEX;
	if (text == "client_max_body_size")
		this->type_ = MAX_SIZE;
	if (text == "autoindex")
		this->type_ = AUTOINDEX;
	if (text == "is_cgi")
		this->type_ = IS_CGI;
	if (text == "return")
		this->type_ = RETURN;
	if (text == "{" || text == "}")
		this->type_ = BRACE;
	this->type_ = VALUE;
}
