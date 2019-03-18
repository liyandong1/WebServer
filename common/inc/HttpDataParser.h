
#ifndef _HTTPDATAPARSER_H_
#define _HTTPDATAPARSER_H_

#include <vector>
#include <string>
#include "noncopyable.h"

struct HttpRequest
{
	std::string method;
	std::string url;
	std::string version;
	std::string host;
	std::string connection;
	std::vector<std::string> accepts;
};

class HttpDataParser:public noncopyable
{
private:
	std::string rawRequest_;
	HttpRequest parseResult_;
	size_t resourceLen_;
	std::vector<char> currentResourse_;
	std::string headerMessage_;
	void parse();
	void readResourse(const std::string& type = ".html");
	void constructHeaderMessage(const std::string& type = ".html");
public:
	HttpDataParser();
	void process(const std::string& text);
	std::vector<char> getResult();
	~HttpDataParser();
};

#endif