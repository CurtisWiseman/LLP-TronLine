#pragma once

#include <SFML\Network.hpp>
#include <memory>
#include <chrono>
#include "..\Common\PlayerData.h"

using namespace std::chrono_literals;
using s_clock = std::chrono::steady_clock;
using microSecs = std::chrono::microseconds;

class Client
{
public:
	//Construction and assignment
	Client(sf::TcpSocket* _socket);
	Client(Client&& _other);
	Client& operator=(Client&& rhs);

	//Comparison
	bool operator==(const Client& rhs) { return data.id == rhs.getClientID(); }

	//Getters
	sf::TcpSocket& getSocket()			{ return *socket; }
	sf::Uint8 getClientID()		const { return data.id; }
	sf::Uint8 getXPos()				{ return data.x_pos; }
	sf::Uint8 getYPos()				{ return data.y_pos; }
	PlayerDirection getDir()			{ return static_cast<PlayerDirection>(data.dir); }
	PlayerData getData()				{ return data; }
	const auto& getLatency()		const { return this->latency; }
	const auto& getPingTime()		const { return timestamp; }
	sf::Uint8 getFailedPings()				{ return failed_pings; }
	bool isConnected()					{ return connected; }

	//Setters
	void setLatency(microSecs _ms)		{ latency = _ms; }
	void setConnected(bool _con)		{ connected = _con; }
	void setID(sf::Uint8 _id)		{ data.id = _id; }
	void setDir(PlayerDirection _dir)	{ data.dir = _dir; }
	void setPos(sf::Uint8 _x, sf::Uint8 _y) { data.x_pos = _x; data.y_pos = _y; }

	//Functions
	void ping();
	void pingFailed() { failed_pings++; }
	void pingSuccess() { failed_pings = 0; }
	void pong();

private:
	std::unique_ptr<sf::TcpSocket> socket = nullptr;
	static sf::Uint8 next_id;
	microSecs latency = 100us;
	s_clock::time_point timestamp = s_clock::now();
	sf::Uint8 failed_pings = 0;
	bool connected = false;

	PlayerData data;
};