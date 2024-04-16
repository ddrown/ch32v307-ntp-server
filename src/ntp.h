#pragma once

#include "tiny-macro-os.h"

void ntp_init();
extern OS_TASK(ntp_poll, void);