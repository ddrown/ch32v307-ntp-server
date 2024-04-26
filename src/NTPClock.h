#pragma once

class NTPClock {
  public:
    NTPClock() : timeset_(0), ppb_(0), refTime_(0) {};
    void setTime(uint32_t counter, uint64_t ntpTimestamp);
    uint8_t getTime(uint64_t *ntpTimestamp);
    uint8_t getTime(uint32_t now, uint64_t *ntpTimestamp);
    int64_t getOffset(uint32_t now, uint64_t ntpTimestamp);
    void setPpb(int32_t ppb);
    int32_t getPpb() { return ppb_; };
    uint32_t getReftime() { return refTime_; };
    void setRefTime(uint32_t refTime) { refTime_ = refTime; };
    uint8_t isTimeSet() { return timeset_; };

  private:
    uint8_t timeset_;
    // lastCounter_ local time, ntpTimestamp_ real time
    uint32_t lastCounter_;
    uint64_t ntpTimestamp_;
    int32_t ppb_;
    uint32_t refTime_;
};

extern NTPClock localClock;
