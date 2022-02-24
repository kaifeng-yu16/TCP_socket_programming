#include "player.h"

Player::Player(std::string host, int port) : host(host), port(port) {
  connected_sock.resize(3);
}

int Player::start_game() {
  std::string port_str = std::to_string(port);
  int socket_fd = client_init(host.c_str(), port_str.c_str());
  if (socket_fd == -1) {
    std::cerr << "Can not init client for player\n";
    return -1;
  }
  connected_sock[0] = socket_fd;
  
  // set up as a server inorder to provide port num to neighbour
  int listensocket_fd = server_init("0");
  if (listensocket_fd == -1) {
    std::cerr << "Can not init server for listening\n";
    return -1;
  }
  // get local host & port
  if (send_info_to_master(listensocket_fd) == -1) {
    return -1;
  }
  //  Recieve neighbour & player info
  if (receive_info_from_master() == -1) {
    return -1;
  }
  //std::cout << "id = " << id << " num players = " << num_players << " host = " << host << " port = " << port << " host size = " << host.size() <<  std::endl;
  // connecting to neighbours
  std::string t_port = std::to_string(port);
  int rsocket_fd = client_init(host.c_str(), t_port.c_str());
  if (socket_fd == -1) {
    std::cerr << "Can not init client for rightneighbour\n";
    return -1;
  }
  int lsocket_fd = accept_client(listensocket_fd);
  if (lsocket_fd == -1) {
    std::cerr << "Can not accept leftneightbour\n";
    return -1;
  }
  close(listensocket_fd);
  connected_sock[1] = rsocket_fd;
  connected_sock[2] = lsocket_fd;
  //std::cout << "successfully connected\n"; 
  std::cout << "Connected as player " << id << " out of " << num_players << " total players\n";
  srand((unsigned int)time(NULL) + id);
  // waiting for potatos
  send_potato();
  return 0;
}

int Player::send_info_to_master(int listensocket_fd) {
  char localhost[1024];
  int rtn = gethostname(localhost, 1023);
  if (rtn != 0) {
    std::cerr << "Can not get local host for player\n";
    return -1;
  }
  localhost[1023] = 0;
  //std::cout << "size of local host: "  << strlen(localhost) << std::endl; 
  struct sockaddr_in socket_addr;
  unsigned int socket_addr_len = sizeof(socket_addr);
  rtn = getsockname(listensocket_fd, (struct sockaddr*)&socket_addr, &socket_addr_len);
  if (rtn != 0) {
    std::cerr << "Can not get local port for player\n";
    return -1;
  }
  int localport = ntohs(socket_addr.sin_port);
  send(connected_sock[0], (char*)&localport, sizeof(int), 0);
  send(connected_sock[0], localhost, 1024, 0);
  return 0;
}

int Player::receive_info_from_master() {
  //  Recieve neighbour & player info
  recv(connected_sock[0], (char *)&id, sizeof(int), MSG_WAITALL);
  recv(connected_sock[0], (char *)&num_players, sizeof(int), MSG_WAITALL);
  recv(connected_sock[0], (char *)&port, sizeof(int), MSG_WAITALL);
  char buffer[1024];
  int size = recv(connected_sock[0], buffer, 1024, MSG_WAITALL);
  buffer[1023] = 0;
  if (size == 0 || size == -1) {
      std::cerr << "Error happend when player is receiving host of neighbour\n";
      return -1;
  }
  // printf("size = %d\n", size);
  buffer[size] = 0;
  host = std::string(buffer);
  return 0;
}

void Player::send_potato() {
  // waiting for potatos
  Potato potato;
  fd_set readfds;
  int max_ele = *max_element(connected_sock.begin(), connected_sock.end());
  while(1) {
    FD_ZERO(&readfds);
    for (int i = 0; i < 3; ++i) {
      FD_SET(connected_sock[i], &readfds);
    }
    //std::cout << "befor select\n";
    select(max_ele + 1, &readfds, NULL, NULL, NULL);
    //std::cout << "after select\n";
    for (int i = 0; i < 3; ++i) {
      if (FD_ISSET(connected_sock[i], &readfds)) {
        int size_recv = recv(connected_sock[i], (char *)&potato, sizeof(potato), MSG_WAITALL);
        //std::cout << "recv size: " << size_recv << std::endl;
        if (potato.hops == 0 || size_recv == 0) {
          for (int j = 0; j < 3; ++j) {
            close(connected_sock[j]);
          }
          return;
        }
        if (potato.hops == 1) {
          potato.hops -= 1;
          potato.trace[potato.hops] = id;
          //printf("potato hops = %d, trace = %d\n", potato.hops, potato.trace[potato.hops]);
          std::cout << "I'm it\n";
          send(connected_sock[0], (char *)&potato, sizeof(potato), 0);
        } else {
          potato.hops -= 1;
          potato.trace[potato.hops] = id;
          int random = rand() % 2;
          if (random == 0) {
            if (id != 0) {
              std::cout << "Sending potato to " << id - 1 << std::endl;
              send(connected_sock[2], (char *)&potato, sizeof(potato), 0);
            } else {
              std::cout << "Sending potato to " << num_players - 1 << std::endl;
              send(connected_sock[2], (char *)&potato, sizeof(potato), 0);
            }
          } else {
            if (id != num_players - 1) {
              std::cout << "Sending potato to " << id + 1 << std::endl;
              send(connected_sock[1], (char *)&potato, sizeof(potato), 0);
              
            } else {
              std::cout << "Sending potato to " << 0 << std::endl;
              send(connected_sock[1], (char *)&potato, sizeof(potato), 0);
            }
          }
        }
      }
    }
  }
}

void print_help() {
  std::cout << "Please provide following input:\n./playerr <machine_name> <port_num>\n";
}

int main(int argc, char** argv) {
  if (argc != 3) {
    print_help();
    return -1;
  }
  std::string host;
  int port;
  try {
    host = std::string(argv[1]);
    port = std::stoi(argv[2]);
  }
  catch (std::exception &e) {
    std::cout << "Invalid input!\n";
    print_help();
    return -1;
  }
  if (port < 0) {
    std::cout << "Invalid input!\n";
    print_help();
    return -1;
  }
  Player player(host, port);
  return player.start_game();
}
