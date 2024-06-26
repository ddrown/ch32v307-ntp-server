#include "lwip_task.h"
#include "tiny-macro-os.h"
#include "debug.h"
#include "lwipsetup.h"
#include "uart.h"
#include "ntp.h"

// run the periodic tasks here
void lwip_loop() {
    OS_RUN_TASK(os_lwip);
    OS_RUN_TASK(os_lwip_timeouts);
    OS_RUN_TASK(uart_poll);
    OS_RUN_TASK(ntp_poll);
}
