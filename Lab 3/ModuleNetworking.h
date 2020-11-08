#pragma once



class ModuleNetworking : public Module
{
private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool init() override;

	bool preUpdate() override;

	bool cleanUp() override;


	//////////////////////////////////////////////////////////////////////
	// Socket event callbacks
	//////////////////////////////////////////////////////////////////////

	virtual bool isListenSocket(SOCKET socket) const { return false; }

	virtual void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) { }

	virtual void onSocketReceivedData(SOCKET s, const InputMemoryStream &packet) = 0;

	virtual void onSocketDisconnected(SOCKET s) = 0;

protected:

	virtual bool sendPacket(const OutputMemoryStream& packet, SOCKET socket);

	//////////////////////////////////////////////////////////////////////
	// Chat Messages
	//////////////////////////////////////////////////////////////////////

	std::map<std::string, ImVec4> colors =
	{
		{"White"	, ImColor(0xFFFFFF)},
		{"Red"		, ImColor(0xFF0000)},
		{"Lime"		, ImColor(0x00FF00)},
		{"Blue"		, ImColor(0x0000FF)},
		{"Yellow"	, ImColor(0xFFFF00)},
		{"Cyan"		, ImColor(0x00FFFF)},
		{"Magenta"	, ImColor(0xFF00FF)},
		{"Silver"	, ImColor(0xC0C0C0)},
		{"Gray"		, ImColor(0x808080)},
		{"Maroon"	, ImColor(0x800000)},
		{"Olive"	, ImColor(0x808000)},
		{"Green"	, ImColor(0x008000)},
		{"Purple"	, ImColor(0x800080)},
		{"Teal"		, ImColor(0x008080)},
		{"Navy"		, ImColor(0x000080)}
	};



protected:

	std::vector<SOCKET> sockets;

	void addSocket(SOCKET socket);

	void disconnect();

	static void reportError(const char *message);
};


