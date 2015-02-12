// This file is part of the "geoipd" project
//   (c) 2009-2014 Christian Parpart <trapni@gmail.com>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

void die(const char* msg) {
  fprintf(stderr, "Error. %s\n", msg);
  exit(1);
}

int main(int argc, char**argv) {
  if (argc != 3) die("usage: geoipcli SERVER_IP LOOKUP_IP");

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) die(strerror(errno));

  // timeout half a second (500k microseconds)
  struct timeval timeout { .tv_sec = 0, .tv_usec = 500 * 1000 };
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = inet_addr(argv[1]);
  saddr.sin_port = htons(1337);

  char resp[1000];
  const char* query = argv[2];
#if defined(BENCHMARK)
  for (uint64_t i = 0; i < 1000000; ++i) { // 1mio repeat
    sendto(fd, query, strlen(query), 0, (sockaddr *)&saddr, sizeof(saddr));
    recvfrom(fd, resp, sizeof(resp), 0, nullptr, nullptr);
  }
#else
  int n = sendto(fd, query, strlen(query), 0, (sockaddr *)&saddr, sizeof(saddr));
  if (n < 0) die(strerror(errno));
  n = recvfrom(fd, resp, sizeof(resp), 0, nullptr, nullptr);
  if (n < 0) die(strerror(errno));
  resp[n] = '\n';
  resp[n + 1] = 0;
  fputs(resp, stdout);
#endif

  close(fd);

  return 0;
}
