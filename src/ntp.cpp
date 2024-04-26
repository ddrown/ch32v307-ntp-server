extern "C" {

#include <string.h>
#include <stdint.h>
#include "tiny-macro-os.h"

#include "main.h"
#include "ntp.h"
#include "timer.h"

#include "lwip/udp.h"

static struct udp_pcb *ntp_pcb = NULL;

struct __attribute__((packed)) ntp_packet {
  // 0
  uint8_t leap_version_mode;
  uint8_t stratum;
  int8_t poll;
  int8_t precision;
  // 4
  uint16_t root_delay_s;
  uint16_t root_delay_subs;
  // 8
  uint16_t root_dispersion_s;
  uint16_t root_dispersion_subs;
  // 12
  uint32_t refid;
  uint32_t reftime_s;
  // 20
  uint32_t reftime_subs;
  uint32_t origin_s;
  // 28
  uint32_t origin_subs;
  uint32_t rx_s;
  // 36
  uint32_t rx_subs;
  uint32_t tx_s;
  // 44
  uint32_t tx_subs;
  // 48
};

struct ntp_server_destinations {
  const char *ipstr;
  ip_addr_t dest_addr;
  uint32_t start_ntp;
  uint32_t end_ntp;
  uint32_t last_rx_s;
  uint32_t last_rx_subs;
};

static struct ntp_server_destinations dests[] = {
  {.ipstr = "10.42.0.1"},
  {.ipstr = "10.1.2.143"},
  {.ipstr = "10.1.2.139"},
  {.ipstr = NULL},
};

static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  struct ntp_server_destinations *src = NULL;
  uint32_t clocks;
  float rtt_us;
  uint32_t packet_ts = time_now();

  for(int i = 0; dests[i].ipstr != NULL; i++) {
    if(ip_2_ip4(addr)->addr == ip_2_ip4(&dests[i].dest_addr)->addr) {
      src = &dests[i];
      break;
    }
  }

  if(src == NULL) {
    char srcaddr[IP4ADDR_STRLEN_MAX];
    ip4addr_ntoa_r(ip_2_ip4(addr), srcaddr, IP4ADDR_STRLEN_MAX);
    printf("unknown source address %s\n", srcaddr);

    goto free_return;
  }

  if(p->tot_len < sizeof(struct ntp_packet)) {
    printf("short packet: %u < %u\n", p->tot_len, sizeof(struct ntp_packet));

    goto free_return;
  }

  if(p->len < sizeof(struct ntp_packet)) { // shouldn't ever happen
    printf("short fragment: %u < %u\n", p->len, sizeof(struct ntp_packet));

    goto free_return;
  }

  struct ntp_packet response;
  memcpy(&response, p->payload, sizeof(struct ntp_packet));
  src->end_ntp = packet_ts;
  src->last_rx_s = response.rx_s;
  src->last_rx_subs = response.rx_subs;

  clocks = src->end_ntp - src->start_ntp;
  rtt_us = clocks / 144.0; // 144MHz
  //printf("%s %u %u %u %u %u %u %u %u\n", src->ipstr, sys_now(), (int)(rtt_us * 1000), clocks, src->end_ntp,
  //  ntohl(response.rx_s), ntohl(response.rx_subs), ntohl(response.tx_s), ntohl(response.tx_subs));

free_return:
  pbuf_free(p);
}

static void ntp_send(struct ntp_server_destinations *dest) {
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct ntp_packet), PBUF_RAM);
  err_t err;

  if(!p) {
    printf("pbuf_alloc failed\n");
    return;
  }

  struct ntp_packet *request = (struct ntp_packet *)p->payload;
  memset(request, 0, sizeof(struct ntp_packet));
  request->leap_version_mode = 0b11100011;   // LI=unsync, Version=4, Mode=3(client)
  request->poll = 6; // 2^6 = 64s
  request->precision = -27;  // Peer Clock Precision in 2^x seconds, -27=7ns
  request->origin_s = htonl(dest->end_ntp);
  request->origin_subs = htonl(0);
  request->rx_s = htonl(dest->end_ntp);
  request->rx_subs = htonl(0);

  dest->start_ntp = time_now();
  request->tx_s = htonl(dest->start_ntp);
  request->tx_subs = htonl(0);

  err = udp_sendto(ntp_pcb, p, &dest->dest_addr, 123);
  pbuf_free(p);

  if (err != ERR_OK) {
    printf("Error sending packet: %d\n", err);
  }
}

static void ntp_poll_dest(uint8_t dest) {
  if(dests[dest].ipstr != NULL) {
    ntp_send(&dests[dest]);
  }
}

void ntp_init() {
  ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  udp_recv(ntp_pcb, ntp_recv, NULL);
  udp_bind(ntp_pcb, IP_ANY_TYPE, 1123);

  for(int i = 0; dests[i].ipstr != NULL; i++) {
    dests[i].end_ntp = 0;
    dests[i].start_ntp = 0;
    dests[i].last_rx_s = 0;
    dests[i].last_rx_subs = 0;
    IP_SET_TYPE_VAL(dests[i].dest_addr, IPADDR_TYPE_V4);
    if(!ip4addr_aton(dests[i].ipstr, ip_2_ip4(&dests[i].dest_addr))) {
      printf("ip4addr_aton failed on %s\n", dests[i].ipstr);
    }
  }

  OS_TASK_RESTART_ANOTHER(ntp_poll, 5);
}

// poll the ntp sources every second
OS_TASK(ntp_poll, void) {
    static int sourceid = 0;
    OS_TASK_START(ntp_poll);

    ntp_poll_dest(sourceid);
    sourceid++;
    if (dests[sourceid].ipstr == NULL)
        sourceid = 0;

    // yield and return to the start next call
    OS_TASK_CWAITX(1000);

    OS_TASK_END(ntp_poll);
}

};
