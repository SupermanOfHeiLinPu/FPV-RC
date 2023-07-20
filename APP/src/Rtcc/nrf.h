#ifndef __NRF_H__
#define __NRF_H__

#include <stdint.h>
#include <stdbool.h>

bool nrf_check(void);

void NRF2_RX_Mode(uint8_t channel);

void NRF2_TX_Mode(uint8_t channel);

uint8_t NRF2_Tx_Dat(uint8_t *txbuf);

uint8_t NRF2_Rx_Dat(uint8_t *rxbuf);


#endif