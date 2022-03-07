/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    uart_bridge.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "UART_BRIDGE_Initialize" and "UART_BRIDGE_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "UART_BRIDGE_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _UART_BRIDGE_H
#define _UART_BRIDGE_H

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
#include "system_definitions.h"
#include "peripheral/uart/plib_uart1.h"



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
#define UART_BRIDGE_TRANSMIT_DATA_SIZE  4096
//#define UART_BRIDGE_ETH_BUFFER_SIZE     4096

#define SERVER_PORT 47111

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
        UART_BRIDGE_STATE_WAIT_FOR_TCP_INIT = 0,
        UART_BRIDGE_STATE_SERVICE_TASKS,
        UART_BRIDGE_TCPIP_OPENING_SERVER,
        UART_BRIDGE_TCPIP_WAIT_FOR_CONNECTION,
        UART_BRIDGE_TCPIP_SERVING_CONNECTION,
        UART_BRIDGE_TCPIP_CLOSING_CONNECTION,
        UART_BRIDGE_STATE_IDLE,
        UART_BRIDGE_STATE_ERROR,

        /* TODO: Define states used by the application state machine. */

    } UART_BRIDGE_STATES;


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
        uint8_t UartTransBuffer[UART_BRIDGE_TRANSMIT_DATA_SIZE];
//        uint8_t EthernetTransBuffer[UART_BRIDGE_ETH_BUFFER_SIZE];
        UART_BRIDGE_STATES state;
        int WriteIndex;
        int ReadIndex;
        uint32_t tcp_write_count;
        uint32_t uart_write_count;
        NET_PRES_SKT_HANDLE_T socket;
        TCP_PORT port;
        IP_MULTI_ADDRESS address;

        /* Device layer handle returned by device layer open function */
        USB_DEVICE_HANDLE deviceHandle;

        /* Device configured state */
        bool isConfigured;

        /* Write transfer handle */
        USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;

        /* Application CDC read buffer */
//        uint8_t * cdcReadBuffer;
//
//        /* Application CDC Write buffer */
//        uint8_t * cdcWriteBuffer;

        /* Number of bytes read from Host */
        uint32_t numBytesRead;
    } UART_BRIDGE_DATA;


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
        void UART_BRIDGE_Initialize ( void )

      Summary:
         MPLAB Harmony application initialization routine.

      Description:
        This function initializes the Harmony application.  It places the
        application in its initial state and prepares it to run so that its
        UART_BRIDGE_Tasks function can be called.

      Precondition:
        All other system initialization routines should be called before calling
        this routine (in "SYS_Initialize").

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        UART_BRIDGE_Initialize();
        </code>

      Remarks:
        This routine must be called from the SYS_Initialize function.
     */

    void UART_BRIDGE_Initialize(void);


    /*******************************************************************************
      Function:
        void UART_BRIDGE_Tasks ( void )

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
        UART_BRIDGE_Tasks();
        </code>

      Remarks:
        This routine must be called from SYS_Tasks() routine.
     */

    void UART_BRIDGE_Tasks(void);

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _UART_BRIDGE_H */

/*******************************************************************************
 End of File
 */

