// This file is part of the "geoipd" project
//   (c) 2009-2014 Christian Parpart <trapni@gmail.com>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>

static inline uint32_t make_ipnum(const char* ip) {
  in_addr addr;
  inet_aton(ip, &addr);
  return static_cast<uint32_t>(htonl(addr.s_addr));
}

struct IPRange {
  uint32_t first;
  uint32_t second;
  std::string country;

  IPRange(const char* a, const char* b, const char* c)
      : first(make_ipnum(a)), second(make_ipnum(b)), country(c) {}
  bool operator<(uint32_t ip) const { return second < ip; }
  bool operator>(uint32_t ip) const { return ip < first; }
};

typedef std::vector<IPRange> GeoipVec;

void die(const char* msg) {
  fprintf(stderr, "Error. %s\n", msg);
  exit(1);
}

void load(GeoipVec* geoip, const char* path) {
  std::ifstream ifs(path);
  if (ifs.good() == false) die("could not open geoip db");

  char line[256];
  while (ifs.getline(line, sizeof(line)) > 0) {
    char* a = strtok(line, ",");
    char* b = strtok(a + strlen(a) + 1, ",");
    char* c = b + strlen(b) + 1;
    geoip->emplace_back(a, b, c);
  }
}

const IPRange* lookup(const GeoipVec& geoip, const char* ip) {
  uint32_t ipn = make_ipnum(ip);
  size_t left = 0;
  size_t right = geoip.size() - 1;

  for (;;) {
    if (left > right) return nullptr;

    const size_t imid = left + (right - left) / 2;
    const IPRange& mid = geoip[imid];

    if (mid < ipn) left = imid + 1;
    else if (mid > ipn) right = imid - 1;
    else return &mid;
  }

  return nullptr;
}

int main(int argc, const char** argv) {
  GeoipVec geoip;
  load(&geoip, argc == 2 ? argv[1] : "geoip.csv"); // file must be sorted

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(1337);
  if (bind(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
    die(strerror(errno));

  for (;;) {
    sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buf[1024];
    int n = recvfrom(sockfd, buf, 1000, 0, (sockaddr *)&cliaddr, &len);
    if (n < 0) {
      perror("recvfrom");
    } else {
      buf[n] = '\0';
      if (const IPRange* res = lookup(geoip, buf)) {
        sendto(sockfd, res->country.data(), res->country.size(), 0, (sockaddr *)&cliaddr, sizeof(cliaddr));
      }
    }
  }

  return 0;
}
