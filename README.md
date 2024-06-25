# Dynamic Lighting System with Temperature-Driven Color Variation

## Project Description
This project utilizes the MSP430G2553 LaunchPad board to create a dynamic lighting system that changes color based on internal temperature readings. The RGB LED on the LaunchPad serves as the light source, emitting hues that reflect temperature variations.

## Key Features
- **Temperature-Driven Color Variation**: The RGB LED illuminates with full intensity, exhibiting colors ranging from warm to cool based on internal temperature sensor readings.
- **Reference Temperature Capture**: Upon boot-up, the system captures the initial reference temperature. Subsequent temperature readings adjust the LED's color scheme accordingly. Users can update the reference temperature by pressing a button, facilitating adaptability to changing environmental conditions.
- **Real-time Temperature Monitoring**: The system measures the internal temperature every 100 milliseconds, enabling rapid response to temperature fluctuations.
- **Color Spectrum**: A comprehensive color chart consisting of 20 hues provides a wide range of temperature differentiation. Warmer temperatures manifest as colors toward the left of the chart, while colder temperatures correspond to hues on the right. The nominal color remains green.
- **Optimized ADC Configuration**: Utilizing the internal 1.5V reference voltage for the Analog-to-Digital Converter (ADC) enhances temperature measurement accuracy, enabling precise color rendition across the spectrum.
- **Dimming Control**: To achieve desired hues, the system employs dimming techniques to modulate the intensity of individual color components. Different dimming duty cycles for each color component facilitate accurate color reproduction and flexibility in color variation.

## Design & Implementation
The project's design and implementation are outlined in the following sections:

### Code Structure
- **Global Variables**: Store temperature and LED control parameters.
- **Interrupt Service Routines (ISRs)**: Timer and ADC ISRs handle timing and temperature readings, respectively.
- **Main Function**: Initializes system configurations and variables, triggers initial ADC capture for reference temperature, and utilizes a cyclic scheduler to manage system operations.
- **Cyclic Scheduler**: Enters low-power mode between scheduling ticks, handles button inputs for setting temperature reference, manages ADC triggers and temperature-based LED color changes, and controls LED dimming and intensity.

### Implementation Details
- **Timer Configuration**: Timer_A is configured to have precise timing, and the timer interrupt is utilized to wake up the cyclic scheduling and ADC triggering (inside the cyclic scheduler).
- **ADC Configuration**: The Analog-to-Digital Converter (ADC) captures temperature readings from the internal sensor, and ADC interrupts handle new temperature values.
- **Temperature and Color Control**: The system compares the current temperature with a reference value to determine temperature changes, and temperature differences trigger changes in LED color. LED color transitions from warm to cool or vice versa based on the temperature trend. The reference temperature is updated with the button.
- **LED Dimming and Control**: Dimming counters manage LED on/off cycles and intensity, and the variable `led_off_ticks` can take on one of four values (0, 1, 3, or 9) to correspond to different levels of intensity for the LED. These dimming levels are used in generating the 20 distinct hues that correspond to different temperature levels.

## Tests and Results
The project includes the following test scenarios:
- **Test 1**: Heat the system using a hair dryer. The results can be viewed in the `test1.mp4` video file.
- **Test 2**: Cool down the system using an ice cube. The results can be viewed in the `test2.mp4` video file.
