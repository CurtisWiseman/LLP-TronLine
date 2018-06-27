#include <vector>
#include <SFML\Network.hpp>

class GameGrid
{
public:
	GameGrid(sf::Uint16 _x, sf::Uint16 _y);
	~GameGrid();

	//Getters
	std::vector<sf::Uint8> getGrid() { return m_grid; }
	sf::Uint8 getVal(sf::Uint16 _x, sf::Uint16 _y);
	sf::Uint8 getValByIndex(sf::Uint32 _index);
	sf::Uint16 getWidth() { return m_width; }
	sf::Uint16 getHeight() { return m_height; }

	//Setters
	void setVal(sf::Uint16 _x, sf::Uint16 _y, sf::Uint8 _val);
	void GameGrid::setValByIndex(sf::Uint32 _index, sf::Uint8 _val);

private:
	sf::Uint16 m_width = 80;
	sf::Uint16 m_height = 80;
	std::vector<sf::Uint8> m_grid;
};