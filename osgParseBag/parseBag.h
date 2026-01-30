#ifndef PARSEBAG_H
#define PARSBAG_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

//#define MAX_LEN 65536
#define MAX_LEN 200000

enum Options {
	None_0,
	None_1,
	Message_2,
	BAGHEAD_3,
	INDEXDATA_4,
	CHUNK_5,
	CHUNKINFO_6,
	CONNECTION_7
};

class ParseBag {
public:
	ParseBag();
	virtual ~ParseBag();

	static ParseBag* getInstance() {
		if (instance == nullptr) {
			instance = new ParseBag();
		}
		return instance;
	}

	void parseBag(const std::string& sFileName);

private:
	int readIMU(std::ifstream& ifs);
	int readImage(std::ifstream& ifs);
	int readPoint(std::ifstream& ifs);

	int readHeader(std::ifstream& ifs, bool bBagHeader = false);
	int readData(std::ifstream& ifs);

	int toInt(unsigned char* buf, int st, int len);
	int toString(unsigned char* buf, int st, int len, std::string& name, std::string& value);

	int readHeader(std::FILE* file);
private:
	static ParseBag* instance;

	unsigned char clen[4];
	unsigned char buffer[MAX_LEN];
	std::unordered_map<std::string, std::string> mapHeaderAttribute;
	std::unordered_set<std::string> setHeaderString = { "compression", "topic", "callerid", "md5sum", "message_definition", "type" };
	std::vector<int> counts;
	size_t nIdx = 0;
};

#endif