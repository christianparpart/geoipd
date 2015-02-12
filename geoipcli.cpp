#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char**argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: geoipcli SERVER_IP LOOKUP_IP\n");
    return 1;
  }

  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  // timeout half a second (500k microseconds)
  struct timeval timeout { .tv_sec = 0, .tv_usec = 500000 };
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
  sendto(fd, query, strlen(query), 0, (sockaddr *)&saddr, sizeof(saddr));
  int n = recvfrom(fd, resp, sizeof(resp), 0, nullptr, nullptr);
  resp[n] = '\n';
  resp[n + 1] = 0;
  fputs(resp, stdout);
#endif

  return 0;
}
