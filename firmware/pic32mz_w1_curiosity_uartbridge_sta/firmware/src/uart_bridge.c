/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    uart_bridge.c

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
#include <string.h>
#include "usb/usb_device_cdc.h"
#include "uart_bridge.h"
#include "monitor.h"

///??????????????????????????????????????????????????????????????????????????????????????
#include "config/pic32mz_w1_eth_wifi_freertos/system/console/src/sys_console_usb_cdc.h"
uint16_t USB_DEVICE_CDC_WritePacketSizeGet(USB_DEVICE_CDC_INDEX iCDC);
///??????????????????????????????????????????????????????????????????????????????????????

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

extern MONITOR_DATA monitorData;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the UART_BRIDGE_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */

UART_BRIDGE_DATA uart_bridge;

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

/*******************************************************************************
  Function:
    void UART_BRIDGE_Initialize ( void )

  Remarks:
    See prototype in uart_bridge.h.
 */

void UART_BRIDGE_Initialize(void) {
    /* Place the App state machine in its initial state. */
    uart_bridge.state = UART_BRIDGE_STATE_WAIT_FOR_TCP_INIT;

    uart_bridge.port = SERVER_PORT;
    uart_bridge.address.v4Add.v[0] = 192;
    uart_bridge.address.v4Add.v[1] = 168;
    uart_bridge.address.v4Add.v[2] = 1;
    uart_bridge.address.v4Add.v[3] = 1;

    /* Device Layer Handle  */
    uart_bridge.deviceHandle = USB_DEVICE_HANDLE_INVALID;

    /* Device configured status */
    uart_bridge.isConfigured = false;

    /* Write Transfer Handle */
    uart_bridge.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;


    /* Set up the read buffer */
    //   uart_bridge.cdcReadBuffer = &uart_bridge.UartTransBuffer[0];

    /* Set up the read buffer */
    //    uart_bridge.cdcWriteBuffer = &uart_bridge.UartTransBuffer[0];

    uart_bridge.uart_write_count = 0;
    uart_bridge.tcp_write_count = 0;

}

/******************************************************************************
  Function:
    void UART_BRIDGE_Tasks ( void )

  Remarks:
    See prototype in uart_bridge.h.
 */


void UART_BRIDGE_Tasks(void) {

    /* Check the application's current state. */
    switch (uart_bridge.state) {
            /* Application's initial state. */


        case UART_BRIDGE_STATE_WAIT_FOR_TCP_INIT:
        {
            if (TCPIP_STACK_Status(sysObj.tcpip) == SYS_STATUS_READY) {
                SYS_CONSOLE_PRINT("UART Bridge State Run\n\r");
                uart_bridge.WriteIndex = 0;
                uart_bridge.ReadIndex = 0;
                uart_bridge.tcp_write_count = 0;
                NET_PRES_SocketClose(uart_bridge.socket);
                uart_bridge.socket = INVALID_SOCKET;
                /* Enable UART1_FAULT Interrupt */
                //IEC1SET = _IEC1_U1EIE_MASK;
                /* Enable UART1_RX Interrupt */
                /* BAUD Rate register Setup for 115200 */
                uart_bridge.state = UART_BRIDGE_STATE_SERVICE_TASKS;
                break;
            }
        }

        case UART_BRIDGE_STATE_SERVICE_TASKS:
        {
            if (monitorData.wlan_connect == true) {
                uart_bridge.state = UART_BRIDGE_TCPIP_OPENING_SERVER;
                SYS_CONSOLE_PRINT("Opening Server\n\r");
            }
        }
            break;

        case UART_BRIDGE_TCPIP_OPENING_SERVER:
        {
            uart_bridge.socket = NET_PRES_SocketOpen(0,
                    NET_PRES_SKT_UNENCRYPTED_STREAM_CLIENT,
                    IP_ADDRESS_TYPE_IPV4,
                    uart_bridge.port,
                    (NET_PRES_ADDRESS *) & uart_bridge.address,
                    NULL);
            NET_PRES_SocketWasReset(uart_bridge.socket);
            if (uart_bridge.socket == INVALID_SOCKET) {
                SYS_CONSOLE_MESSAGE("UART Bridge Could not create socket - aborting\r\n");
                uart_bridge.state = UART_BRIDGE_STATE_IDLE;
            }
            uart_bridge.state = UART_BRIDGE_TCPIP_WAIT_FOR_CONNECTION;
        }
            break;

        case UART_BRIDGE_TCPIP_WAIT_FOR_CONNECTION:
        {
            if (!NET_PRES_SocketIsConnected(uart_bridge.socket)) {
                break;
            }
            SYS_CONSOLE_MESSAGE("Connection Opened: Starting Clear Text Communication\r\n");
            MONITOR_SetLEDState(MON_LED_GREEN,MON_LED_ON,0);
            uart_bridge.state = UART_BRIDGE_TCPIP_SERVING_CONNECTION;
        }
            break;

        case UART_BRIDGE_TCPIP_SERVING_CONNECTION:
        {
            if (!NET_PRES_SocketIsConnected(uart_bridge.socket) || NET_PRES_SocketWasDisconnected(uart_bridge.socket)) {
                uart_bridge.state = UART_BRIDGE_TCPIP_CLOSING_CONNECTION;
                SYS_CONSOLE_PRINT("Connection was closed\r\n");
                break;
            }

            int16_t wMaxGet;
            int16_t wMaxPut;

            /* Figure out how many bytes have been received and how many we can transmit. */
            wMaxGet = NET_PRES_SocketReadIsReady(uart_bridge.socket); // Get TCP RX FIFO byte count
            wMaxPut = NET_PRES_SocketWriteIsReady(uart_bridge.socket, 1, 0); // Get TCP TX FIFO free space

            if (wMaxPut > 0) {
                uart_bridge.numBytesRead = Console_USB_CDC_ReadCountGet(USB_DEVICE_CDC_INDEX_0);
                if (wMaxPut < uart_bridge.numBytesRead) uart_bridge.numBytesRead = wMaxPut;
                if (wMaxPut > UART_BRIDGE_TRANSMIT_DATA_SIZE) uart_bridge.numBytesRead = UART_BRIDGE_TRANSMIT_DATA_SIZE;
                if (uart_bridge.numBytesRead > 0) {
                    Console_USB_CDC_Read(0, uart_bridge.UartTransBuffer, uart_bridge.numBytesRead);
                    NET_PRES_SocketWrite(uart_bridge.socket, uart_bridge.UartTransBuffer, uart_bridge.numBytesRead);
                    LED_RED_Clear(); // Signaling that data is transfered to TCP
                    uart_bridge.tcp_write_count += uart_bridge.numBytesRead;
                }
            }

            if (wMaxGet > 0) {
                int maxCDC = Console_USB_CDC_WriteFreeBufferCountGet(USB_DEVICE_CDC_INDEX_0);
                if (wMaxGet > maxCDC)wMaxGet = maxCDC;
                if (wMaxGet > UART_BRIDGE_TRANSMIT_DATA_SIZE) wMaxGet = UART_BRIDGE_TRANSMIT_DATA_SIZE;
                NET_PRES_SocketRead(uart_bridge.socket, uart_bridge.UartTransBuffer, wMaxGet);
                Console_USB_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        uart_bridge.UartTransBuffer, wMaxGet);
                LED_GREEN_Clear(); // Signaling that data is transfered to UART
                uart_bridge.uart_write_count += wMaxGet;
            }

        }
            break;

        case UART_BRIDGE_TCPIP_CLOSING_CONNECTION:
        {
            // Close the socket connection.
            NET_PRES_SocketClose(uart_bridge.socket);
            SYS_CONSOLE_PRINT("UART Bridge closing connection\r\n");
            uart_bridge.socket = INVALID_SOCKET;
            uart_bridge.state = UART_BRIDGE_STATE_WAIT_FOR_TCP_INIT;

        }
            break;

        case UART_BRIDGE_STATE_ERROR:
        case UART_BRIDGE_STATE_IDLE:
        {
        }
            break;

    }
}



/*******************************************************************************
 End of File
 */
