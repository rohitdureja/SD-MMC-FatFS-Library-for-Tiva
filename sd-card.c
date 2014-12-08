#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "ff.h"
#include "diskio.h"

// defines for onboard LEDs. used for debugging
#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

// the error routine that is called if the driver library encounters an error
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

/*Variable required for SD Card R/W*/
FATFS fatfs;
FIL fil;
WORD buffer[4096];
FRESULT rc;
UINT br, bw;

// test function to blink led
void test_function(void)
{
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, RED_LED);
	ROM_SysCtlDelay(ROM_SysCtlClockGet()/6);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, 0);
	ROM_SysCtlDelay(ROM_SysCtlClockGet()/6);
}


// systick handler for FatFs library
void
SysTickHandler(void)
{
	ROM_SysTickIntDisable();
    disk_timerproc();
    ROM_SysTickIntEnable();
}


// function to initialize onboard UART.
void uart_init(void)
{
	// enable uart gpio.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	// configure gpio pins as uart.
	ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
	ROM_GPIOPinConfigure(GPIO_PA1_U0TX);

	// enable the uart module.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	// set clock for baud rate generation.
	ROM_UARTClockSourceSet(UART0_BASE, UART_CLOCK_SYSTEM);

	// set pin type to uart.
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// configure uart 0 at 115200 bps.
	UARTStdioConfig(0, 115200, SysCtlClockGet());
}

// main function
int main(void)
{
    unsigned long i;
	//
    // Set the system clock to run at 80MHz from the PLL.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);



	// enable the peripherals used
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);

	// make led gpio pins as outputs
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);

	test_function();

	// configure the systick timer for a 100Hz interrupt. required by the FatFs driver.
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
    SysTickIntRegister(SysTickHandler);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();

    // enable interrupts
    ROM_IntMasterEnable();

    // initialize UART for console I/O
    uart_init();



    // mount the file system, using logical disk 0.
    rc = f_mount(0, &fatfs);
    if(rc != FR_OK)
    {
        UARTprintf("f_mount error\n");
        return 0;
    }
    else
    {
    	UARTprintf("f_mount success!\n");
    }
    //rc = f_mkfs(0, 0, 64);
    /*char file[]="lol";
    char filename[10];
    char ctr = 0;
    unsigned long itr;
    sprintf(filename, "%s.%d", file, ctr);*/
    //strcpy((char *)buffer, "rohit is the best\n");
    //UARTprintf("%s\n", buffer);
    for(i = 0 ; i<4096 ; i++)
    	buffer[i]='a';
    rc = f_open(&fil, "lol.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if(rc != FR_OK)
    {
    	UARTprintf("f_open error\n");
        return 0;
    }
    else
    {
    	UARTprintf("new file created\n");
    }
    UARTprintf("writing to file\n");
    for(i = 0 ; i < 10000 ; i++)
    {
    	rc = f_write(&fil, buffer, strlen((const char *)buffer), &bw);
    	if(rc != FR_OK)
    	{
    		UARTprintf("f_write error\n");
    		return 0;
    	}
    }
    UARTprintf("done writing\n");
    test_function();
    f_close(&fil);
    f_mount(0, NULL);

    // loop forever
    while(1)
    {
    	/*UARTprintf("inside loop\n");
    	if(ctr < 10)
    	{
			if(itr==100)
			{
				f_close(&fil);
				ctr = ctr + 1;
				itr = 0;
				sprintf(filename, "%s.%d", file, ctr);
				UARTprintf("new file created: %s", filename);
				rc = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
				if(rc != FR_OK)
				{
					UARTprintf("f_open error\n");
					return 0;
				}
			}
			rc = f_write(&fil, buffer, 1, &bw);
			if(rc != FR_OK)
			{
				UARTprintf("f_write error\n");
				return 0;
			}
			UARTprintf("%d\n", bw);
			itr = itr + 1;
    	}*/
    }
}
