/*
 * adc_driver.c
 *
 *  Created on: Oct 30, 2019
 *      Author: ryanmyers
 */

#include "msp.h"
#include "adc_driver.h"

static volatile uint8_t ADC_PP_Flag;
static volatile uint16_t peak = 0;
static volatile uint16_t trough = 16384;
static volatile uint16_t max_per_cycle;
static volatile uint16_t min_per_cycle;

void Initialize_ADC(void) {
    CS->KEY = CS_KEY_VAL;   // Unlock clock registers

    CS->CTL1 &= ~(CS_CTL1_DIVHS_MASK | CS_CTL1_DIVS_MASK | CS_CTL1_SELS_MASK); // Clear CS registers
    CS->CTL1 |= CS_CTL1_DIVHS_0 | CS_CTL1_DIVS_0 | CS_CTL1_SELS_3; // Set DCO to drive HSCLK and SMCLK

    CS->KEY = 0; // Lock clock registers

    ADC14->CTL0 &= ~ADC14_CTL0_ENC;      //Disable conversion
    ADC14->CTL0 = ADC14_CTL0_SHP |       //Enable internal sample timer
                  ADC14_CTL0_SSEL_4 |    //Select SMCLK
                  ADC14_CTL0_CONSEQ_2 |  //Single repeat channel
                  ADC14_CTL0_SHT0_6 |    //Sample 96 clocks
                  ADC14_CTL0_MSC |       //Auto repeat
                  ADC14_CTL0_ON;         //Turn on ADC
    ADC14->CTL1 = ADC14_CTL1_RES_3;      //14 Bit resolution and mem[0]
    ADC14->MCTL[0] = ADC14_MCTLN_INCH_0; //Set all to 0 as not needed
    ADC14->IER0 = ADC14_IER0_IE0;        //Enable interrupts on mem[0]
    ADCPORT->SEL0 |= ADCPIN;             //Initialize port to accept input
    ADCPORT->SEL1 |= ADCPIN;
    NVIC->ISER[0] = 1 << (ADC14_IRQn & 0x1F);
    __enable_irq();
    ADC14->CTL0 |= ADC14_CTL0_ENC |       //Enable conversion again
                   ADC14_CTL0_SC;         //Enable sampling

    ADC_PP_Flag = ADC_UNAVAILABLE;

}

void ADC14_IRQHandler(void) {
    static volatile irq_cnt = 0;
    ADC14->CLRIFGR0 |= ADC14_CLRIFGR0_CLRIFG0;
    ADC_Value = ADC14->MEM[0];
    if (irq_cnt == 2000) {
        max_per_cycle = peak;
        min_per_cycle = trough;
        peak = 0;
        trough = 16384;
        irq_cnt = 0;
    }
    if (ADC_Value > peak) {
        peak = ADC_Value;
    }
    else if (ADC_Value < trough) {
        trough = ADC_Value;
    }
    irq_cnt++;
}

int Read_Peak(void) {
    if (ADC_PP_Flag) {
        ADC_PP_Flag = ADC_UNAVAILABLE;
        return
    }
    return ADC_PP_Flag;
}

