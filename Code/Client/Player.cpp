#include "Player.h"
#include <time.h>

Player::Player() : CircleShape(5.0f)
{
	srand(static_cast<unsigned int>(time(NULL) + rand() % 255));
	setFillColor(sf::Color(rand() % 226 + 30, rand() % 226 + 30, rand() % 226 + 30));
}

Player::~Player()
{

}

void Player::setPos(const unsigned int _x, const unsigned int _y)
{
	data.x_pos = _x;
	data.y_pos = _y;
	setPosition(sf::Vector2f((float)_x * grid_scale, (float)_y * grid_scale));
}
