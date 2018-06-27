#pragma once
#include <SFML\Network.hpp>

//struct PlayerData
//{
//	unsigned int xPos		: 6;
//	unsigned int yPos		: 6;
//	unsigned int ID			: 6;
//	unsigned int dir		: 2;
//	unsigned				: 0;
//};

enum PlayerDirection : sf::Uint8
{
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3
};

struct PlayerData
{
	sf::Uint8 x_pos;
	sf::Uint8 y_pos;
	sf::Uint8 dir;
	sf::Uint8 id;
};