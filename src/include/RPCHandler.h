#pragma once

#include <cstdint>
#include <string>

class MandelbrotWidget;

class RPCHandler
{
public:
	explicit RPCHandler(uint16_t port, MandelbrotWidget& widget);
	virtual ~RPCHandler() = default;

	virtual void StartRPCService() = 0;

	virtual void ShutdownService() {}

protected:
	static std::string LoadStringFromFile(std::string path);

	const uint16_t m_port;
	MandelbrotWidget& m_widget;

};
