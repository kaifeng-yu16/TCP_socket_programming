#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include "socket.cpp"
#include "potato.h"

class Player {
public:
  Player(std::string host, int port);
  int start_game();
private:
  // three sockfd: 1.ringmaster 2. neighbour id + 1, which is the server 3. neighbour id - 1, which is the client
  std::vector<int> connected_sock;
  int id;
  int num_players;
  std::string host;
  int port;

  int receive_info_from_master();
  int send_info_to_master(int listendsocket_fd);
  void send_potato();
};
