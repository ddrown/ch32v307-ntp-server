#include "debug.h"
#include "main.h"
#include "ping.h"
#include "timer.h"

#include "lwip/raw.h"
#include "lwip/icmp6.h"
#include "lwip/ip_addr.h"

static struct raw_pcb *ping_pcb = NULL;
static uint32_t start_ping;
static uint16_t ping_seq = 0;

static unsigned char ping6_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr) {
  uint32_t end_ping;

  if(p->tot_len < IP6_HLEN+sizeof(struct icmp6_echo_hdr)) {
    printf("short icmp6 packet\n");
    return 0;
  }

  uint8_t *packet = (uint8_t *)p->payload;
  struct ip6_hdr *iph = (struct ip6_hdr *)packet;

  if(IP6H_V(iph) != 6) {
    return 0;
  }
  if(IP6H_NEXTH(iph) != IP6_NEXTH_ICMP6) {
    return 0;
  }

  struct icmp6_echo_hdr *icmph = (struct icmp6_echo_hdr *)(packet + IP6_HLEN);
  if(icmph->type != ICMP6_TYPE_EREP) {
    return 0;
  }

  end_ping = time_now();
  uint32_t clocks = end_ping - start_ping;
  uint32_t us = clocks / 144; // 144MHz, 144 clocks per us

  printf("rtt %lu us (%lu)\n", us, clocks);
  printf("id: %u seq: %u\n", ntohs(icmph->id), ntohs(icmph->seqno));

  pbuf_free(p);
  return 1;
}

void ping6_send(const char *dest) {
  ip_addr_t dest_addr;

  IP_SET_TYPE_VAL(dest_addr, IPADDR_TYPE_V6);
  if(!ip6addr_aton(dest, ip_2_ip6(&dest_addr))) {
    printf("ip6addr_aton failed\n");
    return;
  }

  struct pbuf *p = pbuf_alloc(PBUF_IP, sizeof(struct icmp6_echo_hdr), PBUF_RAM);
  if(!p) {
    printf("pbuf_alloc failed\n");
    return;
  }

  struct icmp6_echo_hdr *icmph = (struct icmp6_echo_hdr *)p->payload;

  icmph->type = ICMP6_TYPE_EREQ;
  icmph->chksum = 0;
  icmph->id     = htons(12345);
  icmph->seqno  = htons(ping_seq++);

  start_ping = time_now();
  raw_sendto(ping_pcb, p, &dest_addr);
  pbuf_free(p);
}

void ping6_init() {
  ping_pcb = raw_new(IP6_NEXTH_ICMP6);
  raw_recv(ping_pcb, ping6_recv, NULL);
  raw_bind(ping_pcb, IP6_ADDR_ANY);
}
