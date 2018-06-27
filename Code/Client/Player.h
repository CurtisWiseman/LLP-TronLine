#pragma once
#include <SFML\Graphics.hpp>
#include "..\Common\PlayerData.h"

using Vector2 = sf::Vector2f;

static float grid_scale;

class Player : public sf::CircleShape
{
public:
	Player();
	~Player();

	//Getters
	unsigned int getID() { return data.id; }
	unsigned int getXPos() { return data.x_pos; }
	unsigned int getYPos() { return data.y_pos; }
	PlayerDirection getDirection() { return static_cast<PlayerDirection>(data.dir); }

	//Setters
	void setID(const unsigned int _id) { data.id = _id; }

	void setPos(const unsigned int _x, const unsigned int _y);

	void setDirection(const PlayerDirection _dir) { data.dir = _dir; }

	

private:
	PlayerData data;
};