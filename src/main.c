#include "Debug/debug.h"
#include "lwipsetup.h"
#include "lwip/ip_addr.h"
#include "timer.h"
#include "ping.h"
#include "uart.h"
#include "tiny-os-task.h"

int main(void)
{
    Delay_Init();
    uart_init(115200);

    lwip_setup();
    timer_init();

    printf("Enter main loop.\n");
    while(1) {
    	lwip_loop();
        uart_poll();
    }
}

/* IP allocation success callback, users add initialization function about network process here */
void lwip_init_success_callback(ip_addr_t *ip)
{
    printf("IP %ld.%ld.%ld.%ld\n\n",  \
        ((ip->addr)&0x000000ff),       \
        (((ip->addr)&0x0000ff00)>>8),  \
        (((ip->addr)&0x00ff0000)>>16), \
        ((ip->addr)&0xff000000)>>24);
    ping_init();
}
