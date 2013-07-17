/**************************************************************************//**
 * @file
 * @brief XMODEM protocol
 * @author Energy Micro AS
 * @version 1.64
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdio.h>
#include "em_device.h"

#include "xmodem.h"
#include "usart.h"
#include "flash.h"
#include "crc.h"
#include "config.h"
#include "encode.h"
#include "decode.h"
#include "board.h"

#define ALIGNMENT(base,align) (((base)+((align)-1))&(~((align)-1)))

/* Packet storage. Double buffered version. */
#if defined (__ICCARM__)
#pragma data_alignment=4
uint8_t rawPacket[2][ALIGNMENT(sizeof(XMODEM_packet),4)];
#elif defined (__CC_ARM)
uint8_t rawPacket[2][ALIGNMENT(sizeof(XMODEM_packet),4)] __attribute__ ((aligned(4)));
#elif defined (__GNUC__)
uint8_t rawPacket[2][ALIGNMENT(sizeof(XMODEM_packet),4)] __attribute__ ((aligned(4)));
#else
#error Undefined toolkit, need to define alignment
#endif

/**************************************************************************//**
 * @brief Verifies checksum, packet numbering and
 * @param pkt The packet to verify
 * @param sequenceNumber The current sequence number.
 * @returns -1 on packet error, 0 otherwise
 *****************************************************************************/
__ramfunc __INLINE int XMODEM_verifyPacketChecksum(XMODEM_packet *pkt, int sequenceNumber)
{
  uint16_t packetCRC;
  uint16_t calculatedCRC;

#if XMODEM==1
  USART_printString("\r\n");
  USART_printHex(pkt->packetNumber);
  USART_printString("  ");
  USART_printHex(sequenceNumber);
  USART_printString("  ");
#endif
  /* Check the packet number integrity */
  if (pkt->packetNumber + pkt->packetNumberC != 255)
  {
#if XMODEM==1
	USART_printString("iErr.\r\n");
#endif
    return -1;
  }

  /* Check that the packet number matches the excpected number */
  if (pkt->packetNumber != (sequenceNumber % 256))
  {
#if XMODEM==1
	USART_printString("pErr.\r\n");
#endif
    return -1;
  }

  calculatedCRC = CRC_calc((uint8_t *) pkt->data, (uint8_t *) &(pkt->crcHigh));
  packetCRC     = pkt->crcHigh << 8 | pkt->crcLow;

  /* Check the CRC value */
  if (calculatedCRC != packetCRC)
  {
#if XMODEM==1
	USART_printString("cErr.\r\n");
#endif
    return -1;
  }
  return 0;
}

/**************************************************************************//**
 * @brief Starts a XMODEM download.
 *
 * @param baseAddress
 *   The address to start writing from
 *
 * @param endAddress
 *   The last address. This is only used for clearing the flash
 *****************************************************************************/
__ramfunc int XMODEM_download(uint32_t baseAddress, uint32_t endAddress, uint8_t ch)
{
  XMODEM_packet *pkt;
  uint32_t      i;
  uint32_t      addr;
  uint32_t      byte;
  uint32_t      sequenceNumber = 1;
  uint8_t		txByte;

  TIMER_Reset(HIJACK_RX_TIMER);
  for (addr = baseAddress; addr < endAddress; addr += flashPageSize)
  {
    FLASH_eraseOneBlock(addr);
  }
  TIMER_setup();

  /* Send one start transmission packet. Wait for a response. If there is no
   * response, we resend the start transmission packet.
   * Note: This is a fairly long delay between retransmissions(~6 s). */
  while (1)
  {
	txByte = XMODEM_NCG;
	if( rxByteChannel & (0x01 << 1) ){
	  HIJACKPutData( &txByte, &encBuf, 1);
	}
	else{
	  USART_txByte(txByte);
	}

    for (i = 0; i < 1000000; i++)
    {
	  if( rxByteChannel & (0x01 << 1) ){
	  	if(decBuf.pendingBytes)
      	{
          goto xmodem_transfer;
      	}
	  }
	  else{
#ifdef BOOTLOADER_LEUART_CLOCK
      	if (BOOTLOADER_USART->STATUS & LEUART_STATUS_RXDATAV)
#else
      	if (BOOTLOADER_USART->STATUS & USART_STATUS_RXDATAV)
#endif
		{
          goto xmodem_transfer;
      	}
	  }
    }
  }
 xmodem_transfer:
  while (1)
  {
    /* Swap buffer for packet buffer */
    pkt = (XMODEM_packet *) rawPacket[sequenceNumber & 1];

    /* Fetch the first byte of the packet explicitly, as it defines the
     * rest of the packet */
	if( rxByteChannel & (0x01 << 1) ){
	  while( !decBuf.pendingBytes );
	  pkt->header = dec_rxByte();
	}
	else{
	  pkt->header = USART_rxByte();
	}

    /* Check for end of transfer */
    if (pkt->header == XMODEM_EOT)
    {
	  txByte = XMODEM_ACK;
      /* Acknowledget End of transfer */
	  if( rxByteChannel & (0x01 << 1) ){
	  	HIJACKPutData( &txByte, &encBuf, 1);
	  }
	  else{
		USART_txByte(txByte);
	  }
      break;
    }

    /* If the header is not a start of header (SOH), then cancel *
     * the transfer. */
    if (pkt->header != XMODEM_SOH)
    {
#if XMODEM==1
	  USART_printString("\r\nhErr\r\n");
#endif
	
	  txByte = XMODEM_NAK;
      /* Acknowledget End of transfer */
	  if( rxByteChannel & (0x01 << 1) ){
	  	HIJACKPutData( &txByte, &encBuf, 1);
	  }
	  else{
		USART_txByte(txByte);
	  }
	  continue;
      //return -1;
    }
#if XMODEM==1
	else{
	  USART_printString("\r\nhOk\r\n");
	}
#endif

    /* Fill the remaining bytes packet */
    /* Byte 0 is padding, byte 1 is header */
    for (byte = 2; byte < sizeof(XMODEM_packet); byte++)
    {
	  if( rxByteChannel & (0x01 << 1) ){
		while( !decBuf.pendingBytes );
	    *(((uint8_t *) pkt) + byte) = dec_rxByte();
	  }
	  else{
	    *(((uint8_t *) pkt) + byte) = USART_rxByte();
	  }
    }

    if (XMODEM_verifyPacketChecksum(pkt, sequenceNumber) != 0)
    {
      /* On a malformed packet, we send a NAK, and start over */
	  txByte = XMODEM_NAK;
      /* Acknowledget End of transfer */
	  if( rxByteChannel & (0x01 << 1) ){
	  	HIJACKPutData( &txByte, &encBuf, 1);
	  }
	  else{
		USART_txByte(txByte);
	  }
      continue;
    }
#if XMODEM==1
	else{
	  USART_printString("\r\nvOk.\r\n");
	}
#endif
	if( rxByteChannel & (0x01 << 1) )
	  TIMER_Reset(HIJACK_RX_TIMER);
	
    /* Write data to flash */
	if( rxByteChannel & (0x01 << 1) )
	  enc_delay_tmr_cnt = 200;	
    FLASH_writeBlock((void *) baseAddress,
                     (sequenceNumber - 1) * XMODEM_DATA_SIZE,
                     XMODEM_DATA_SIZE,
                     (uint8_t const *) pkt->data);

	if( rxByteChannel & (0x01 << 1) )
      TIMER_setup();

    sequenceNumber++;
    /* Send ACK */
	txByte = XMODEM_ACK;
    /* Acknowledget End of transfer */
	if( rxByteChannel & (0x01 << 1) ){
	  HIJACKPutData( &txByte, &encBuf, 1);
	}
	else{
	  USART_txByte(txByte);
	}
  }
  /* Wait for the last DMA transfer to finish. */
  while (DMA->CHENS & DMA_CHENS_CH0ENS) ;
  rxByteChannel = 0;
  return 0;
}
