#include "..\Common\GameGrid.h"

GameGrid::GameGrid(sf::Uint16 _x, sf::Uint16 _y)
{
	m_width = _x;
	m_height = _y;
	m_grid.reserve(m_width*m_height);
	for (int i = 0; i < m_width*m_height; i++)
	{
		m_grid.push_back(255);
	}
}

GameGrid::~GameGrid()
{

}

sf::Uint8 GameGrid::getVal(sf::Uint16 _x, sf::Uint16 _y)
{
	if (_x < 0)
	{
		_x = 0;
	}
	if (_y < 0)
	{
		_y = 0;
	}
	if (_x > m_width -1)
	{
		_x = m_width -1;
	}
	if (_y > m_height -1)
	{
		_y = m_height -1;
	}

	sf::Uint32 index = (_y*m_width) + _x;
	return m_grid[index];
}

sf::Uint8 GameGrid::getValByIndex(sf::Uint32 _index)
{
	if (_index < 0)
	{
		_index = 0;
	}
	if (_index > (m_width*m_height) - (unsigned int)1)
	{
		_index = m_width*m_height - 1;
	}
	
	return m_grid[_index];
}

void GameGrid::setVal(sf::Uint16 _x, sf::Uint16 _y, sf::Uint8 _val)
{
	if (_x < 0)
	{
		_x = 0;
	}
	if (_y < 0)
	{
		_y = 0;
	}
	if (_x > m_width - 1)
	{
		_x = m_width - 1;
	}
	if (_y > m_height - 1)
	{
		_y = m_height - 1;
	}

	sf::Uint32 index = (_y*m_width) + _x;
	m_grid[index] = _val;
}

void GameGrid::setValByIndex(sf::Uint32 _index, sf::Uint8 _val)
{
	if (_index < 0)
	{
		_index = 0;
	}
	if (_index > m_width*m_height - (unsigned int)1)
	{
		_index = m_width*m_height - 1;
	}
	
	m_grid[_index] = _val;
}