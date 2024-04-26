#include "debug.h"
#include "timer.h"
#include <ch32v30x.h>
#include <ch32v30x_rcc.h>
#include <ch32v30x_tim.h>

uint32_t time_now() {
  uint16_t t1b, t2, t1a;
  
  t1b = TIM2->CNT;
  t2 = TIM1->CNT;
  t1a = TIM2->CNT;

  if(t1b == t1a)
    return t1b << 16 | t2;

  if(t2 < 0x7FFF) {
    return t1a << 16 | t2;
  }
  return t1b << 16 | t2;
}

void timer_init() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_CounterModeConfig(TIM1, TIM_CounterMode_Up);
  TIM_CounterModeConfig(TIM2, TIM_CounterMode_Up);
  TIM_PrescalerConfig(TIM1, 0, TIM_PSCReloadMode_Immediate);
  TIM_PrescalerConfig(TIM2, 0, TIM_PSCReloadMode_Immediate);

  TIM_SetAutoreload(TIM1, 0xFFFF);
  TIM_SetAutoreload(TIM2, 0xFFFF);
  
  TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
  TIM_ITRxExternalClockConfig(TIM2, TIM_TS_ITR0);
  TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_External1);

  TIM_Cmd(TIM1, ENABLE);
  TIM_Cmd(TIM2, ENABLE);
}