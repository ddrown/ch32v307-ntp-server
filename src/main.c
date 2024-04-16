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
    }
}

/* IP allocation success callback, users add initialization function about network process here */
void lwip_init_success_callback(ip_addr_t *ip)
{
    char ip_str[IP4ADDR_STRLEN_MAX];
    ip4addr_ntoa_r(ip_2_ip4(ip), ip_str, sizeof(ip_str));
    printf("IP %s\n\n", ip_str);
    ping_init();
}
