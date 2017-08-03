#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "chess.hpp"

using namespace std;

class Game {
	vector<int> m_players;
	Chess 		m_game;

public:
	Game() {}
	
	void add(int fd) {
		m_players.push_back(fd);
	}
	
	void setup() {
		m_game.setup();
		int active = m_players[m_game.turn()];
		send("setup");
		send(active, "your turn");
	}
	
	void accept_move(int fd, string move) {
		int active = m_players[m_game.turn()];
		
		if (fd != active) {
			send(fd, "not your turn");
			return;
		}
		
		switch ( m_game.enter_move(move.c_str()) ) {
			case 0: case 1:
				send(move);
				break;
			case 2:
				send(move);
				break;
			default:
				send(fd, "invalid move");
		}

		int next = m_players[m_game.turn()];
		send(next, "your turn");
	}
	
	int player1() const { return m_players[0]; }
	int player2() const { return m_players[1]; }
	int other(int fd) const { return m_players[0] ^ m_players[1] ^ fd; }
	
private:
	void send(string response) {
		write(m_players[0], response.c_str(), response.size() + 1);
		write(m_players[1], response.c_str(), response.size() + 1);
	}
	
	void send(int fd, string response) {
		write(fd, response.c_str(), response.size() + 1);
	}
};

class Client {
	int		m_fd;
	Game	*m_game;

public:
	Client(int fd) { m_fd = fd; }
	
	void join_game(Game *game) {
		game->add(m_fd);
		m_game = game;
	}
	
	void make_move(string move) {
		m_game->accept_move(m_fd, move);
	}
	
	Game *game() const { return m_game; }
};

class Server {
	static const int s_max_events = 64;
	static const int s_max_clients = 32;
	static const int s_port = 3000;
	static const int s_listen = 10;
	Client			*m_clients[s_max_clients];
	vector<Game*>	m_games;
	Client			*m_waiting;

public:
	Server() {
		m_waiting = NULL;
		for (int i = 0; i < s_max_clients; i++) m_clients[i] = NULL;
	}

	void run() {
		int fd = create_socket();
		int efd = epoll_create1(0);
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = fd;
		epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
		for (;;) {
			struct epoll_event events[s_max_events];
			int nevents = epoll_wait(efd, events, s_max_events, -1);
			for (int n = 0; n < nevents; n++) {
				if (events[n].data.fd == fd) {
					int conn = accept(fd, NULL, NULL);
					ev.events = EPOLLIN;
					ev.data.fd = conn;
					epoll_ctl(efd, EPOLL_CTL_ADD, conn, &ev);
					on_connect(conn);
				} else {
					on_request(events[n].data.fd);
				}
			}
		}
	}

private:
	void on_connect(int fd) {
		if (fd >= s_max_clients) {
			write(fd, "server is full", 15);
			return;
		}
		
		Client *client = new Client(fd);
		m_clients[fd] = client;
		
		if (m_waiting == NULL) {
			m_waiting = client;
			return;
		}
		
		Game *game = new Game;
		m_waiting->join_game(game);
		client->join_game(game);
		m_games.push_back(game);
		game->setup();
		m_waiting = NULL;
	}
	
	void on_request(int fd) {
		char request[64];
		Client *client = m_clients[fd];
		if (read(fd, request, sizeof request) > 0) {
			client->make_move(request);
		} else {
			close(fd);
		}
	}

	static int create_socket() {
		struct sockaddr_in sa;
		int fd = socket(PF_INET, SOCK_STREAM, 0);
		sa.sin_family = AF_INET;
		sa.sin_port = htons(s_port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(fd, (struct sockaddr*) &sa, sizeof sa);
		listen(fd, s_listen);
		return fd;
	}
};

int main() {
	Server server;
	server.run();
	return 0;
}
