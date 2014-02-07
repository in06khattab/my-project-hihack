/***************************************************************************//**
 * @file
 * @brief AES ECB 128-bit example for EFM32
 * @author Energy Micro AS
 * @version 1.11
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
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
#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_aes.h"
#include "aes_ecb_128.h"

const uint8_t exampleData[] = { 0x01, 0x02, 0x03, 0x04,
                   0x05, 0x06, 0x07, 0x08,
                   0x09, 0x0A, 0x0B, 0x0C,
                   0x0D, 0x0E, 0x0F, 0x00 };

const uint8_t expectedEncryptedData[] = { 0xbf, 0xb1, 0xf9, 0xa1,
														0x98, 0xc0, 0x1c, 0x87,
														0x7d, 0x66, 0x43, 0x30,
														0x8a, 0x26, 0x5a, 0xc4};

const uint8_t exampleKey[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };

uint8_t       dataBuffer[sizeof(exampleData) / sizeof(exampleData[0])];
uint8_t       decryptionKey[16];

/**************************************************************************//**
 * @brief  Main function
 *         The example data is first encrypted and encrypted data is checked.
 *         The encrypted data is then decrypted and checked against original data.
 *         Program ends at last while loop if all is OK.
 *****************************************************************************/
int  main(void)
{
  uint32_t i;

  /* Initialize error indicator */
  bool error = false;

  /* Chip errata */
  CHIP_Init();

  /* Enable AES clock */
  CMU_ClockEnable(cmuClock_AES, true);

  /* Calculate decryption key from original key. Only needs to be done once for each key */
  AES_DecryptKey128(decryptionKey, exampleKey);

  /* Copy plaintext to dataBuffer */
  for (i=0; i<(sizeof(exampleData) / sizeof(exampleData[0])); i++)
  {
    dataBuffer[i] = exampleData[i];
  }

  /* Encrypt data in AES-128 ECB */
  AesEcb128(exampleKey,
            dataBuffer,
            dataBuffer,
            false,
            1);

  /* Wait for AES to finish */
  while(!AesFinished());

  /* Check whether encrypted results are correct */
  for (i = 0; i < (sizeof(dataBuffer) / sizeof(dataBuffer[0])); i++)
  {
    if (dataBuffer[i] != expectedEncryptedData[i])
    {
      error = true;
    }
  }

  /* Decrypt data with AES-128 ECB */
  AesEcb128(decryptionKey,
            dataBuffer,
            dataBuffer,
            true,
            2);

  /* Wait for AES to finish */
  while(!AesFinished());

  /* Check whether decrypted result is identical to the plaintext */
  for (i = 0; i < (sizeof(dataBuffer) / (sizeof(dataBuffer[0])*16)); i++)
  {
    if (dataBuffer[i] != exampleData[i])
    {
      error = true;
    }
  }

  /* Check for success */
  if (error)
  {
    while (1) ; /* Ends here if there has been an error */
  }
  else
  {
    while (1) ; /* Ends here if all OK */
  }
}
