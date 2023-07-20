#include "nrf.h"
#include "spi.h"
#include "main.h"
#include "stm32h7xx_hal_spi.h"

#include <stdint.h>
#include <string.h>


#define TX_ADR_WIDTH    5   	//
#define RX_ADR_WIDTH    5   	//
#define TX_PLOAD_WIDTH  32  	//
#define RX_PLOAD_WIDTH  32  	//

// static uint8_t s_chanal = 40;	// 频率

// SPI(NRF14L01) commands ,	NRF
#define NRF_READ_REG    0x00  // Define read command to register
#define NRF_WRITE_REG   0x20  // Define write command to register
#define RD_RX_PLOAD 0x61  // Define RX payload register address
#define WR_TX_PLOAD 0xA0  // Define TX payload register address
#define FLUSH_TX    0xE1  // Define flush TX register command
#define FLUSH_RX    0xE2  // Define flush RX register command
#define REUSE_TX_PL 0xE3  // Define reuse TX payload register command
#define NOP         0xFF  // Define No Operation, might be used to read status register

// SPI(NRF14L01) registers(addresses)
#define CONFIG      0x00  // 'Config' register address
#define EN_AA       0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02  // 'Enabled RX addresses' register address
#define SETUP_AW    0x03  // 'Setup address width' register address
#define SETUP_RETR  0x04  // 'Setup Auto. Retrans' register address
#define RF_CH       0x05  // 'RF channel' register address
#define RF_SETUP    0x06  // 'RF setup' register address
#define STATUS      0x07  // 'Status' register address
#define OBSERVE_TX  0x08  // 'Observe TX' register address
#define CD          0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0  0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1  0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2  0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3  0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4  0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5  0x0F  // 'RX address pipe5' register address
#define TX_ADDR     0x10  // 'TX address' register address
#define RX_PW_P0    0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1    0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2    0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3    0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4    0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5    0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS 0x17  // 'FIFO Status Register' register address

															
#define MAX_TX  		0x10
#define TX_DS   		0x20
#define RX_DR   		0x40


#define NRF2_CE_0()         HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET)
#define NRF2_CE_1()         HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET)    

#define NRF2_CSN_0()        HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET)
#define NRF2_CSN_1()        HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET)     

#define NRF2_Read_IRQ()	    HAL_GPIO_ReadPin(NRF_IRQ_GPIO_Port, NRF_IRQ_Pin)

static const uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01};
static const uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01};

static uint8_t SPI_NRF2_RW(uint8_t TxData) {
    uint8_t Rxdata;
    HAL_SPI_TransmitReceive(&hspi1, &TxData, &Rxdata, 1, 1000);
 	return Rxdata;
}


static uint8_t SPI_NRF2_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t len) { 
    NRF2_CSN_0();
	
  	uint8_t status = SPI_NRF2_RW(reg);
	
  	for(uint8_t i = 0; i < len; i++) {
        SPI_NRF2_RW(*pBuf++);
    }

    NRF2_CSN_1();
    return status;             
}


static uint8_t SPI_NRF2_ReadBuf(uint8_t reg,uint8_t *pBuf,uint8_t len) {
    NRF2_CSN_0();

    uint8_t status = SPI_NRF2_RW(reg);
    
    for(size_t i = 0; i < len; i++) {
        pBuf[i] = SPI_NRF2_RW(NOP);
    }
	
    NRF2_CSN_1();

    return status;
}

static uint8_t SPI_NRF2_WriteReg(uint8_t reg, uint8_t value) {
    NRF2_CSN_0();

  	uint8_t status = SPI_NRF2_RW(reg);
	
  	SPI_NRF2_RW(value);
 	
  	NRF2_CSN_1();
	
  	return(status);
}

uint8_t SPI_NRF2_ReadReg(uint8_t reg) {
    NRF2_CSN_0();

    SPI_NRF2_RW(reg);

    uint8_t reg_val = SPI_NRF2_RW(NOP);
	
    NRF2_CSN_1();

    return(reg_val);
}

bool nrf_check(void) {
    uint8_t buf[5]={0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    SPI_NRF2_WriteBuf(NRF_WRITE_REG + TX_ADDR, buf, 5);
    memset(buf, 0, sizeof(buf));
    SPI_NRF2_ReadBuf(TX_ADDR, buf, 5);
    for(uint8_t i = 0; i < 5; i++) {
		    if(buf[i] != 0XA5) {
            return -1;
        }
	}
	return 0;
}

void NRF2_RX_Mode(uint8_t channel) {
	  NRF2_CE_0();
    // RX 节点地址
    SPI_NRF2_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);
    // 使能通道0自动应答
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);
    // 使能通道0的接收地址
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);
    // 设置通信频率
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_CH, channel);
    // 设置通道0的有效数据宽度
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);
    // 设置发射参数，0db增益，2Mbps，低噪声增益开启
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);
    // 设置基本工作模式的参数PWR_UP, EN_CRC, 16BIT_CRC, 接受模式，开启中断
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);
  	NRF2_CE_1();
}

void NRF2_TX_Mode(uint8_t channel) {
    NRF2_CE_0();
    // 写TX节点地址
    SPI_NRF2_WriteBuf(NRF_WRITE_REG + TX_ADDR, (uint8_t*)TX_ADDRESS, TX_ADR_WIDTH);
    // 设置RX和TX地址一样，为了ACK RX_Addr0 same as TX_Adr for Auto Ack
  	SPI_NRF2_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);
    // 使能通道0自动应答
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);
    // 使能通道0的接收地址
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);
    // 设置自动重发的间隔： 500us + 86us ， 最大重发次数：10
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);
    // 设置通信频率
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_CH, channel);
    // 设置发射参数，0db增益，2Mbps，低噪声增益开启
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);
    // 设置基本工作模式的参数PWR_UP, EN_CRC, 16BIT_CRC, 开启中断，发送模式
  	SPI_NRF2_WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);
    NRF2_CE_1();
	  HAL_Delay(100);
}

uint8_t NRF2_Tx_Dat(uint8_t *txbuf) {
    NRF2_CE_0();			
    SPI_NRF2_WriteBuf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
    NRF2_CE_1();
                        
	while(NRF2_Read_IRQ() != 0); 	
	
	uint8_t state = SPI_NRF2_ReadReg(STATUS);
              
	SPI_NRF2_WriteReg(NRF_WRITE_REG + STATUS, state); 	

	SPI_NRF2_WriteReg(FLUSH_TX, NOP);
 
	if(state & MAX_TX){
        return MAX_TX; 
    } else if(state & TX_DS){
        return TX_DS;
    } else {
        return ERROR;
    }
}

uint8_t NRF2_Rx_Dat(uint8_t *rxbuf) {
    NRF2_CE_1();
    while(NRF2_Read_IRQ()==0) {
        NRF2_CE_0();
        uint8_t state = SPI_NRF2_ReadReg(STATUS);    
        SPI_NRF2_WriteReg(NRF_WRITE_REG + STATUS, state);
        if(state  & RX_DR) {
            SPI_NRF2_ReadBuf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH);
            SPI_NRF2_WriteReg(FLUSH_RX, NOP);
            return RX_DR; 
        } else {
            return ERROR;
        }
    }
    return ERROR;
}

