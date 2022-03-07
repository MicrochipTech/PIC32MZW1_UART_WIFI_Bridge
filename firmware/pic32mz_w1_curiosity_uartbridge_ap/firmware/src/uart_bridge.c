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


/* TODO:  Add any necessary local functions.
 */
#define CMD_MAX_CMD             3
#define BAUDRATE_CMD_115200     "\x1b""XRELOAF"" LF"
#define BAUDRATE_CMD_19200      "\x1b""XRELOAD"" LF"
#define BAUDRATE_CMD_9600       "\x1b""XRELOAG"" LF"


#define CMD_MAX_SIZE            48
#define CMD_RAMTYPE_N_BYTES     4
#define CMD_STUFFBYTE_N_BYTES   19

char cmd_list[CMD_MAX_CMD][CMD_MAX_SIZE] = {
    BAUDRATE_CMD_115200,
    BAUDRATE_CMD_19200,
    BAUDRATE_CMD_9600
};

//void DumpMemory(uint8_t *pData, uint32_t count);
void UART_BRIDGE_uart1_Initialize(uint32_t baud_reg_value);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
bool disp_cmd = true;

void UART_BRIDGE_Command_Parser(uint8_t rcv) {
    static int state = 0;
    static char rcv_str[CMD_MAX_SIZE];
    static int ix = 0;
    static int ix_cmd = 0;
    static int baud = 0;

    switch (state) {

        case 0:
            memset(rcv_str, 0, CMD_MAX_SIZE);
            ix = 0;
            state = 1;

        case 1:
            /* check for ESCAPE code  */
            if (rcv == '\x1b') {
                rcv_str[ix++] = rcv;
                state = 2;
            }
            break;

        case 2:
            /* collect data */
            rcv_str[ix++] = rcv;
            /* check for delimeter LF */
            if (rcv == 'L') {
                state = 3;
            }
            break;

        case 3:
            rcv_str[ix++] = rcv;
            /* if delimiter is detected 
             * process the data 
             */
            if (rcv == 'F') {
                /* stuff a zero to separate 
                 * the string from the data 
                 */
                rcv_str[ix] = 0;
                ix++;
                ix_cmd = ix;
                /* if no command is found 
                 * start the detection from 
                 * the beginning 
                 */
                int i;
                /* check for different command */
                for (i = 0; i < CMD_MAX_CMD; i++) {
                    if (strcmp(rcv_str, (char*) &cmd_list[i][0]) == 0) {
                        if (i == 0) {
                            baud = 115200;
                            state = 4;
                            break;
                        }
                        if (i == 1) {
                            baud = 19200;
                            state = 4;
                            break;
                        }
                        if (i == 2) {
                            baud = 9600;
                            state = 4;
                            break;
                        }
                    }
                }
                if (state == 3) {
                    state = 0;
                }
            } else {
                /* was not Delimiter */
                state = 2;
            }
            break;

        case 4:
            /* collect further data and stuff bytes */
            rcv_str[ix++] = rcv;
            if (ix == (ix_cmd + CMD_RAMTYPE_N_BYTES + CMD_STUFFBYTE_N_BYTES + 1)) {
                if (baud == 115200) {
                    state = 5;
                } else if (baud == 19200) {
                    state = 6;
                } else if (baud == 9600) {
                    state = 7;
                } else {
                    state = 0;
                }
            }
            break;

        case 5:
            /* BAUD Rate register Setup for 115200 */
            UART_BRIDGE_uart1_Initialize(UART_BAUD_115200);
            SYS_CONSOLE_PRINT("Set UART1 Baudrate to 115200\n\r");
            state = 0;
            break;

        case 6:
            /* BAUD Rate register Setup for 19200 */
            UART_BRIDGE_uart1_Initialize(UART_BAUD_19200);
            SYS_CONSOLE_PRINT("Set UART1 Baudrate to 19200\n\r");
            //DumpMemory((uint8_t *) rcv_str, CMD_MAX_SIZE);
            state = 0;
            break;

        case 7:
            /* BAUD Rate register Setup for 9600 */
            UART_BRIDGE_uart1_Initialize(UART_BAUD_9600);
            SYS_CONSOLE_PRINT("Set UART1 Baudrate to 9600\n\r");
            //DumpMemory((uint8_t *) rcv_str, CMD_MAX_SIZE);
            state = 0;
            break;
    }

    if (ix == CMD_MAX_SIZE) {
        SYS_CONSOLE_PRINT("Max size\n\r");
        //DumpMemory((uint8_t *) rcv_str, CMD_MAX_SIZE);
        state = 0;
    }
}

void UART_BRIDGE_RX_InterruptHandler(void) {

    /* Clear UART1 RX Interrupt flag */
    IFS1CLR = _IFS1_U1RXIF_MASK;

    while (_U1STA_URXDA_MASK == (U1STA & _U1STA_URXDA_MASK)) {
        uart_bridge.UartRecvBuffer[uart_bridge.WriteIndex++] = (uint8_t) (U1RXREG);
        if (uart_bridge.WriteIndex == UART_BRIDGE_RECEIVE_DATA_SIZE) {
            uart_bridge.WriteIndex = 0;
        }
    }

}

void UART_BRIDGE_FAULT_InterruptHandler(void) {
    /* Disable the fault interrupt */
    IEC1CLR = _IEC1_U1EIE_MASK;
    /* Disable the receive interrupt */
    IEC1CLR = _IEC1_U1RXIE_MASK;

    /* Client must call UARTx_ErrorGet() function to clear the errors */
    UART_ERROR errors = UART_ERROR_NONE;
    uint32_t status = U1STA;

    errors = (UART_ERROR) (status & (_U1STA_OERR_MASK | _U1STA_FERR_MASK | _U1STA_PERR_MASK));

    if (errors != UART_ERROR_NONE) {
        /* rxBufferLen = (FIFO level + RX register) */
        uint8_t rxBufferLen = UART_RXFIFO_DEPTH;
        uint8_t dummyData = 0u;

        /* If it's a overrun error then clear it to flush FIFO */
        if (U1STA & _U1STA_OERR_MASK) {
            U1STACLR = _U1STA_OERR_MASK;
        }

        /* Read existing error bytes from FIFO to clear parity and framing error flags */
        while (U1STA & (_U1STA_FERR_MASK | _U1STA_PERR_MASK)) {
            dummyData = (uint8_t) (U1RXREG);
            rxBufferLen--;

            /* Try to flush error bytes for one full FIFO and exit instead of
             * blocking here if more error bytes are received */
            if (rxBufferLen == 0u) {
                break;
            }
        }

        // Ignore the warning
        (void) dummyData;

        /* Clear error interrupt flag */
        IFS1CLR = _IFS1_U1EIF_MASK;

        /* Clear up the receive interrupt flag so that RX interrupt is not
         * triggered for error bytes */
        IFS1CLR = _IFS1_U1RXIF_MASK;
    }

    /* All errors are cleared, but send the previous error state */
}

int32_t UART_BRIDGE_GetReadSize(void) {
    int32_t size;

    size = uart_bridge.WriteIndex - uart_bridge.ReadIndex;
    if (size < 0) {
        size += UART_BRIDGE_RECEIVE_DATA_SIZE;
    }

    return size;
}

void UART_BRIDGE_Read(uint8_t *buffer, int size) {

    while (size) {
        *buffer++ = uart_bridge.UartRecvBuffer[uart_bridge.ReadIndex++];
        if (uart_bridge.ReadIndex == UART_BRIDGE_RECEIVE_DATA_SIZE) {
            uart_bridge.ReadIndex = 0;
        }
        size--;
    };

}

/*******************************************************************************
  Function:
    void UART_BRIDGE_Initialize ( void )

  Remarks:
    See prototype in uart_bridge.h.
 */

void UART_BRIDGE_Initialize(void) {
    /* Place the App state machine in its initial state. */
    uart_bridge.state = UART_BRIDGE_STATE_INIT;
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
        case UART_BRIDGE_STATE_INIT:
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
                UART_BRIDGE_uart1_Initialize(UART_BAUD_115200);
                IEC1SET = _IEC1_U1RXIE_MASK;
                uart_bridge.state = UART_BRIDGE_TCPIP_OPENING_SERVER;
                break;
            }
        }

        case UART_BRIDGE_STATE_SERVICE_TASKS:
        {
//            /* Loopback for hardware test */
//            int32_t size;
//            if (UART1_WriteIsBusy() == false) {
//                size = UART_BRIDGE_Read(uart_bridge.UartTransBuffer);
//                if (size > 0) {
//                    UART1_Write(uart_bridge.UartTransBuffer, size);
//                }
//            }
            break;
        }

        case UART_BRIDGE_TCPIP_OPENING_SERVER:
        {
            SYS_CONSOLE_PRINT("Waiting for Client Connection on port: %d\r\n", SERVER_PORT);
            uart_bridge.socket = NET_PRES_SocketOpen(0, NET_PRES_SKT_DEFAULT_STREAM_SERVER, IP_ADDRESS_TYPE_ANY, SERVER_PORT, 0, 0);
            if (uart_bridge.socket == INVALID_SOCKET) {
                SYS_CONSOLE_PRINT("Couldn't open server socket\r\n");
                break;
            }
            uart_bridge.state = UART_BRIDGE_TCPIP_WAIT_FOR_CONNECTION;
        }
            break;

        case UART_BRIDGE_TCPIP_WAIT_FOR_CONNECTION:
        {
            if (!NET_PRES_SocketIsConnected(uart_bridge.socket)) {
                vTaskDelay(10 / portTICK_PERIOD_MS);
                return;
            } else {
                // We got a connection
                uart_bridge.state = UART_BRIDGE_TCPIP_SERVING_CONNECTION;
                /* Enable UART1_FAULT Interrupt */
                IEC1SET = _IEC1_U1EIE_MASK;
                /* Enable UART1_RX Interrupt */
                IEC1SET = _IEC1_U1RXIE_MASK;
                SYS_CONSOLE_PRINT("Received a UART Bridge connection\r\n");
                MONITOR_SetLEDState(MON_LED_GREEN,MON_LED_ON,0);
                //UART_BRIDGE_CONSOLE_PRINT("All Commands in List\r\n");
                //DumpMemory((uint8_t*) cmd_list, CMD_MAX_CMD * CMD_MAX_SIZE);
            }
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
            int32_t uart_rcv_size;

            // Figure out how many bytes have been received and how many we can transmit.
            wMaxGet = NET_PRES_SocketReadIsReady(uart_bridge.socket); // Get TCP RX FIFO byte count
            wMaxPut = NET_PRES_SocketWriteIsReady(uart_bridge.socket, 1, 0); // Get TCP TX FIFO free space

            if (wMaxPut > 0) {
                uart_rcv_size = UART_BRIDGE_GetReadSize();
                if (uart_rcv_size > 0) {
                    if (wMaxPut < uart_rcv_size) uart_rcv_size = wMaxPut;
                    UART_BRIDGE_Read(uart_bridge.UartTransBuffer, uart_rcv_size);
                    NET_PRES_SocketWrite(uart_bridge.socket, uart_bridge.UartTransBuffer, uart_rcv_size);
                    LED_RED_Clear(); // Signaling that data is transfered to TCP
                    uart_bridge.tcp_write_count += uart_rcv_size;
                }
            }

            if (wMaxGet > 0) {
                if (UART1_WriteIsBusy() == false) {
                    NET_PRES_SocketRead(uart_bridge.socket, uart_bridge.EthernetTransBuffer, wMaxGet);
                    int ix;
                    for (ix = 0; ix < wMaxGet; ix++) {
                        UART_BRIDGE_Command_Parser(uart_bridge.EthernetTransBuffer[ix]);
                    }
                    UART1_Write(uart_bridge.EthernetTransBuffer, wMaxGet);
                    LED_GREEN_Clear(); // Signaling that data is transfered to UART
                    uart_bridge.uart_write_count += wMaxGet;
                }
            }

        }
            break;

        case UART_BRIDGE_TCPIP_CLOSING_CONNECTION:
        {
            // Close the socket connection.
            NET_PRES_SocketClose(uart_bridge.socket);
            SYS_CONSOLE_PRINT("closing connection\r\n");
            uart_bridge.socket = INVALID_SOCKET;
            uart_bridge.state = UART_BRIDGE_STATE_INIT;

        }
            break;

        case UART_BRIDGE_STATE_IDLE:
            break;

            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

void UART_BRIDGE_uart1_Initialize(uint32_t baud_reg_value) {
    /* Set up UxMODE bits */
    /* STSEL  = 0*/
    /* PDSEL = 0 */
    /* BRGH = 1 */
    /* RXINV = 0 */
    /* ABAUD = 0 */
    /* LPBACK = 0 */
    /* WAKE = 0 */
    /* SIDL = 0 */
    /* RUNOVF = 0 */
    /* CLKSEL = 0 */
    /* SLPEN = 0 */
    /* UEN = 0 */
    U1MODE = 0x8;

    /* Enable UART1 Receiver, Transmitter and TX Interrupt selection */
    U1STASET = (_U1STA_UTXEN_MASK | _U1STA_URXEN_MASK | _U1STA_UTXISEL1_MASK);

    /* BAUD Rate register Setup */
    U1BRG = baud_reg_value;

    /* Disable Interrupts */
    IEC1CLR = _IEC1_U1EIE_MASK;

    IEC1CLR = _IEC1_U1RXIE_MASK;

    IEC1CLR = _IEC1_U1TXIE_MASK;

    /* Turn ON UART1 */
    U1MODESET = _U1MODE_ON_MASK;

    /* Enable UART1_RX Interrupt */
    IEC1SET = _IEC1_U1RXIE_MASK;
}



/*******************************************************************************
 End of File
 */
