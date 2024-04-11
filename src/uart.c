#include "debug.h"
#include <ch32v30x.h>
#include "commandline.h"
#include "uart.h"

static void uart_init_clocks() {
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
}

static void uart_init_gpio() {
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void uart_init_base(uint32_t baudrate) {
    USART_InitTypeDef USART_InitStructure = {0};

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

}

void uart_init(uint32_t baudrate) {
    uart_init_clocks();
    uart_init_gpio();
    uart_init_base(baudrate);
}

OS_TASK(uart_poll, void) {
    OS_TASK_START(uart_poll);

    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
        cmdline_addchr(USART_ReceiveData(USART1));
    }

    // yield and return to the start next call
    OS_TASK_CWAITX(0);

    OS_TASK_END(uart_poll);
}