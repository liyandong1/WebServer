
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include "Config.h"
#include "HttpDataParser.h"

extern Config* m_config;

HttpDataParser::HttpDataParser():rawRequest_(),currentResourse_(1024, '0'),
			resourceLen_(0),headerMessage_("")
{
	
}

void HttpDataParser::parse()
{
	size_t pos1 = rawRequest_.find_first_of(' ');
	parseResult_.method = rawRequest_.substr(0, pos1);//起始位置，字符个数
	
	size_t pos2 = rawRequest_.find_first_of(' ', pos1 + 1);
	parseResult_.url = rawRequest_.substr(pos1 + 1, pos2 - pos1);
	
	pos2 = rawRequest_.find_first_of(' ', pos1 + 1);
	pos1 = rawRequest_.find_first_of('\n', pos2 + 1);
	parseResult_.version = rawRequest_.substr(pos2 + 1, pos1 - pos2 - 1);
	
	pos1 = rawRequest_.find_first_of(' ', pos2 + 1);
	pos2 = rawRequest_.find_first_of('\n', pos1 + 1);
	parseResult_.host = rawRequest_.substr(pos1 + 1, pos2 - pos1 - 1);
	
	pos1 = rawRequest_.find_first_of(' ', pos2 + 1);
	pos2 = rawRequest_.find_first_of('\n', pos1 + 1);
	parseResult_.connection = rawRequest_.substr(pos1 + 1, pos2 - pos1 - 1);
}

void HttpDataParser::readResourse(const std::string& type)
{
	currentResourse_.clear();
	std::string path = m_config->getStr("ResoursePath");
	path = path.substr(0, path.size() - 1);
	if(type == ".html")
	{
		path = path + std::string("//test.html");
	}
	else if(type == ".jpg")
	{
		path = path + std::string("//index.jpg");
	}
	//根据路径读取数据
	int fd = ::open(path.c_str(), O_RDONLY);
	resourceLen_ = ::read(fd, &*currentResourse_.begin(), 1024);
	//std::cout << "resourceLen_ = " << resourceLen_ << std::endl;
	::close(fd);
}

void HttpDataParser::constructHeaderMessage(const std::string& type)
{
	headerMessage_.clear();
	if(type == ".html")
	{
		headerMessage_ += "HTTP/1.1 200 OK\r\n";
		headerMessage_ += "Content-Type: text/html\r\n\r\n";
	}
	else if(type == ".jpg")
	{
		headerMessage_ += "HTTP/1.1 200 OK\r\n";
		headerMessage_ += "Content-Type: image/jpeg\r\n\r\n";
	}
	else
	{
		headerMessage_ += "HTTP/1.1 404 NotFound\r\n";
	}
}


void HttpDataParser::process(const std::string& text)
{
	assert(text.size() > 0);
	rawRequest_ = text;
	parse();
	//std::cout << "method " << parseResult_.method << std::endl;
	//std::cout << "url " << parseResult_.url << std::endl;
	//std::cout << "version " << parseResult_.version << std::endl;
	//std::cout << "host " << parseResult_.host << std::endl;
	//std::cout << "connection " << parseResult_.connection << std::endl;
	std::string type = ".html";
	if(parseResult_.url.find(".jpg") != std::string::npos)
		type = ".jpg";
	
	readResourse(type);
	constructHeaderMessage(type);
}

std::vector<char> HttpDataParser::getResult()
{
	std::vector<char> ret;
	for(size_t i = 0; i < headerMessage_.size(); i ++)
		ret.push_back(headerMessage_[i]);
	for(size_t i = 0; i < resourceLen_; i ++) // /0
		ret.push_back(currentResourse_[i]);
	return ret;
}

HttpDataParser::~HttpDataParser()
{
	
}