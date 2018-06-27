// ChatServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <future>
#include <SFML\Network.hpp>
#include "..\Common\MessageTypes.h" 
#include "..\Common\PlayerData.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "Client.h"
#include "..\Common\GameGrid.h"

//Constants
constexpr int SERVER_TCP_PORT(53000);
constexpr int SERVER_UDP_PORT(53001);

//Aliases
using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;

//Functions
bool bindServerPort(sf::TcpListener&);
void listen(sf::TcpListener&, sf::SocketSelector&, TcpClients&, GameGrid&);
void connect(sf::TcpListener&, sf::SocketSelector&, TcpClients&, GameGrid&);
void recieveMsg(TcpClients&, sf::SocketSelector&, GameGrid&);
void ping(TcpClients&, GameGrid&, sf::SocketSelector&);
void runServer();
void runGame(TcpClients&, GameGrid&);
void broadcastPlayerData(TcpClients&, PlayerData&);
void broadcastGridData(TcpClients&, GameGrid&);
void spawnPlayer(Client&, GameGrid&);
void despawnPlayer(Client&, GameGrid&);

//Globals
std::atomic_bool running = true;

sf::Packet& operator <<(sf::Packet& _packet, PlayerData& _data)
{
	return _packet << _data.x_pos << _data.y_pos << _data.dir << _data.id;
}

sf::Packet& operator >>(sf::Packet& _packet, PlayerData& _data)
{
	return _packet >> _data.x_pos >> _data.y_pos >> _data.dir >> _data.id;
}

int main()
{
	runServer();
	running = false;
	return 0;
}

bool bindServerPort(sf::TcpListener& listener)
{
	if (listener.listen(SERVER_TCP_PORT) != sf::Socket::Done)
	{
		std::cout << "Could not bind server port";
		std::cout << std::endl << "Port: " << SERVER_TCP_PORT;
		std::cout << std::endl;
		return false;
	}

	std::cout << "Server launched on port: " << SERVER_TCP_PORT << std::endl;;
	std::cout << "Scanning for messages..." << std::endl;
	return true;
}

void listen(sf::TcpListener& listener, sf::SocketSelector& selector, TcpClients& clients, GameGrid& _grid)
{
	while (running)
	{
		const sf::Time timeout = sf::Time(sf::milliseconds(250));
		if (selector.wait(timeout))
		{
			if (selector.isReady(listener))
			{
				connect(listener, selector, clients, _grid);
			}
			else
			{
				recieveMsg(clients, selector, _grid);
				//auto handle = std::async(std::launch::async, recieveMsg, std::ref(clients), std::ref(selector), std::ref(_grid));
			}
		}
		else
		{
			ping(clients, _grid, selector);
		}
	}
}

void connect(sf::TcpListener& listener, sf::SocketSelector& selector, TcpClients& clients, GameGrid& _grid)
{
	auto client_ptr = new sf::TcpSocket;
	auto& client_ref = *client_ptr;
	if (listener.accept(client_ref) == sf::Socket::Done && clients.size() < 8)
	{
		selector.add(client_ref);

		auto client = Client(client_ptr);

		sf::Packet init_msg;
		init_msg << NetMsg::BLANK;
		client.getSocket().send(init_msg);
		spawnPlayer(client, _grid);
		
		sf::Packet pack_data;

		if (pack_data << NetMsg::CONNECT << client.getData())
		{}

		client.getSocket().send(pack_data);

		broadcastPlayerData(clients, client.getData());

		clients.push_back(std::move(client));
		std::cout << "Connected to client on port " << SERVER_TCP_PORT
			<< " with ID" << client.getClientID() << std::endl;
	}
}

void recieveMsg(TcpClients& senders, sf::SocketSelector& selector, GameGrid& _grid)
{
	for (auto& sender : senders)
	{
		auto& sender_ref = sender.getSocket();
		if (selector.isReady(sender_ref) && sender.isConnected())
		{
			sf::Packet packet;
			sender_ref.receive(packet);
			int header = 0;
			packet >> header;

			NetMsg msgType = static_cast<NetMsg>(header);
			if (msgType == NetMsg::PLAYERDIRECTION)
			{
				sf::Uint8 msg;
				packet >> msg;

				switch (static_cast<PlayerDirection>(msg))
				{
				case DOWN:
					if (sender.getDir() == LEFT || sender.getDir() == RIGHT)
					{
						sender.setDir(DOWN);
						std::cout << "Client ID" 
							<< sender.getClientID() << " direction DOWN" 
							<< std::endl;
					}
					break;
				case UP:
					if (sender.getDir() == LEFT || sender.getDir() == RIGHT)
					{
						sender.setDir(UP);
						std::cout << "Client ID"
							<< sender.getClientID() << " direction UP"
							<< std::endl;
					}
					break;
				case LEFT:
					if (sender.getDir() == UP || sender.getDir() == DOWN)
					{
						sender.setDir(LEFT);
						std::cout << "Client ID"
							<< sender.getClientID() << " direction LEFT"
							<< std::endl;
					}
					break;
				case RIGHT:
					if (sender.getDir() == UP || sender.getDir() == DOWN)
					{
						sender.setDir(RIGHT);
						std::cout << "Client ID"
							<< sender.getClientID() << " direction RIGHT"
							<< std::endl;
					}
					break;
				}
				
				//std::cout << "Message recieved: '" << msg << "' from client ID" << sender.getClientID() << std::endl;
				//std::cout << "Latency: " << sender.getLatency().count() << "us" << std::endl;
			}
			else if (msgType == NetMsg::PONG)
			{
				sender.pong();
			}
			else if (msgType == NetMsg::INVALID || msgType == NetMsg::DISCONNECT)
			{
				//DC
				std::cout << "Invalid/Disconnect message recieved from client ID"
					<< sender.getClientID() << ", disconnecting..." << std::endl;

				sf::Packet pack_data;

				if (pack_data << NetMsg::DISCONNECT << sender.getClientID())
				{
				}

				for (auto& client : senders)
				{
					client.getSocket().send(pack_data);
				}

				despawnPlayer(sender, _grid);
				sender.setConnected(false);
				selector.remove(sender.getSocket());
				senders.erase(std::remove(senders.begin(), senders.end(), sender), senders.end());
				break;
			}
		}
	}
}

void ping(TcpClients& tcp_clients, GameGrid& _grid, sf::SocketSelector& _selector)
{
	constexpr auto timeout = 10s;
	for (auto& client : tcp_clients)
	{
		if (client.isConnected())
		{
			const auto& timestamp = client.getPingTime();
			const auto now = std::chrono::steady_clock::now();
			auto delta = now - timestamp;
			if (delta > timeout)
			{
				client.pingFailed();
				std::cout << "Ping request sent to client ID" << client.getClientID() << std::endl;
				client.ping();
			}
			else
			{
				client.pingSuccess();
			}
			const int ping_out = 5;
			if (client.getFailedPings() > ping_out)
			{
				std::cout << "No response from client ID" << client.getClientID()
					<< " after " << ping_out << " pings. Disconnecting..." << std::endl;
				//Disconnect
				sf::Packet pack_data;

				if (pack_data << NetMsg::DISCONNECT << client.getClientID())
				{
				}

				for (auto& _client : tcp_clients)
				{
					_client.getSocket().send(pack_data);
				}
				despawnPlayer(client, _grid);
				client.setConnected(false);
				_selector.remove(client.getSocket());
				tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), client), tcp_clients.end());
			}
		}
	}
}

void runServer()
{
	sf::TcpListener listener;
	if (!bindServerPort(listener))
	{
		return;
	}

	sf::SocketSelector selector;
	selector.add(listener);

	TcpClients clients;
	GameGrid grid(64, 45);
	auto handle = std::async(std::launch::async, runGame, std::ref(clients), std::ref(grid));
	auto handle2 = std::async(std::launch::async, broadcastGridData, std::ref(clients), std::ref(grid));
	return listen(listener, selector, clients, grid);
}

void runGame(TcpClients& _players, GameGrid& _grid)
{
	const long double player_speed = 1;
	s_clock::time_point timestamp = s_clock::now();
	
	while (running)
	{
		s_clock::time_point end = s_clock::now();
		std::chrono::milliseconds milli_secs = std::chrono::duration_cast<std::chrono::milliseconds>(end - timestamp);
		if (milli_secs.count() >= 32)
		{
			timestamp = s_clock::now();
			for (auto& _player : _players)
			{

				switch (_player.getDir()) //Move the player in current direction
				{
				case UP:
					if (_player.getYPos() > 0)
					{
						_player.setPos(_player.getXPos(), _player.getYPos() - (player_speed));
					}
					break;
				case DOWN:
					if (_player.getYPos() < _grid.getHeight())
					{
						_player.setPos(_player.getXPos(), _player.getYPos() + (player_speed));
					}
					break;
				case LEFT:
					if (_player.getXPos() > 0)
					{
						_player.setPos(_player.getXPos() - (player_speed), _player.getYPos());
					}
					break;
				case RIGHT:
					if (_player.getXPos() < _grid.getWidth())
					{
						_player.setPos(_player.getXPos() + (player_speed), _player.getYPos());
					}
					break;
				}

				if (_grid.getVal(_player.getXPos(), _player.getYPos()) != 255) //Kill player if they hit a wall
				{
					//Player dead
					std::cout << "Player should be dead" << std::endl;
					//Remove player, send back to start area, remove all walls player made
					despawnPlayer(_player, _grid);
					spawnPlayer(_player, _grid);
				}

				_grid.setVal(_player.getXPos(), _player.getYPos(), (_player.getClientID())); //Place a wall in the player's new position

				broadcastPlayerData(_players, _player.getData());
			}
		}
	}
}

void broadcastPlayerData(TcpClients& _all_clients, PlayerData& _data_to_send)
{
	sf::Packet pack_data;

	if (pack_data << NetMsg::PLAYERDATA << _data_to_send)
	{}

	for (auto& client : _all_clients)
	{
		client.getSocket().send(pack_data);
	}
}

void broadcastGridData(TcpClients& _all_clients, GameGrid& _grid_to_send)
{
	sf::Packet pack_data;
	while (running)
	{
		pack_data.clear();
		if (pack_data << NetMsg::GRID << _grid_to_send.getWidth() << _grid_to_send.getHeight())
		{}

		for (int i = 0; i < _grid_to_send.getWidth() * _grid_to_send.getHeight(); i++)
		{
			pack_data << _grid_to_send.getGrid()[i];
		}

		for (auto& client : _all_clients)
		{
			client.getSocket().send(pack_data);
		}
	}
}

void spawnPlayer(Client& _client_to_spawn, GameGrid& _grid)
{
	while (running)
	{
		int x = rand() % _grid.getWidth();
		int y = rand() % _grid.getHeight();

		if (_grid.getVal(x - 1, y - 1) == 255 &&
			_grid.getVal(x, y - 1) == 255 &&
			_grid.getVal(x + 1, y - 1) == 255 &&
			_grid.getVal(x - 1, y) == 255 &&
			_grid.getVal(x, y) == 255 &&
			_grid.getVal(x + 1, y) == 255 &&
			_grid.getVal(x - 1, y + 1) == 255 &&
			_grid.getVal(x, y + 1) == 255 &&
			_grid.getVal(x + 1, y + 1) == 255)
		{
			_client_to_spawn.setPos(x, y);
			break;
		}
	}
}

void despawnPlayer(Client& _client_to_despawn, GameGrid& _grid)
{
	_client_to_despawn.setPos(0, 0);

	for (int i = 0; i < _grid.getWidth(); i++)
	{
		for (int j = 0; j < _grid.getHeight(); j++)
		{
			if (_grid.getVal(i, j) == _client_to_despawn.getClientID())
			{
				_grid.setVal(i, j, 255);
			}
		}
	}
}