#include <stdint.h>
#include <cstddef>

#include "NTPClock.h"
#include "platform-clock.h"
#include "timer.h"

NTPClock localClock;

void NTPClock::setTime(uint32_t counter, uint64_t ntpTimestamp) {
  timeset_ = 1;
  lastCounter_ = counter;
  ntpTimestamp_ = ntpTimestamp;
}

uint8_t NTPClock::getTime(uint64_t *ntpTimestamp) {
  uint32_t now = COUNTERFUNC();

  return getTime(now, ntpTimestamp);
}

uint8_t NTPClock::getTime(uint32_t now, uint64_t *ntpTimestamp) {
  uint64_t tempTS;

  if (!timeset_)
    return 0;

  int64_t ntpFracPassed = (now - lastCounter_) * 4294967296LL / (int64_t)COUNTSPERSECOND;
  int32_t ntpFracPassedDrift = ntpFracPassed * ppb_ / 1000000000LL;
  ntpFracPassed += ntpFracPassedDrift;
  tempTS = ntpTimestamp_ + ntpFracPassed;
  if(ntpFracPassed >= 4294967296LL) { // every second
    lastCounter_ = now;
    ntpTimestamp_ = tempTS;
  }
  if(ntpTimestamp != NULL)
    *ntpTimestamp = tempTS;

  return 1;
}

// returns + for local slower, - for local faster
int64_t NTPClock::getOffset(uint32_t now, uint64_t ntpTimestamp) {
  uint64_t localS;

  if (getTime(now, &localS) != 1) {
    return 0;
  }

  int64_t diff = ntpTimestamp - localS;

  return diff;
}

// use + for local slower, - for local faster
// can be set once per second
void NTPClock::setPpb(int32_t ppb) {
  if(ppb >= -500000 && ppb <= 500000) { // limited to +/-500ppm
    ppb_ = ppb;
  }
}
