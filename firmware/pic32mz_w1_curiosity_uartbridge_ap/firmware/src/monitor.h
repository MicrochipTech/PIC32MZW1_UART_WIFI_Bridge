/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    monitor.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "MONITOR_Initialize" and "MONITOR_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "MONITOR_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _MONITOR_H
#define _MONITOR_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"
#include "vt100.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    // DOM-IGNORE-END

    // *****************************************************************************
    // *****************************************************************************
    // Section: Type Definitions
    // *****************************************************************************
    // *****************************************************************************

#define MONITOR_TIMER_SEC_COUNT      1000
#define MONITOR_DELAY_1_SECOND       (1*(1000/MONITOR_TIMER_SEC_COUNT))
#define MONITOR_DELAY_2_SECONDS      (2*(1000/MONITOR_TIMER_SEC_COUNT)) 

#define MONITOR_TIMER_BLINK_COUNT    250

#define WLAN_NET 0
#define ETH_NET  1

#define MON_LED_RED     0
#define MON_LED_GREEN   1
#define MON_LED_OFF     0
#define MON_LED_ON      1
#define MON_LED_BLINK   2


void MONITOR_SetLEDState(int led, int state, int count);

    // *****************************************************************************

    /* Application states

      Summary:
        Application states enumeration

      Description:
        This enumeration defines the valid application states.  These states
        determine the behavior of the application at various times.
     */

    typedef enum {
        /* Application's state machine's initial state. */
        MONITOR_STATE_INIT = 0,
        MONITOR_STATE_WAIT_FOR_TCP_STACK_READY,
        MONITOR_STATE_SERVICE_TASKS,
        /* TODO: Define states used by the application state machine. */

    } MONITOR_STATES;


    // *****************************************************************************

    /* Application Data

      Summary:
        Holds application data

      Description:
        This structure holds the application's data.

      Remarks:
        Application strings and buffers are be defined outside this structure.
     */

    typedef struct {
        /* The application's current state */
        MONITOR_STATES state;

        uint32_t seconds;
        uint32_t minutes;
        uint32_t hours;
        uint32_t seconds_total;
        bool status_display_flag;
        SYS_TIME_HANDLE timer_sec_hdl;
        SYS_TIME_HANDLE timer_blink_hdl;
        bool trigger_every_second;
        TCPIP_NET_HANDLE wlan_net_hdl;
        TCPIP_NET_HANDLE eth_net_hdl;

        int led_red_state;
        int led_red_count;
        int led_green_state;
        int led_green_count;
        
    } MONITOR_DATA;

    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Callback Routines
    // *****************************************************************************
    // *****************************************************************************
    /* These routines are called by drivers when certain events occur.
     */

    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Initialization and State Machine Functions
    // *****************************************************************************
    // *****************************************************************************

    /*******************************************************************************
      Function:
        void MONITOR_Initialize ( void )

      Summary:
         MPLAB Harmony application initialization routine.

      Description:
        This function initializes the Harmony application.  It places the
        application in its initial state and prepares it to run so that its
        MONITOR_Tasks function can be called.

      Precondition:
        All other system initialization routines should be called before calling
        this routine (in "SYS_Initialize").

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        MONITOR_Initialize();
        </code>

      Remarks:
        This routine must be called from the SYS_Initialize function.
     */

    void MONITOR_Initialize(void);


    /*******************************************************************************
      Function:
        void MONITOR_Tasks ( void )

      Summary:
        MPLAB Harmony Demo application tasks function

      Description:
        This routine is the Harmony Demo application's tasks function.  It
        defines the application's state machine and core logic.

      Precondition:
        The system and application initialization ("SYS_Initialize") should be
        called before calling this.

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        MONITOR_Tasks();
        </code>

      Remarks:
        This routine must be called from SYS_Tasks() routine.
     */

    void MONITOR_Tasks(void);

    void MONITOR_WifiCallback(uint32_t event, void * data, void *cookie) ;
    
    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _MONITOR_H */

/*******************************************************************************
 End of File
 */

