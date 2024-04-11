#pragma once

#include "tiny-macro-os.h"

void uart_init(uint32_t baudrate);
extern OS_TASK(uart_poll, void);