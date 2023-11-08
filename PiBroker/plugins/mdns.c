#include "../broker.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int sock;
static struct sockaddr_in addr;
static socklen_t addrlen = sizeof(addr);

static int event_handler(broker_event_t* event, short revents) {
 if (POLLIN & revents) {
    int len = recvfrom(sock, event->data, event->data_len, 0, (struct sockaddr* )&addr, &addrlen);
    if (len <= 0) {
      return -1;
    }

    ((char* )event->data)[len] = 0;
    return 0;
  }
  
  return 1;
}

static int event_callback(void* data, uint32_t data_len) {
  ((char* )data)[data_len - 1] = 0;
  printf("POLLIN!\n");
  
  return 0;
}

int PluginInit(broker_event_loop_t* loop) {
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  int optval = 1;
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    printf("failed to set SOL_SOCKET SO_RESUEADDR\n");
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1)
    printf("failed to set SOL_SOCKET SO_RESUEADDR\n");
  if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1)
    printf("failed to set SOL_SOCKET SO_BROADCAST\n");
  
  struct ip_mreq group;
  group.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
  group.imr_interface.s_addr = htonl(INADDR_ANY);

  if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) == -1)
    printf("failed to set IPPROTO_IP IP_ADD_MEMBERSHIP\n");
  if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &optval, sizeof(optval)) == -1)
    printf("failed to set IPPROTO_IP IP_MULTICAST_LOOP\n");

  addr.sin_family = AF_INET;
  addr.sin_port = htons(5353);
  inet_pton(AF_INET, "224.0.0.251", &addr.sin_addr);
  
  if (bind(sock, (struct sockaddr* )&addr, sizeof(addr)) == -1) {
    printf("failed to bind\n");
    return 1;
  }

  broker_event_loop_add_event(loop, sock, event_handler, event_callback, malloc(1024), 1024);

  printf("mDNS Plugin Init\n");
  return 0;
}
