#include <Arduino.h>
#include "main.hpp"

uint32_t                sampleRate = 4096;              // Mhz = this % 2

volatile uint32_t       scanClockPulseCounter;
volatile bool           enableScanClk = false;
volatile uint32_t       scanShiftRegister_0;
uint8_t                 shift;

// this must align with staticPins active state inactive value
static uint8_t          scanClockState = 1;

/**
  * @name   timers_scanChainCapture
  * @brief  capture scan chain data & control CLK
  * @param  None
  * @retval None
  */
void timers_scanChainCapture(void)
{
    // initialize vars used by timer handler
    scanClockPulseCounter = 0;
    scanShiftRegister_0 = 0;
    shift = 31;

    // SCAN_CLK high
    digitalWrite(OCP_SCAN_CLK, scanClockState);

    // SCAN_LD_N low briefly
    digitalWrite(OCP_SCAN_LD_N, 0);
    delayMicroseconds(200);
    digitalWrite(OCP_SCAN_LD_N, 1);

    // start capture (when CLK falls)
    enableScanClk = true;

    while ( scanClockPulseCounter < 32 )
    {
        // wait for shifted data in
        ;
    }

    enableScanClk = false;
}

/**
  * @name   TC5_Handler
  * @brief  TC ISR
  * @param  None
  * @retval None
  */
void TC5_Handler(void) 
{
    if ( enableScanClk )
    {      
        if ( scanClockState == 1 )
        {
            // latch bit on falling edge of clock + 10 usec Figure 97
            scanClockState = 0;
            digitalWrite(OCP_SCAN_CLK, scanClockState);
            delayMicroseconds(10);
            scanShiftRegister_0 |= (digitalRead(OCP_SCAN_DATA_IN) << shift--);
        }
        else
        {
            scanClockState = 1;
            scanClockPulseCounter++;
            digitalWrite(OCP_SCAN_CLK, scanClockState);
        }
    }

    TC5->COUNT16.INTFLAG.bit.MC0 = 1; 
}

/**
  * @name   tcIsSyncing
  * @brief  returning SYNCBUSY flag
  * @param  None
  * @retval None
  */
bool tcIsSyncing(void)
{
    return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

/**
  * @name   tcStartCounter
  * @brief  start timer counter
  * @param  None
  * @retval None
  */
void tcStartCounter(void)
{
    TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (tcIsSyncing());
}

/**
  * @name   tcReset
  * @brief  reset timer
  * @param  None
  * @retval None
  */
void tcReset(void)
{
    TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
    while (tcIsSyncing());
    while (TC5->COUNT16.CTRLA.bit.SWRST);
}

/**
  * @name   tcDisable
  * @brief  disable timer
  * @param  None
  * @retval None
  */
void tcDisable(void)
{
    TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (tcIsSyncing());
}

/**
  * @name   tcConfigure
  * @brief  configure timer TC5
  * @param  None
  * @retval None
  */
void tcConfigure(int sampleRate)
{
    // select the generic clock generator used as source to the generic clock multiplexer
    GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
    while (GCLK->STATUS.bit.SYNCBUSY);

    tcReset();

    // Set Timer counter 5 Mode to 16 bits, it will become a 16bit counter ('mode1' in the datasheet)
    TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;

    // Set TC5 waveform generation mode to 'match frequency'
    TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;

    //set prescaler
    //the clock normally counts at the GCLK_TC frequency, but we can set it to divide that frequency to slow it down
    //you can use different prescaler divisons here like TC_CTRLA_PRESCALER_DIV1 to get a different range
    TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_ENABLE; //it will divide GCLK_TC frequency by 1

    //set the compare-capture register. 
    //The counter will count up to this value (it's a 16bit counter so we use uint16_t)
    //this is how we fine-tune the frequency, make it count to a lower or higher value
    //system clock should be 1MHz (8MHz/8) at Reset by default
    TC5->COUNT16.CC[0].reg = (uint16_t) (SystemCoreClock / sampleRate);
    while (tcIsSyncing());

    // Configure interrupt request
    NVIC_DisableIRQ(TC5_IRQn);
    NVIC_ClearPendingIRQ(TC5_IRQn);
    NVIC_SetPriority(TC5_IRQn, 0);
    NVIC_EnableIRQ(TC5_IRQn);

    // Enable the TC5 interrupt request
    TC5->COUNT16.INTENSET.bit.MC0 = 1;
    while (tcIsSyncing()); //wait until TC5 is done syncing 
} 

/**
  * @name   timers_Init
  * @brief  initialize timers used by firmware
  * @param  None
  * @retval None
  */
void timers_Init(void) 
{
    tcConfigure(sampleRate);
    tcStartCounter();
}
