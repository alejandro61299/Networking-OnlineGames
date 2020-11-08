#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	ChatMessage,
	ChatCommand,
};

enum class ServerMessage
{
	Welcome,
	ChatMessage
};

enum class CommandType
{
	help,
	list,
	kick,
	whisper,
	changeName,
	changeColor,
	clear,
};


// Chat ---------------------------

class ChatMessage {

public:

	enum class Type {
		Normal,
		Whisper,
		Server,
		Command,
		Unknown
	};

	ChatMessage() {};
	ChatMessage(std::string text, Type type, std::string color, std::string ownerName = "", std::string time = "")
		: text(text), type(type) , color(color), ownerName(ownerName) {};


	void Write(OutputMemoryStream& stream) {
		stream << text;
		stream << type;
		stream << ownerName;
		stream << time;
		stream << color;
	};

	void Read(const InputMemoryStream& stream) {
		stream >> text;
		stream >> type;
		stream >> ownerName;
		stream >> time;
		stream >> color;
	}
	
	std::string text;
	Type type = Type::Unknown;
	std::string color;
	std::string ownerName;
	std::string time;
};

std::vector<std::string> Split(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}



