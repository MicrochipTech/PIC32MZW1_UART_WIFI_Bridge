/*******************************************************************************
  UART1 PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_uart1.c

  Summary:
    UART1 PLIB Implementation File

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#include "device.h"
#include "plib_uart1.h"

// *****************************************************************************
// *****************************************************************************
// Section: UART1 Implementation
// *****************************************************************************
// *****************************************************************************

UART_OBJECT uart1Obj;

void static UART1_ErrorClear( void )
{
    UART_ERROR errors = UART_ERROR_NONE;
    uint8_t dummyData = 0u;

    errors = (UART_ERROR)(U1STA & (_U1STA_OERR_MASK | _U1STA_FERR_MASK | _U1STA_PERR_MASK));

    if(errors != UART_ERROR_NONE)
    {
        /* If it's a overrun error then clear it to flush FIFO */
        if(U1STA & _U1STA_OERR_MASK)
        {
            U1STACLR = _U1STA_OERR_MASK;
        }

        /* Read existing error bytes from FIFO to clear parity and framing error flags */
        while(U1STA & _U1STA_URXDA_MASK)
        {
            dummyData = U1RXREG;
        }

        /* Clear error interrupt flag */
        IFS1CLR = _IFS1_U1EIF_MASK;

        /* Clear up the receive interrupt flag so that RX interrupt is not
         * triggered for error bytes */
        IFS1CLR = _IFS1_U1RXIF_MASK;
    }

    // Ignore the warning
    (void)dummyData;
}

void UART1_Initialize( void )
{
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
    U1STASET = (_U1STA_UTXEN_MASK | _U1STA_URXEN_MASK | _U1STA_UTXISEL1_MASK );

    /* BAUD Rate register Setup */
    U1BRG = 216;

    /* Disable Interrupts */
    IEC1CLR = _IEC1_U1EIE_MASK;

    IEC1CLR = _IEC1_U1RXIE_MASK;

    IEC1CLR = _IEC1_U1TXIE_MASK;

    /* Initialize instance object */
    uart1Obj.rxBuffer = NULL;
    uart1Obj.rxSize = 0;
    uart1Obj.rxProcessedSize = 0;
    uart1Obj.rxBusyStatus = false;
    uart1Obj.rxCallback = NULL;
    uart1Obj.txBuffer = NULL;
    uart1Obj.txSize = 0;
    uart1Obj.txProcessedSize = 0;
    uart1Obj.txBusyStatus = false;
    uart1Obj.txCallback = NULL;
    uart1Obj.errors = UART_ERROR_NONE;

    /* Turn ON UART1 */
    U1MODESET = _U1MODE_ON_MASK;
}

bool UART1_SerialSetup( UART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    bool status = false;
    uint32_t baud;
    int32_t brgValHigh = 0;
    int32_t brgValLow = 0;
    uint32_t brgVal = 0;
    uint32_t uartMode;

    if((uart1Obj.rxBusyStatus == true) || (uart1Obj.txBusyStatus == true))
    {
        /* Transaction is in progress, so return without updating settings */
        return status;
    }

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if (baud == 0)
        {
            return status;
        }

        /* Turn OFF UART1 */
        U1MODECLR = _U1MODE_ON_MASK;

        if(srcClkFreq == 0)
        {
            srcClkFreq = UART1_FrequencyGet();
        }

        /* Calculate BRG value */
        brgValLow = (((srcClkFreq >> 4) + (baud >> 1)) / baud ) - 1;
        brgValHigh = (((srcClkFreq >> 2) + (baud >> 1)) / baud ) - 1;

        /* Check if the baud value can be set with low baud settings */
        if((brgValLow >= 0) && (brgValLow <= UINT16_MAX))
        {
            brgVal =  brgValLow;
            U1MODECLR = _U1MODE_BRGH_MASK;
        }
        else if ((brgValHigh >= 0) && (brgValHigh <= UINT16_MAX))
        {
            brgVal = brgValHigh;
            U1MODESET = _U1MODE_BRGH_MASK;
        }
        else
        {
            return status;
        }

        if(setup->dataWidth == UART_DATA_9_BIT)
        {
            if(setup->parity != UART_PARITY_NONE)
            {
               return status;
            }
            else
            {
               /* Configure UART1 mode */
               uartMode = U1MODE;
               uartMode &= ~_U1MODE_PDSEL_MASK;
               U1MODE = uartMode | setup->dataWidth;
            }
        }
        else
        {
            /* Configure UART1 mode */
            uartMode = U1MODE;
            uartMode &= ~_U1MODE_PDSEL_MASK;
            U1MODE = uartMode | setup->parity ;
        }

        /* Configure UART1 mode */
        uartMode = U1MODE;
        uartMode &= ~_U1MODE_STSEL_MASK;
        U1MODE = uartMode | setup->stopBits ;

        /* Configure UART1 Baud Rate */
        U1BRG = brgVal;

        U1MODESET = _U1MODE_ON_MASK;

        status = true;
    }

    return status;
}

bool UART1_Read(void* buffer, const size_t size )
{
    bool status = false;
    uint8_t* lBuffer = (uint8_t* )buffer;

    if(lBuffer != NULL)
    {
        /* Check if receive request is in progress */
        if(uart1Obj.rxBusyStatus == false)
        {
            /* Clear error flags and flush out error data that may have been received when no active request was pending */
            UART1_ErrorClear();

            uart1Obj.rxBuffer = lBuffer;
            uart1Obj.rxSize = size;
            uart1Obj.rxProcessedSize = 0;
            uart1Obj.rxBusyStatus = true;
            uart1Obj.errors = UART_ERROR_NONE;
            status = true;

            /* Enable UART1_FAULT Interrupt */
            IEC1SET = _IEC1_U1EIE_MASK;

            /* Enable UART1_RX Interrupt */
            IEC1SET = _IEC1_U1RXIE_MASK;
        }
    }

    return status;
}

bool UART1_Write( void* buffer, const size_t size )
{
    bool status = false;
    uint8_t* lBuffer = (uint8_t*)buffer;

    if(lBuffer != NULL)
    {
        /* Check if transmit request is in progress */
        if(uart1Obj.txBusyStatus == false)
        {
            uart1Obj.txBuffer = lBuffer;
            uart1Obj.txSize = size;
            uart1Obj.txProcessedSize = 0;
            uart1Obj.txBusyStatus = true;
            status = true;

            /* Initiate the transfer by writing as many bytes as we can */
             while((!(U1STA & _U1STA_UTXBF_MASK)) && (uart1Obj.txSize > uart1Obj.txProcessedSize) )
            {
                if (( U1MODE & (_U1MODE_PDSEL0_MASK | _U1MODE_PDSEL1_MASK)) == (_U1MODE_PDSEL0_MASK | _U1MODE_PDSEL1_MASK))
                {
                    /* 9-bit mode */
                    U1TXREG = ((uint16_t*)uart1Obj.txBuffer)[uart1Obj.txProcessedSize++];
                }
                else
                {
                    /* 8-bit mode */
                    U1TXREG = uart1Obj.txBuffer[uart1Obj.txProcessedSize++];
                }
            }

            IEC1SET = _IEC1_U1TXIE_MASK;
        }
    }

    return status;
}

UART_ERROR UART1_ErrorGet( void )
{
    UART_ERROR errors = uart1Obj.errors;

    uart1Obj.errors = UART_ERROR_NONE;

    /* All errors are cleared, but send the previous error state */
    return errors;
}

bool UART1_AutoBaudQuery( void )
{
    if(U1MODE & _U1MODE_ABAUD_MASK)
        return true;
    else
        return false;
}

void UART1_AutoBaudSet( bool enable )
{
    if( enable == true )
    {
        U1MODESET = _U1MODE_ABAUD_MASK;
    }

    /* Turning off ABAUD if it was on can lead to unpredictable behavior, so that
       direction of control is not allowed in this function.                      */
}

void UART1_ReadCallbackRegister( UART_CALLBACK callback, uintptr_t context )
{
    uart1Obj.rxCallback = callback;

    uart1Obj.rxContext = context;
}

bool UART1_ReadIsBusy( void )
{
    return uart1Obj.rxBusyStatus;
}

size_t UART1_ReadCountGet( void )
{
    return uart1Obj.rxProcessedSize;
}

bool UART1_ReadAbort(void)
{
    if (uart1Obj.rxBusyStatus == true)
    {
        /* Disable the fault interrupt */
        IEC1CLR = _IEC1_U1EIE_MASK;

        /* Disable the receive interrupt */
        IEC1CLR = _IEC1_U1RXIE_MASK;

        uart1Obj.rxBusyStatus = false;

        /* If required application should read the num bytes processed prior to calling the read abort API */
        uart1Obj.rxSize = uart1Obj.rxProcessedSize = 0;
    }

    return true;
}

void UART1_WriteCallbackRegister( UART_CALLBACK callback, uintptr_t context )
{
    uart1Obj.txCallback = callback;

    uart1Obj.txContext = context;
}

bool UART1_WriteIsBusy( void )
{
    return uart1Obj.txBusyStatus;
}

size_t UART1_WriteCountGet( void )
{
    return uart1Obj.txProcessedSize;
}

void UART1_FAULT_InterruptHandler (void)
{
    /* Save the error to be reported later */
    uart1Obj.errors = (UART_ERROR)(U1STA & (_U1STA_OERR_MASK | _U1STA_FERR_MASK | _U1STA_PERR_MASK));

    /* Disable the fault interrupt */
    IEC1CLR = _IEC1_U1EIE_MASK;

    /* Disable the receive interrupt */
    IEC1CLR = _IEC1_U1RXIE_MASK;

    /* Clear size and rx status */
    uart1Obj.rxBusyStatus = false;

    UART1_ErrorClear();

    /* Client must call UARTx_ErrorGet() function to get the errors */
    if( uart1Obj.rxCallback != NULL )
    {
        uart1Obj.rxCallback(uart1Obj.rxContext);
    }
}

void UART1_RX_InterruptHandler (void)
{
    if(uart1Obj.rxBusyStatus == true)
    {
        /* Clear UART1 RX Interrupt flag */
        IFS1CLR = _IFS1_U1RXIF_MASK;

        while((_U1STA_URXDA_MASK == (U1STA & _U1STA_URXDA_MASK)) && (uart1Obj.rxSize > uart1Obj.rxProcessedSize) )
        {
            if (( U1MODE & (_U1MODE_PDSEL0_MASK | _U1MODE_PDSEL1_MASK)) == (_U1MODE_PDSEL0_MASK | _U1MODE_PDSEL1_MASK))
            {
                /* 9-bit mode */
                ((uint16_t*)uart1Obj.rxBuffer)[uart1Obj.rxProcessedSize++] = (uint16_t )(U1RXREG);
            }
            else
            {
                /* 8-bit mode */
                uart1Obj.rxBuffer[uart1Obj.rxProcessedSize++] = (uint8_t )(U1RXREG);
            }
        }


        /* Check if the buffer is done */
        if(uart1Obj.rxProcessedSize >= uart1Obj.rxSize)
        {
            uart1Obj.rxBusyStatus = false;

            /* Disable the fault interrupt */
            IEC1CLR = _IEC1_U1EIE_MASK;

            /* Disable the receive interrupt */
            IEC1CLR = _IEC1_U1RXIE_MASK;

            if(uart1Obj.rxCallback != NULL)
            {
                uart1Obj.rxCallback(uart1Obj.rxContext);
            }
        }
    }
    else
    {
        // Nothing to process
        ;
    }
}

void UART1_TX_InterruptHandler (void)
{
    if(uart1Obj.txBusyStatus == true)
    {
        /* Clear UART1TX Interrupt flag */
        IFS1CLR = _IFS1_U1TXIF_MASK;

        while((!(U1STA & _U1STA_UTXBF_MASK)) && (uart1Obj.txSize > uart1Obj.txProcessedSize) )
        {
            if (( U1MODE & (_U1MODE_PDSEL0_MASK | _U1MODE_PDSEL1_MASK)) == (_U1MODE_PDSEL0_MASK | _U1MODE_PDSEL1_MASK))
            {
                /* 9-bit mode */
                U1TXREG = ((uint16_t*)uart1Obj.txBuffer)[uart1Obj.txProcessedSize++];
            }
            else
            {
                /* 8-bit mode */
                U1TXREG = uart1Obj.txBuffer[uart1Obj.txProcessedSize++];
            }
        }


        /* Check if the buffer is done */
        if(uart1Obj.txProcessedSize >= uart1Obj.txSize)
        {
            uart1Obj.txBusyStatus = false;

            /* Disable the transmit interrupt, to avoid calling ISR continuously */
            IEC1CLR = _IEC1_U1TXIE_MASK;

            if(uart1Obj.txCallback != NULL)
            {
                uart1Obj.txCallback(uart1Obj.txContext);
            }
        }
    }
    else
    {
        // Nothing to process
        ;
    }
}


