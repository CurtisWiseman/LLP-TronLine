#include <SDKDDKVer.h>
#include <future>
#include "..\Common\MessageTypes.h"
#include "..\Common\PlayerData.h"
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include "Player.h"
#include "..\Common\GameGrid.h"

//Constants
const sf::IpAddress SERVER_IP("127.0.0.1");
constexpr int SERVER_TCP_PORT(53000);
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 720;

//Aliases
using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClientPtr>;

//Functions
void input(TcpClient&);
void disconnectFromServer(TcpClient&);
void draw();
void client(TcpClient&);
void recieveMessage(TcpClient&);
void processMsg(TcpClient&, sf::Packet&);

//Globals
sf::Uint8 clientID = -1;
sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "TRON");
std::vector<std::unique_ptr<Player>> players;
std::unique_ptr<GameGrid> m_grid = nullptr;
std::mutex m_mutex;
std::atomic_bool running = false;

//Stream operators
sf::Packet& operator <<(sf::Packet& _packet, PlayerData& _data)
{
	return _packet << _data.x_pos << _data.y_pos << _data.dir << _data.id;
}

sf::Packet& operator >>(sf::Packet& _packet, PlayerData& _data)
{
	if (_packet >> _data.x_pos >> _data.y_pos >> _data.dir >> _data.id)
	{}
	
	return _packet;
}

int main()
{
	running = true;
	grid_scale = 1.0f;
	sf::TcpSocket socket;

	auto handle = std::async(std::launch::async, &client, std::ref(socket));

	sf::Event event;
	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				disconnectFromServer(socket);
				running = false;
				return 0;
			}
		}
		draw();
	}

	running = false;
	return 0;
}

void client(TcpClient& socket)
{
	auto status = socket.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		return;
	}

	auto handle = std::async(std::launch::async, &recieveMessage, std::ref(socket));

	return input(socket);
}

void input(TcpClient& socket)
{
	sf::Keyboard m_keyboard;
	sf::Packet pack;
	sf::Uint8 msg;

	sf::Keyboard::Key m_up = sf::Keyboard::Key::Up;
	sf::Keyboard::Key m_down = sf::Keyboard::Key::Down;
	sf::Keyboard::Key m_left = sf::Keyboard::Key::Left;
	sf::Keyboard::Key m_right = sf::Keyboard::Key::Right;

	while (clientID > 8 && running)
	{
		//Loop until client id has been retrieved
	}
	switch (clientID % 4)
	{
	case 0:
		m_up = sf::Keyboard::Key::Up;
		m_down = sf::Keyboard::Key::Down;
		m_left = sf::Keyboard::Key::Left;
		m_right = sf::Keyboard::Key::Right;
		break;
	case 1:
		m_up = sf::Keyboard::Key::W;
		m_down = sf::Keyboard::Key::S;
		m_left = sf::Keyboard::Key::A;
		m_right = sf::Keyboard::Key::D;
		break;
	case 2:
		m_up = sf::Keyboard::Key::I;
		m_down = sf::Keyboard::Key::K;
		m_left = sf::Keyboard::Key::J;
		m_right = sf::Keyboard::Key::L;
		break;
	case 3:
		m_up = sf::Keyboard::Key::T;
		m_down = sf::Keyboard::Key::G;
		m_left = sf::Keyboard::Key::F;
		m_right = sf::Keyboard::Key::H;
		break;
	}
	
	while (running)
	{
		
		pack.clear();
		if (m_keyboard.isKeyPressed(m_up))
		{
			msg = PlayerDirection::UP;
			pack << NetMsg::PLAYERDIRECTION << msg;
			socket.send(pack);
		}
		else if (m_keyboard.isKeyPressed(m_down))
		{
			msg = PlayerDirection::DOWN;
			pack << NetMsg::PLAYERDIRECTION << msg;
			socket.send(pack);
		}
		else if (m_keyboard.isKeyPressed(m_left))
		{
			msg = PlayerDirection::LEFT;
			pack << NetMsg::PLAYERDIRECTION << msg;
			socket.send(pack);
		}
		else if (m_keyboard.isKeyPressed(m_right))
		{
			msg = PlayerDirection::RIGHT;
			pack << NetMsg::PLAYERDIRECTION << msg;
			socket.send(pack);
		}
	}
	return;
}

void disconnectFromServer(TcpClient& socket)
{
	sf::Packet pack;
	pack << NetMsg::DISCONNECT;
	socket.send(pack);
}

void draw()
{
	window.clear();
	if (m_grid.get() != nullptr && m_grid.get()->getValByIndex(0) > -1 && running)
	{
		sf::RectangleShape rect;

		float window_x = window.getSize().x;
		float grid_x = m_grid->getWidth();
		float rect_x = window_x / grid_x;

		float window_y = window.getSize().y;
		float grid_y = m_grid->getHeight();
		float rect_y = window_y / grid_y;

		rect.setSize(sf::Vector2f(rect_x, rect_y));

		for (sf::Uint16 i = 0; i < m_grid->getHeight(); i++)
		{
			for (sf::Uint16 j = 0; j < m_grid->getWidth(); j++)
			{
				rect.setPosition(sf::Vector2f(j*rect_x, i*rect_y));
				
				switch (m_grid->getVal(j, i))
				{
				case 255:
					break;
				case 254:
					rect.setFillColor(sf::Color::White);
					window.draw(rect);
					break;
				default:
					sf::Uint16 player_to_use = m_grid->getVal(j, i);
					sf::Color players_colour;
					m_mutex.lock();
					if (players.size() > player_to_use)
					{
						players_colour = players[player_to_use]->getFillColor();
					}
					else
					{
						players_colour = sf::Color::Blue;
					}
					m_mutex.unlock();
					
					rect.setFillColor(players_colour);
					window.draw(rect);
					break;
				}
			}
		}
	}
	window.display();
}

void recieveMessage(TcpClient& socket)
{
	//Track the status of the socket
	sf::Socket::Status status;

	do
	{
		sf::Packet packet;
		status = socket.receive(packet);
		if (status == sf::Socket::Done)
		{
			auto handle = std::async(std::launch::async, &processMsg, std::ref(socket), std::ref(packet));
		}
	} while (status != sf::Socket::Disconnected && running);
}

void processMsg(TcpClient& socket, sf::Packet& packet)
{
	if (running)
	{
		int header = 0;
		packet >> header;

		NetMsg msg = static_cast<NetMsg>(header);
		if (msg == NetMsg::GRID)
		{
			sf::Uint16 _grid_width, _grid_height;
			packet >> _grid_width >> _grid_height;
			sf::Uint8 _grid_vals;

			if (m_grid.get() == nullptr)
			{
				m_grid = std::make_unique<GameGrid>(_grid_width, _grid_height);
				grid_scale = window.getSize().x / _grid_width;
			}

			for (int i = 0; i < _grid_width*_grid_height; i++)
			{
				packet >> _grid_vals;
				m_grid->setValByIndex(i, _grid_vals);
			}
		}
		else if (msg == NetMsg::PLAYERDATA)
		{
			PlayerData _pd;
			if (packet >> _pd)
			{
				//okay
				bool playerexists = false;
				m_mutex.lock();
				for (unsigned int i = 0; i < players.size(); i++)
				{
					if (players[i]->getID() == _pd.id)
					{
						playerexists = true;
						players[i]->setPos(_pd.x_pos, _pd.y_pos);
						players[i]->setDirection(static_cast<PlayerDirection>(_pd.dir));
					}
				}
				m_mutex.unlock();
				if (playerexists == false)
				{
					std::unique_ptr<Player> temp = std::make_unique<Player>();
					temp->setID(_pd.id);
					temp->setDirection(static_cast<PlayerDirection>(_pd.dir));
					temp->setPos(_pd.x_pos, _pd.y_pos);
					players.push_back(std::move(temp));
				}
			}
			else
			{
				//failed
			}

		}
		else if (msg == NetMsg::PING)
		{
			sf::Packet pongMsg;
			pongMsg << NetMsg::PONG;
			socket.send(pongMsg);
		}
		else if (msg == NetMsg::CONNECT)
		{
			PlayerData _pd;
			if (packet >> _pd)
			{
				clientID = _pd.id;
				std::unique_ptr<Player> temp = std::make_unique<Player>();
				temp->setID(_pd.id);
				temp->setDirection(static_cast<PlayerDirection>(_pd.dir));
				temp->setPos(_pd.x_pos, _pd.y_pos);
				players.push_back(std::move(temp));
			}
		}
		else if (msg == NetMsg::DISCONNECT)
		{
			sf::Uint8 other_id;
			packet >> other_id;

			m_mutex.lock();
			for (auto& player : players)
			{
				if (player->getID() == other_id)
				{
					players.erase(std::remove(players.begin(), players.end(), player), players.end());
					break;
				}
			}
			m_mutex.unlock();
		}
	}
}