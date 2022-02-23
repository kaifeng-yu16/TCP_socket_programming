#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "socket.cpp"
#include "potato.h"

class RingMaster {
public:
  RingMaster(int port, int p_num, int hops);
  int start_game();
  void print_info();
private:
  std::vector<int> player_sock;
  std::vector<int> player_port;
  std::vector<std::string> player_host;
  int player_num;
  int total_hops;
  int port;
  
  void play();
  void end_game();
};
