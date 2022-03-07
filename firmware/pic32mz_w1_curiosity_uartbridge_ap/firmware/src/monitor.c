/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    monitor.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "monitor.h"
#include "uart_bridge.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the MONITOR_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */



MONITOR_DATA monitorData;

//uint32_t seconds = 0;
//uint32_t minutes = 0;
//uint32_t hours = 0;
//uint32_t seconds_total = 0;
//bool status_display_flag = false;
//SYS_TIME_HANDLE timer_sec_hdl;
//bool trigger_every_second = false;
//TCPIP_NET_HANDLE wlan_net_hdl;
//TCPIP_NET_HANDLE eth_net_hdl;

extern EXCEPT_MSG last_expt_msg;
extern int RFMAC_count;
extern int ETHERNET_counter;

extern UART_BRIDGE_DATA uart_bridge;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void MONITOR_Initialize ( void )

  Remarks:
    See prototype in monitor.h.
 */

void MONITOR_TimerSecCallback(uintptr_t context) {
    monitorData.trigger_every_second = true;
}


void MONITOR_SetLEDState(int led, int state, int count) {
    switch(led){
        case MON_LED_RED:
            monitorData.led_red_state = state;
            monitorData.led_red_count = count;
            break;
        case MON_LED_GREEN:
            monitorData.led_green_state = state;
            monitorData.led_green_count = count;
            break;            
    }
}

void MONITOR_TimerBlinkCallback(uintptr_t context) {
    static int led_red_counter = 0;
    static int led_green_counter = 0;

    switch (monitorData.led_red_state) {
        case MON_LED_OFF:
            LED_RED_Clear();
            break;
        case MON_LED_ON:
            LED_RED_Set();
            break;
        case MON_LED_BLINK:
            if (led_red_counter == 0) {
                led_red_counter = monitorData.led_red_count;
                LED_RED_Toggle();
            }
            led_red_counter--;
            break;
    }

    switch (monitorData.led_green_state) {
        case MON_LED_OFF:
            LED_GREEN_Clear();
            break;
        case MON_LED_ON:
            LED_GREEN_Set();
            break;
        case MON_LED_BLINK:
            if (led_green_counter == 0) {
                led_green_counter = monitorData.led_green_count;
                LED_GREEN_Toggle();
            }
            led_green_counter--;
            break;
    }

}


void MONITOR_Initialize(void) {
    /* Place the App state machine in its initial state. */
    monitorData.state = MONITOR_STATE_INIT;

    monitorData.seconds = 0;
    monitorData.minutes = 0;
    monitorData.hours = 0;
    monitorData.seconds_total = 0;
    monitorData.status_display_flag = false;
    monitorData.timer_sec_hdl = 0;
    monitorData.trigger_every_second = false;
    monitorData.wlan_net_hdl = NULL;
    monitorData.eth_net_hdl = NULL;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */

    monitorData.timer_sec_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(MONITOR_TIMER_SEC_COUNT), &MONITOR_TimerSecCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    SYS_TIME_TimerStart(monitorData.timer_sec_hdl);
    
    LED_RED_OutputEnable();
    LED_GREEN_OutputEnable();
    LED_RED_Clear();
    LED_GREEN_Set();

    monitorData.timer_blink_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(MONITOR_TIMER_BLINK_COUNT), &MONITOR_TimerBlinkCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    SYS_TIME_TimerStart(monitorData.timer_blink_hdl);
    
}

void MONITOR_CheckForDHCPLease(void) {
    TCPIP_NET_HANDLE netHdl = TCPIP_STACK_NetHandleGet("PIC32MZWINT");
    TCPIP_DHCPS_LEASE_HANDLE dhcpsLease = 0;
    TCPIP_DHCPS_LEASE_ENTRY dhcpsLeaseEntry;
    static TCPIP_DHCPS_LEASE_ENTRY dhcpsLeaseEntry_old;

    dhcpsLease = TCPIP_DHCPS_LeaseEntryGet(netHdl, &dhcpsLeaseEntry, dhcpsLease);
    if (dhcpsLeaseEntry_old.ipAddress.Val == dhcpsLeaseEntry.ipAddress.Val) {
        return;
    }
    if (0 != dhcpsLease) {
        SYS_CONSOLE_PRINT("\r\nConnected ETH   IP:%d.%d.%d.%d   MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                dhcpsLeaseEntry.ipAddress.v[0], dhcpsLeaseEntry.ipAddress.v[1], dhcpsLeaseEntry.ipAddress.v[2], dhcpsLeaseEntry.ipAddress.v[3],
                dhcpsLeaseEntry.hwAdd.v[0], dhcpsLeaseEntry.hwAdd.v[1], dhcpsLeaseEntry.hwAdd.v[2], dhcpsLeaseEntry.hwAdd.v[3], dhcpsLeaseEntry.hwAdd.v[4], dhcpsLeaseEntry.hwAdd.v[5]);
        dhcpsLeaseEntry_old.ipAddress.Val = dhcpsLeaseEntry.ipAddress.Val;
    }
}

void MONITOR_SetDisplayStatus(bool flag) {
    monitorData.status_display_flag = flag;
}

void MONITOR_Display_Status(void) {
    char str[256];
    HeapStats_t xHeapStats;

    if (monitorData.seconds == 60) {
        monitorData.seconds = 0;
        if (monitorData.minutes == 59) {
            monitorData.minutes = 0;
            monitorData.hours++;
        }
        monitorData.minutes++;
    }
    monitorData.seconds++;
    monitorData.seconds_total++;

    if (monitorData.status_display_flag == false) return;

    vPortGetHeapStats(&xHeapStats);
/*
    sprintf(str,
            VT100_CURSOR_SAVE
            VT100_CURSOR_HOME
            VT100_BACKGROUND_WHITE
            VT100_FOREGROUND_BLUE
            VT100_CLR_LINE
            "Time:%02d:%02d:%02d Int:%d(RFMAC)%d(ETHERNET) Heap:%d\n\r"
            , monitorData.hours, monitorData.minutes, monitorData.seconds
            , RFMAC_count, ETHERNET_counter
            , xHeapStats.xAvailableHeapSpaceInBytes
            );

    SYS_CONSOLE_PRINT(str);
*/
    sprintf(str,
            VT100_BACKGROUND_WHITE
            VT100_FOREGROUND_MAGNETA
            VT100_CLR_LINE
            "UART Write:%d TCP Write:%d"
            VT100_BACKGROUND_BLACK
            VT100_FOREGROUND_GREEN
            VT100_CURSOR_RESTORE,
            uart_bridge.uart_write_count,
            uart_bridge.tcp_write_count
            );

    SYS_CONSOLE_PRINT(str);

}

/******************************************************************************
  Function:
    void MONITOR_Tasks ( void )

  Remarks:
    See prototype in monitor.h.
 */

void MONITOR_Tasks(void) {
    uint32_t DeviceID;

    if (monitorData.trigger_every_second) {
        monitorData.trigger_every_second = false;
        MONITOR_Display_Status();
        MONITOR_CheckForDHCPLease();
    }

    /* Check the application's current state. */
    switch (monitorData.state) {
            /* Application's initial state. */
        case MONITOR_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized) {

                monitorData.state = MONITOR_STATE_WAIT_FOR_TCP_STACK_READY;
            }
            break;
        }

        case MONITOR_STATE_WAIT_FOR_TCP_STACK_READY:
        {
            if (TCPIP_STACK_Status(sysObj.tcpip) == SYS_STATUS_READY) {
                DeviceID = DEVID;
                SYS_CONSOLE_PRINT(
                        "======================================================\n\r");
                SYS_CONSOLE_PRINT("L2 Bridge Build Time  " __DATE__ " " __TIME__ "\n\r");
                SYS_CONSOLE_PRINT("Build Stamp 202201121109 tc1\n\r");
                SYS_CONSOLE_PRINT("AP MODE\n\r");
                SYS_CONSOLE_PRINT("Device ID: %08x\n\r", DeviceID);
                SYS_CONSOLE_PRINT("Monitor Task State Run\n\r");
                if (last_expt_msg.magic == MAGIC_CODE) {
                    SYS_CONSOLE_PRINT(VT100_TEXT_DEFAULT "\n\r!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
                    SYS_CONSOLE_PRINT(VT100_TEXT_DEFAULT "Last Runtime has ended with the following Message:\n\r");
                    {
                        char ch;
                        int ix = 0;
                        for (ix = 0; ix < 4096; ix++) {
                            ch = last_expt_msg.msg[ix];
                            if (ch == 0)break;
                            SYS_CONSOLE_PRINT("%c", ch);
                        }
                    }
                    SYS_CONSOLE_PRINT("%c", last_expt_msg.msg[0]);
                    SYS_CONSOLE_PRINT(VT100_TEXT_DEFAULT "\n\r!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
                    last_expt_msg.magic = 0;
                }
                monitorData.wlan_net_hdl = TCPIP_STACK_IndexToNet(WLAN_NET);
                monitorData.eth_net_hdl = TCPIP_STACK_IndexToNet(ETH_NET);
                MONITOR_SetLEDState(MON_LED_RED,MON_LED_BLINK,1);
                monitorData.state = MONITOR_STATE_SERVICE_TASKS;
            }
            break;
        }

        case MONITOR_STATE_SERVICE_TASKS:
        {

            break;
        }

            /* TODO: implement your application state machine.*/


            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

#include <time.h>

time_t time(time_t * time_r) { /* seconds since 00:00:00 Jan 1 1970 */
    //    if(time_r){
    //        *time_r = seconds_total;
    //    }
    return monitorData.seconds_total;
}


void MONITOR_WifiCallback(uint32_t event, void * data, void *cookie) {
    switch (event) {
        case SYS_WIFI_DISCONNECT:
            SYS_CONSOLE_PRINT("WiFi Event DISCONNECT\n\r");
            MONITOR_SetLEDState(MON_LED_RED,MON_LED_BLINK,1);
            MONITOR_SetLEDState(MON_LED_GREEN,MON_LED_OFF,0);
            //monitorData.wlan_connect = false;
            break;
        case SYS_WIFI_CONNECT:
            SYS_CONSOLE_PRINT("WiFi Event CONNECT\n\r");
            MONITOR_SetLEDState(MON_LED_RED,MON_LED_ON,0);
            MONITOR_SetLEDState(MON_LED_GREEN,MON_LED_BLINK,1);
            //monitorData.wlan_connect = true;
            break;
    }

}

/*******************************************************************************
 End of File
 */
