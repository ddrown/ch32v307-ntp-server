#include "debug.h"
#include "commandline.h"
#include "ping.h"
#include "timer.h"
#include "lwip_task.h"
#include "lwip/ip_addr.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static void print_help() {
  printf(
    "commands:\n"
    "ping [ip] - ping ip\n"
    "phy - show phy status\n"
    "tim - show 32 bit timers\n"
    "mac - show configured mac filters\n"
    "ip  - show ip addresses\n"
  );
}

// TODO: these bit values are wrong
static void phy_status() {
  uint16_t phyreg = ETH_ReadPHYRegister( PHY_ADDRESS, 0x10 );

  if((phyreg & 0b1) == 0) {
    printf("link: no\n");
  } else {
    printf("link: yes\n");
  }

  if((phyreg & 0b10) == 0) {
    printf("speed: 100M\n");
  } else {
    printf("speed: 10M\n");
  }

  if((phyreg & 0b100) == 0) {
    printf("duplex: half\n");
  } else {
    printf("duplex: full\n");
  }

  if((phyreg & 0b1000) != 0) {
    printf("loopback: on\n");
  }

  if((phyreg & 0b10000) == 0) {
    printf("auto-neg: incomplete\n");
  }

  if((phyreg & 0b100000) != 0) {
    printf("jabber detected\n");
  }

  if((phyreg & 0b1000000) != 0) {
    printf("remote fault detected\n");
  }

  if((phyreg & 0b100000000) != 0) {
    printf("received code word page\n");
  }
}

static void mac_status_addr(uint8_t mac_number, uint32_t MacAddr) {
  uint8_t macaddr[6];
  uint32_t flags;

  ETH_GetMACAddress(MacAddr, macaddr);
  flags = (*(__IO uint32_t *)(ETH_MAC_ADDR_HBASE + MacAddr)) & 0xFFFF0000;
  printf("%d: %x:%x:%x:%x:%x:%x F=%x\n", 
    mac_number, 
    macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5],
    flags
    );
}

static void mac_status() {
  mac_status_addr(0, ETH_MAC_Address0);
  mac_status_addr(1, ETH_MAC_Address1);
  mac_status_addr(2, ETH_MAC_Address2);
  mac_status_addr(3, ETH_MAC_Address3);
}

static void ip_status() {
  char ip_str[IP4ADDR_STRLEN_MAX];
  ip4addr_ntoa_r(ip_2_ip4(&netif_default->ip_addr), ip_str, sizeof(ip_str));
  printf("IP %s\n", ip_str);
#if LWIP_IPV6
  for(uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
    char ip6_str[IP6ADDR_STRLEN_MAX];
    ip6addr_ntoa_r(ip_2_ip6(&netif_default->ip6_addr[i]), ip6_str, sizeof(ip6_str));
    printf("IP6 %s s=%x\n", ip6_str, netif_default->ip6_addr_state[i]);
  }
#endif /* LWIP_IPV6 */
}

static void run_command(char *cmdline) {
  if(strncmp("ping ", cmdline, 5) == 0) {
    ping_send(cmdline+5);
  } else if(strcmp("tim", cmdline) == 0) {
    printf("%lu %lu\n", now(), sys_now());
  } else if(strcmp("phy", cmdline) == 0) {
    phy_status();
  } else if(strcmp("mac", cmdline) == 0) {
    mac_status();
  } else if(strcmp("ip", cmdline) == 0) {
    ip_status();
  } else {
    print_help();
  }
}

void cmdline_prompt() {
  printf("> ");
}

static char cmdline[40];
static uint32_t cmd_len = 0;

void reprint_prompt() {
  cmdline_prompt();
  if(cmd_len > 0) {
    printf("%s", cmdline);
  }
}

void cmdline_addchr(char c) {
  switch(c) {
    case '\r':
    case '\n':
      printf("\n");
      run_command(cmdline);
      cmdline_prompt();
      cmdline[0] = '\0';
      cmd_len = 0;
      break;
    case '\b':
    case '\x7F':
      if(cmd_len > 0) {
        printf("\x7F");
        cmd_len--;
        cmdline[cmd_len] = '\0';
      }
      break;
    default:
      if(cmd_len < sizeof(cmdline)-1) {
        printf("%c", c);
        cmdline[cmd_len] = c;
        cmdline[cmd_len+1] = '\0';
        cmd_len = cmd_len + 1;
      }
  }
  fflush(stdout);
}
