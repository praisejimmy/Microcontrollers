/*
 * spi_driver.c
 *
 *  Created on: Oct 16, 2019
 *      Author: ryanmyers
 */

#include "msp.h"
#include "spi_driver.h"

void Send_DAC_Voltage(float v) {

    uint16_t out_volt = (uint16_t)(v / DAC_RESOLUTION);

    uint8_t lobyte = out_volt & 0xFF;
    uint8_t hibyte = (out_volt >> 8) & 0x0F;
    hibyte |= 0x30;

    CHIP_SEL_PORT->OUT &= ~CHIP_SEL;
    EUSCI_B0->TXBUF = hibyte;
    while(!(EUSCI_B0->IFG & EUSCI_A_IFG_TXIFG));
    EUSCI_B0->TXBUF = lobyte;
    while(!(EUSCI_B0->IFG & EUSCI_A_IFG_RXIFG));
    CHIP_SEL_PORT->OUT |= CHIP_SEL;

}

void Initialize_SPI(void) {

    CS->KEY = CS_KEY_VAL;   // Unlock clock registers

    CS->CTL1 &= ~(CS_CTL1_DIVHS_MASK | CS_CTL1_DIVS_MASK | CS_CTL1_SELS_MASK); // Clear CS registers
    CS->CTL1 |= CS_CTL1_DIVHS_0 | CS_CTL1_DIVS_0 | CS_CTL1_SELS_3; // Set DCO to drive HSCLK and SCLK

    CS->KEY = 0; // Lock clock registers

    CHIP_SEL_PORT->DIR |= CHIP_SEL;
    CHIP_SEL_PORT->SEL0 &= ~CHIP_SEL;
    CHIP_SEL_PORT->SEL1 &= ~CHIP_SEL;

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;

    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST
                        | EUSCI_B_CTLW0_MST
                        | EUSCI_B_CTLW0_MSB
                        | EUSCI_B_CTLW0_CKPH
                        | EUSCI_B_CTLW0_SYNC
                        | EUSCI_B_CTLW0_UCSSEL_2;

    EUSCI_B0->BRW = 0x01;

    SPI_PORT->DIR |= SIMO;
    SPI_PORT->REN &= ~SIMO;
    SPI_PORT->SEL0 |= SIMO;
    SPI_PORT->SEL1 &= ~SIMO;

    SPI_PORT->DIR &= ~SOMI;
    SPI_PORT->REN &= ~SOMI;
    SPI_PORT->SEL0 |= SOMI;
    SPI_PORT->SEL1 &= ~SOMI;

    SPI_PORT->DIR |= SPI_CLK;
    SPI_PORT->REN &= ~SPI_CLK;
    SPI_PORT->SEL0 |= SPI_CLK;
    SPI_PORT->SEL1 &= ~SPI_CLK;

    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;

}
