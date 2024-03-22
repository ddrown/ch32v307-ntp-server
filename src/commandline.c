#include "debug.h"
#include "commandline.h"
#include "ping.h"
#include "timer.h"
#include "lwip_task.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static void print_help() {
  printf(
    "commands:\n"
    "ping [ip] - ping ip\n"
    "phy - show phy status\n"
    "tim - show 32 bit timers\n"
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

static void run_command(char *cmdline) {
  if(strncmp("ping ", cmdline, 5) == 0) {
    ping_send(cmdline+5);
  } else if(strcmp("tim", cmdline) == 0) {
    printf("%lu %lu\n", now(), sys_now());
  } else if(strcmp("phy", cmdline) == 0) {
    phy_status();
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
