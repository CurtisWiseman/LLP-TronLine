#pragma once
#include <SFML\Network.hpp>

enum NetMsg : sf::Uint8
{
	INVALID = 0,
	DISCONNECT = 1,
	CONNECT = 2,
	PING = 3,
	PONG = 4,
	GRID = 5,
	PLAYERDATA = 6,
	PLAYERDIRECTION = 7,
	BLANK
};