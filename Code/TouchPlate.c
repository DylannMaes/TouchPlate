#include <device.h>
#include <LED_RGB.h>

extern uint16 CapSense_CSD_SensorSignal[CapSense_CSD_TOTAL_SENSOR_COUNT];

/* Capsense constants */
#define PROX_RANGE_INIT     100
#define UINT16_MAX          65535

void main()
{
    /* Variable declaration */
    int16 proximityCounts, proximityMax;
    uint16 hue=0;

    /* Initial delay to allow user to press reset and get away from the sensor */
    CyDelay(5000);

    CyGlobalIntEnable; /* Uncomment this line to enable global interrupts. */

    /* Start components */
    /* Start LED_RGB module */
    LED_RGB_Start();
    
    /* Start CSD proximity sensor */
    CapSense_CSD_Start();
    CapSense_CSD_EnableWidget(CapSense_CSD_SENSOR_PROXIMITYSENSOR0_0__PROX);
    CapSense_CSD_EnableWidget(CapSense_CSD_SENSOR_PROXIMITYSENSOR1_0__PROX);
    CapSense_CSD_EnableWidget(CapSense_CSD_SENSOR_PROXIMITYSENSOR2_0__PROX);
    CapSense_CSD_InitializeAllBaselines();
    
    /* Start UART */
    UART_Start();
    /* Send "initial" character over UART */
    UART_UartPutCRLF('I');

    /* Perform initial proximity read to set max and min */
    CapSense_CSD_UpdateEnabledBaselines();    
	/* Start scanning all enabled sensors */
	CapSense_CSD_ScanEnabledWidgets();
    /* Wait for scanning to complete */
	while(CapSense_CSD_IsBusy() != 0);
    
    /* Set initial limits */
    proximityMax = PROX_RANGE_INIT;

    /* Endless loop of proximity updates */
    for(;;)
    {
        /* Read capsense proximity sensor */
        /* Update all baselines */
        CapSense_CSD_UpdateEnabledBaselines();
	    /* Start scanning all enabled sensors */
	    CapSense_CSD_ScanEnabledWidgets();
        /* Wait for scanning to complete */
	    while(CapSense_CSD_IsBusy() != 0);
        proximityCounts = CapSense_CSD_SensorSignal[0] - 5;
        //proximityCounts = CapSense_CSD_SensorSignal[1] - 5;
        //proximityCounts = CapSense_CSD_SensorSignal[2] - 5;
        
        /* Floor the counts so no negative values are displayed */
        if(proximityCounts<0)
        {
            proximityCounts=0;
        }
        
        /* Set new limits if appropriate */
        if(proximityCounts>proximityMax)
        {
            proximityMax = proximityCounts;
        }
        
        /* Calculate scaled hue value and write it */
        hue = ((uint32) proximityCounts) * UINT16_MAX / proximityMax;
        
        LED_RGB_SetColorCircle(hue);
        
        /* Print the capsense proximity to the UART for reading in Bridge Control Panel */
        /* Read with BCP command "RX8 [h=43] @1prox @0prox" */
        UART_UartPutChar('C');
        UART_UartPutChar(proximityCounts>>8);
        UART_UartPutChar(proximityCounts&0xff); 
        /* Delay to keep UART traffic down */
        CyDelay(10);
    }
}