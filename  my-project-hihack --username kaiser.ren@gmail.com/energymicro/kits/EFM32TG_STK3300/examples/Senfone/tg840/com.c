/**************************************************************************//**
 * @file com.h
 * @brief Hijack demo for EFM32TG_STK3300
 * Author: kaiser
 *****************************************************************************/

/* Driver header file(s). */
#include "com.h"

/*******************************************************************************
 *******************************   MACROS   ************************************
 ******************************************************************************/


/* Define termination character */
#define TERMINATION_CHAR    '.'

/* Declare a circular buffer structure to use for Rx and Tx queues */
#define BUFFERSIZE          256
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/
volatile struct circularBuffer
{
  uint8_t  data[BUFFERSIZE];  /* data buffer */
  uint32_t rdI;               /* read index */
  uint32_t wrI;               /* write index */
  uint32_t pendingBytes;      /* count of how many bytes are not yet handled */
  bool     overflow;          /* buffer overflow indicator */
} rxBuf, txBuf = { {0}, 0, 0, 0, false };

/*******************************************************************************
 ******************************   CONSTANTS   **********************************
 ******************************************************************************/
/* Declare some strings */
const char     welcomeString[]  = "Energy Micro RS-232 - Please press a key\n";
const char     overflowString[] = "\r\n---RX OVERFLOW---\r\n";
const uint32_t welLen           = sizeof(welcomeString) - 1;
const uint32_t ofsLen           = sizeof(overflowString) - 1;
uint8_t 		appPrintBuf[50];
/*******************************************************************************
 *******************************   STATICS   ***********************************
 ******************************************************************************/

/* Setup USART1 in async mode for RS232*/
static USART_TypeDef           * uart   = USART1;
static USART_InitAsync_TypeDef uartInit = USART_INITASYNC_DEFAULT;


/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/


/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS   ******************************
 ******************************************************************************/


/***************************************************************************//**
 * @brief
 *   Initialize com port USART1.
 ******************************************************************************/
void COM_Init(void)
{
  /* Enable clock for USART module */
  CMU_ClockEnable(cmuClock_USART1, true);

  /* Enable clock for GPIO module (required for pin configuration) */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Configure GPIO pins */
  GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 1);
  GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);


  /* Prepare struct for initializing UART in asynchronous mode*/
  uartInit.enable       = usartDisable;   /* Don't enable UART upon intialization */
  uartInit.refFreq      = 0;              /* Provide information on reference frequency. When set to 0, the reference frequency is */
  uartInit.baudrate     = 38400;         /* Baud rate */
  uartInit.oversampling = usartOVS16;     /* Oversampling. Range is 4x, 6x, 8x or 16x */
  uartInit.databits     = usartDatabits8; /* Number of data bits. Range is 4 to 10 */
  uartInit.parity       = usartNoParity;  /* Parity mode */
  uartInit.stopbits     = usartStopbits1; /* Number of stop bits. Range is 0 to 2 */
  uartInit.mvdis        = false;          /* Disable majority voting */
  uartInit.prsRxEnable  = false;          /* Enable USART Rx via Peripheral Reflex System */
  uartInit.prsRxCh      = usartPrsRxCh0;  /* Select PRS channel if enabled */

  /* Initialize USART with uartInit struct */
  USART_InitAsync(uart, &uartInit);

  /* Prepare UART Rx and Tx interrupts */
  USART_IntClear(uart, _USART_IF_MASK);
  USART_IntEnable(uart, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(USART1_RX_IRQn);
  NVIC_ClearPendingIRQ(USART1_TX_IRQn);
  NVIC_EnableIRQ(USART1_RX_IRQn);
  NVIC_EnableIRQ(USART1_TX_IRQn);

  /* Enable I/O pins at USART1 location #1 */
  uart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | USART_ROUTE_LOCATION_LOC1;

  /* Enable UART */
  USART_Enable(uart, usartEnable);
}

/******************************************************************************
 * @brief  uartGetChar function
 *
 *  Note that if there are no pending characters in the receive buffer, this
 *  function will hang until a character is received.
 *
 *****************************************************************************/
uint8_t uartGetChar( )
{
  uint8_t ch;

  /* Check if there is a byte that is ready to be fetched. If no byte is ready, wait for incoming data */
  if (rxBuf.pendingBytes < 1)
  {
    while (rxBuf.pendingBytes < 1) ;
  }

  /* Copy data from buffer */
  ch        = rxBuf.data[rxBuf.rdI];
  rxBuf.rdI = (rxBuf.rdI + 1) % BUFFERSIZE;

  /* Decrement pending byte counter */
  rxBuf.pendingBytes--;

  return ch;
}

/******************************************************************************
 * @brief  uartPutChar function
 *
 *****************************************************************************/
void uartPutChar(uint8_t ch)
{
  /* Check if Tx queue has room for new data */
  if ((txBuf.pendingBytes + 1) > BUFFERSIZE)
  {
    /* Wait until there is room in queue */
    while ((txBuf.pendingBytes + 1) > BUFFERSIZE) ;
  }

  /* Copy ch into txBuffer */
  txBuf.data[txBuf.wrI] = ch;
  txBuf.wrI             = (txBuf.wrI + 1) % BUFFERSIZE;

  /* Increment pending byte counter */
  txBuf.pendingBytes++;

  /* Enable interrupt on USART TX Buffer*/
  USART_IntEnable(uart, USART_IF_TXBL);
}

/******************************************************************************
 * @brief  uartPutData function
 *
 *****************************************************************************/
void uartPutData(uint8_t * dataPtr, uint32_t dataLen)
{
  uint32_t i = 0;

  /* Check if buffer is large enough for data */
  if (dataLen > BUFFERSIZE)
  {
    /* Buffer can never fit the requested amount of data */
    return;
  }

  /* Check if buffer has room for new data */
  if ((txBuf.pendingBytes + dataLen) > BUFFERSIZE)
  {
    /* Wait until room */
    while ((txBuf.pendingBytes + dataLen) > BUFFERSIZE) ;
  }

  /* Fill dataPtr[0:dataLen-1] into txBuffer */
  while (i < dataLen)
  {
    txBuf.data[txBuf.wrI] = *(dataPtr + i);
    txBuf.wrI             = (txBuf.wrI + 1) % BUFFERSIZE;
    i++;
  }

  /* Increment pending byte counter */
  txBuf.pendingBytes += dataLen;

  /* Enable interrupt on USART TX Buffer*/
  USART_IntEnable(uart, USART_IF_TXBL);
}

/******************************************************************************
 * @brief  uartGetData function
 *
 *****************************************************************************/
uint32_t uartGetData(uint8_t * dataPtr, uint32_t dataLen)
{
  uint32_t i = 0;

  /* Wait until the requested number of bytes are available */
  if (rxBuf.pendingBytes < dataLen)
  {
    while (rxBuf.pendingBytes < dataLen) ;
  }

  if (dataLen == 0)
  {
    dataLen = rxBuf.pendingBytes;
  }

  /* Copy data from Rx buffer to dataPtr */
  while (i < dataLen)
  {
    *(dataPtr + i) = rxBuf.data[rxBuf.rdI];
    rxBuf.rdI      = (rxBuf.rdI + 1) % BUFFERSIZE;
    i++;
  }

  /* Decrement pending byte counter */
  rxBuf.pendingBytes -= dataLen;

  return i;
}

/**************************************************************************//**
 * @brief USART1 RX IRQ Handler
 *
 * Set up the interrupt prior to use
 *
 * Note that this function handles overflows in a very simple way.
 *
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  /* Check for RX data valid interrupt */
  if (uart->STATUS & USART_STATUS_RXDATAV)
  {
    /* Copy data into RX Buffer */
    uint8_t rxData = USART_Rx(uart);
    rxBuf.data[rxBuf.wrI] = rxData;
    rxBuf.wrI             = (rxBuf.wrI + 1) % BUFFERSIZE;
    rxBuf.pendingBytes++;

    /* Flag Rx overflow */
    if (rxBuf.pendingBytes > BUFFERSIZE)
    {
      rxBuf.overflow = true;
    }

    /* Clear RXDATAV interrupt */
    USART_IntClear(USART1, USART_IF_RXDATAV);
  }
}

/**************************************************************************//**
 * @brief USART1 TX IRQ Handler
 *
 * Set up the interrupt prior to use
 *
 *****************************************************************************/
void USART1_TX_IRQHandler(void)
{
  /* Clear interrupt flags by reading them. */
  USART_IntGet(USART1);

  /* Check TX buffer level status */
  if (uart->STATUS & USART_STATUS_TXBL)
  {
    if (txBuf.pendingBytes > 0)
    {
      /* Transmit pending character */
      USART_Tx(uart, txBuf.data[txBuf.rdI]);
      txBuf.rdI = (txBuf.rdI + 1) % BUFFERSIZE;
      txBuf.pendingBytes--;
    }

    /* Disable Tx interrupt if no more bytes in queue */
    if (txBuf.pendingBytes == 0)
    {
      USART_IntDisable(uart, USART_IF_TXBL);
    }
  }
}

void Debug_Print(const char *format, ...)
{
  char *s = (char *)appPrintBuf;
  char nr_of_chars;
  int ans;
  va_list ap;
  _SProutInfo x;

  va_start(ap, format);

  x.s = s;

  ans = _Printf(&_SProut, &x, format, &ap);
        *x.s = '\0';
  va_end(ap);
	
  nr_of_chars = (ans < 0 ? ans : (x.s - s));

  if (nr_of_chars > 0) {
	for(uint8_t i = 0; i < nr_of_chars; i++)
    	uartPutChar(appPrintBuf[i] );
  }
}

