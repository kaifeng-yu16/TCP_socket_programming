#include "ringmaster.h"

RingMaster::RingMaster(int port, int p_num, int hops) :port(port), player_num(p_num), total_hops(hops) {
  player_sock.resize(player_num);
  player_port.resize(player_num);
  player_host.resize(player_num);
}

void RingMaster::print_info() {
  std::cout << "Potato Ringmaster\n";
  std::cout << "Players = " << player_num << std::endl;
  std::cout << "Hops = " << total_hops << std::endl;
}

int RingMaster::start_game() {
  srand ((unsigned int)time(NULL) + player_num);
  std::string port_str = std::to_string(port);
  int socket_fd = server_init(port_str.c_str());
  if (socket_fd == -1) {
    std::cerr << "Can not init server for ringmaster\n";
    return -1;
  }
  for (size_t i = 0; i < player_num; ++i) {
    // accept a player, fill player_sock
    int client_connection_fd = accept_client(socket_fd);
    if (client_connection_fd == -1) {
      std::cerr << "Can not accept client\n";
      return -1;
    }
    player_sock[i] = client_connection_fd;
    // receive port&host from this player, fill player_port&player_host
    int p_port;
    recv(client_connection_fd, (char*)&p_port, sizeof(int), MSG_WAITALL);
    char buffer[1024];
    int size = recv(client_connection_fd, buffer, 1024, MSG_WAITALL);
    //std::cout << "recieved size = " << size << std::endl;
    buffer[1023] = 0;
    if (size == 0 || size == -1) {
      std::cerr << "Error happend when ringmaster is receiving host from player\n";
      return -1;
    }
    buffer[size] = 0;
    player_host[i] = std::string(buffer);
    player_port[i] = p_port;
    //std::cout << "RingMaster recieve host = " << player_host[i] << "   port = " << player_port[i] <<  "  from player " << i << std::endl;
    std::cout << "Player " << i << " is ready to play\n"; 
  }
  close(socket_fd);
  // send players their neighbour info & player id 
  for (int i = 0; i < player_num; ++i) {
    send(player_sock[i], (char*)&i, sizeof(int), 0);
    send(player_sock[i], (char*)&player_num, sizeof(int), 0);
    if (i == player_num - 1)  {
      send(player_sock[i], (char*)&player_port[0], sizeof(int), 0);
      char send_buffer[1024];
      std::strcpy (send_buffer, player_host[0].c_str());
      int size = send(player_sock[i], send_buffer, 1024, 0);
      //printf("host size:%d\n", size);
    } else {
      send(player_sock[i], (char*)&player_port[i + 1], sizeof(int), 0);
      char send_buffer[1024];
      std::strcpy (send_buffer, player_host[i + 1].c_str());
      send(player_sock[i], send_buffer, 1024, 0);
    }
    //std::cout << "id = " << i << " player_num = " << player_num << " host size: " << player_host[i].size() << std::endl;
  }
  // play game
  if (total_hops > 0) {
    play();
  }
  // sending players empty potato to end game
  end_game();
  return 0; 
}

void RingMaster::play() {
  int random = rand() % player_num;
  Potato potato;
  memset(&potato, 0, sizeof(potato));
  potato.hops = total_hops;
  std::cout << "Ready to start the game, sending potato to player "<< random << std::endl;
  int size = send(player_sock[random], (char *)&potato, sizeof(potato), 0);
  //std::cout << "potato sended size: "<<size<<std::endl;
  fd_set readfds;
  FD_ZERO(&readfds);
  for (int i = 0; i < player_num; ++i) {
    FD_SET(player_sock[i], &readfds);
  }
  int max_ele = *max_element(player_sock.begin(), player_sock.end());
  select(max_ele + 1, &readfds, NULL, NULL, NULL);
  for (int i = 0; i < player_num; ++i) {
    if (FD_ISSET(player_sock[i], &readfds)) {
      recv(player_sock[i], (char *)&potato, sizeof(potato), MSG_WAITALL);
    }
  }
  std::cout << "Trace of potato:" << std::endl;
  for (int i = total_hops - 1; i >= 0; --i) {
    if (i == 0) {
      std::cout << potato.trace[i] << std::endl;
    } else {
      std::cout << potato.trace[i] << "," ;
    }
  }
}

void RingMaster::end_game() {
  Potato potato;
  memset(&potato, 0, sizeof(potato));
  potato.hops = 0;
  for (int i = 0; i < player_num; ++i) {
    send(player_sock[i], (char *)&potato, sizeof(potato), 0);
    close(player_sock[i]);
  }
  
}

void print_help() {
  std::cout << "Please provide following input:\n./ringmaster <port_num> <num_players> <num_hops>\n";
}

int main(int argc, char ** argv) {
  // sanity check
  if (argc != 4) {
    print_help();
    return -1;
  }
  int port, num_player, num_hops;
  try {
    port = std::stoi(argv[1]);
    num_player = std::stoi(argv[2]);
    num_hops = std::stoi(argv[3]);
  }
  catch (std::exception &e) {
    std::cout << "Invalid input!\n";
    print_help();
    return -1;
  }
  if (num_player <= 1 || num_hops < 0 || num_hops > 512 || port < 0) {
    std::cout << "Invalid input!\n";
    print_help();
    return -1;
  }
  RingMaster master(port, num_player, num_hops);
  master.print_info();
  return master.start_game();
}
