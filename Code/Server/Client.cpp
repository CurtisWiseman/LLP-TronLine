#include "Client.h"
#include "..\Common\MessageTypes.h"

sf::Uint8 Client::next_id = 0;

//Construction and assignment

Client::Client(Client&& rhs)
{
	data = rhs.getData();
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
	this->timestamp = rhs.timestamp;
	this->failed_pings = rhs.failed_pings;
	this->connected = rhs.connected;
}

Client& Client::operator=(Client&& rhs)
{
	data = rhs.getData();
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
	this->timestamp = rhs.timestamp;
	this->failed_pings = rhs.failed_pings;
	this->connected = rhs.connected;
	return *this;
}

Client::Client(sf::TcpSocket* _socket) : socket(_socket)
{
	connected = true;
	data.id = next_id++;
	data.dir = RIGHT;
	data.x_pos = 0;
	data.y_pos = 0;
}

//Functions

void Client::ping()
{
	sf::Packet pingMsg;
	pingMsg << NetMsg::PING;

	socket->send(pingMsg);

	timestamp = s_clock::now();
}

void Client::pong()
{
	auto end = s_clock::now();
	latency = std::chrono::duration_cast<microSecs>(end - timestamp);
	latency /= 2;
}
