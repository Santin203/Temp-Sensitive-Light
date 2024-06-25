/*  Author: Santiago Jiménez
 *  Project 1 CS-4380
 *  Spring 2024
 *  Description: This project is a temperature sensor that changes the color of the RGB LED based on the temperature. 
 *  The temperature is measured using the internal temperature sensor of the board. The temperature is measured every 
 *  100 ms and the color of the LED is changed based on the temperature. The LED will change to a warmer color if the 
 *  temperature is increasing and to a cooler color if the temperature is decreasing. The reference temperature is set
 *  by pressing the button. Color green is the nominal color.
 */

#include <msp430.h>

#define TIMER_PERIOD 1 // Fastest possible tick = 1/12000 = 83.33 us.
#define LED_ON_TICKS 1
#define ADC_TRIGGER_TICKS 1200      // Every 100 ms

#define B1 0x02 //red
#define B3 0x08 //green
#define B5 0x20 //blue
#define B135 (B1 | B3 | B5) //white
#define B13 (B1 | B3) //yellow
#define B15 (B1 | B5) //magenta
#define B35 (B3 | B5) //cyan

/* Global variables */
int g_new_temp_value = 0;
int g_new_temp_available = 0;

/* TimerA-0 Interrupt Service Routine */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
    /* Return to active mode (wake-up cyclic scheduler) */
    LPM0_EXIT;
}

/* ADC10 Interrupt Service Routine */
#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR(void)
{
    /* Record current temperature */
    g_new_temp_value = ADC10MEM;
    /* Set software flag */
    g_new_temp_available = 1;
}

void main(void)
{
    /* Dimming control */
    int dimming_state = 0;
    int dimming_tick_count = 0;

    /* ADC control */
    int adc_trigger_tick_count = 0;

    /* Temperature variables */
    volatile int current_temp;
    int temp_reference;
    int diff_temperature;

    /* RGB color control */
    int strong_led_bit;
    int weak_led_bit;
    int led_off_tick = 0;
    int i = 8;
    int strong_colors[20] = {B1, B1, B1, B1, B13, B3, B3, B3, B3, B3, B3, B3, B35, B5, B5, B5, B5, B5, B5, B5};
    int weak_colors[20] = {0, B3, B3, B3, 0, B1, B1, B1, 0, B5, B5, B5, 0, B3, B3, B3, 0, B1, B1, B1};
    int led_off_ticks[20] = {0, 9, 3, 1, 0, 1, 3, 9, 0, 9, 3, 1, 0, 1, 3, 9, 0, 9, 3, 1};

    /* Stop watchdog timer */
    WDTCTL = WDTPW | WDTHOLD;

    /* Select ACLK source from VLOCLK */
    BCSCTL3 |= LFXT1S_2;

    /* Configure Timer_A
     * TASSEL_1: Clock select ACLK
     * ID_0: Clock divider = 0 (no division)
     * MC_1: Continuous-up mode to TACCR0
     * TACLR: Clear the counter
     */
    TACCR0 = TIMER_PERIOD;
    TACCTL0 = CCIE; // Enable interrupts on Compare 0
    TACTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    /* Set pin direction to output */
    P2DIR |= B135;

    /* Start with the LED off */
    dimming_state = 0;
    dimming_tick_count = 0;
    P2OUT &= !B135;

    /* Set the initial LED selection */
    strong_led_bit = strong_colors[i];
    weak_led_bit = 0;

    /* Configure ADC
     * SREF_1: Use VREF as VR+, and Vss as VR-
     * ADC10SHT_3: Use 64x sample & hold time
     * ADC10ON: Enable the ADC module
     * REFON: Enable the internal reference
     * Use 1.5V reference
     * ADC10IE: Interrupts enabled
     * INCH_10: Source = internal temperature sensor
     * ADC10SSEL_0: Select ADC10OSC as ADC clock source
     * SHS_0: Use ADC10OSC as source for sample & hold
     * CONSEQ_0: Single conversion
     */
    ADC10CTL0 = SREF_1 | ADC10SHT_3 | ADC10ON | REFON | ADC10IE;
    ADC10CTL1 = INCH_10 | ADC10SSEL_0 | SHS_0 | CONSEQ_0;

    /* Wait 5ms for reference to settle */
    TACTL = TASSEL_1 | ID_0 | MC_2 | TACLR;
    while (TAR < 30);

    /* Enable ADC captures */
    ADC10CTL0 |= ENC;

    /* Enable interrupts globally */
    __enable_interrupt();

    /* Trigger one capture, to set the initial temperature reference */
    ADC10CTL0 |= ADC10SC;

    /* Spin until first ADC capture is done */
    while (g_new_temp_available == 0);
    temp_reference = g_new_temp_value;    // Get new temperature value
    g_new_temp_available = 0;           // Clear SW flag
    current_temp = temp_reference;

    /* Configure Timer_A
     * MC_1: Continuous-up mode to TACCR0
     */
    TACCR0 = TIMER_PERIOD;  // About 1 ms
    TACCTL0 = CCIE; // Enable interrupts on Compare 0
    TACTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    /* Cyclic scheduler */
    while(1)
    {
        /* Go to sleep (low-power mode) until the next cyclic scheduler tick */
        LPM0;

        /* Check for button pressed */
        if ((P1IN & B3) == 0)
        {
            /* Set the temperature reference */
            temp_reference = current_temp;

            /* Reset the LED color to green */
            i = 8;
        }

        /* Handle triggering of new ADC captures */
        adc_trigger_tick_count++;
        if (adc_trigger_tick_count == ADC_TRIGGER_TICKS)
        {
            adc_trigger_tick_count = 0;

            /* Trigger a new capture */
            ADC10CTL0 |= ADC10SC;
        }

        /* Handle new values from ADC */
        if (g_new_temp_available == 1)
        {
            /* Get values from ISR and clear SW flag */
            current_temp = g_new_temp_value;
            g_new_temp_available = 0;

            /* Determine the difference in temperature */
            if (current_temp > temp_reference){
                diff_temperature = current_temp - temp_reference;
            } else {
                diff_temperature = temp_reference - current_temp;
            }

            /* Change colors if the temperature has changed by more than 1 unit */
            if (diff_temperature >= 1){

                /* Determine the LED state and dimming duty cycle */
                if (current_temp > (temp_reference))
                {
                    /* Change to warmer colors */
                    weak_led_bit = weak_colors[i];
                    strong_led_bit = strong_colors[i];
                    led_off_tick = led_off_ticks[i];
                    i--;

                    if (i<0){
                        i = 0;
                    }


                }
                else if (current_temp < (temp_reference))
                {
                    /* Change to cooler colors */
                    weak_led_bit = weak_colors[i];
                    strong_led_bit = strong_colors[i];
                    led_off_tick = led_off_ticks[i];
                    i++;

                    if (i>19){
                        i = 19;
                    }
                }
                else
                {
                    /* Error checking */
                    P2OUT &= !B135;   // Turn off
                }

                /* Update the temperature reference */
                temp_reference = current_temp;
            }

        }

        /* Increment dimming counter */
        dimming_tick_count++;

        /* Process state */
        if (dimming_state == 1)
        {
            if (dimming_tick_count == LED_ON_TICKS)
            {
                /* Go to Off */
                dimming_state = 0;
                P2OUT &= !B135;   // Turn off
                dimming_tick_count = 0;      // Reset count
            }
        }
        else
        {
            if (dimming_tick_count == led_off_tick | led_off_tick == 0)
            {
                /* Go to On */
                dimming_state = 1;
                P2OUT |= weak_led_bit;
                dimming_tick_count = 0;      // Reset count
            }
        }

        /* Turn on the currently selected LED */
        P2OUT |= strong_led_bit;

    }
}


