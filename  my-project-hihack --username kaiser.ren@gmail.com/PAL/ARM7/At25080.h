/**
 * @file at25080.h
 *
 * @brief PAL configuration for at25080
 *
 * This header file contains configuration parameters for AT91SAM7X256.
 */
/**
 * @author
 * kaiser.ren
 */

/* Prevent double inclusion */
#ifndef AT25080_H
#define AT25080_H
/* === Includes ==============================================================*/

#include "AT91SAM7X256.h"
#include "pal_boardtypes.h"


/*
 * This board uses an SPI-attached transceiver.
 */
/*
 * SPI Base Register:
 * SPI1 is used with AT25080
 */
#define CS				(AT91C_PA21_SPI1_NPCS0)
#define MO				(AT91C_PA23_SPI1_MOSI)
#define MI				(AT91C_PA24_SPI1_MISO)
#define ESLK			(AT91C_PA22_SPI1_SPCK)

#define AT91C_BASE_E2PROM_USE				 (AT91C_BASE_SPI1)

/*
 * SPI1 is used with AT25080 for address map
 */
#define EE_WREN_WRITE		0X06

#define EE_WRDI_WRITE		0X04

#define EE_STATUS_READ		0x05

#define EE_STATUS_WRITE		0X01

#define EE_DATA_READ			0X03

#define EE_DATA_WRITE		0x02

/*
 * Dummy value to be written in SPI transmit register to retrieve data form it
 */
#define SPI_DUMMY_VALUE                 (0x00)

/*
 * Slave select made low of SPI1
 */
#define EE_LOW()                        {AT91C_BASE_PIOA->PIO_CODR = CS;}

/*
 * Slave select made high
 */
#define EE_HIGH()                       {AT91C_BASE_PIOA->PIO_SODR = CS;}

/* Reads the data from the SPI1 receive register. */
#define EESPI_READ(register_value)    do {                                \
    while ((AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_RDRF) == 0);        \
    register_value = (AT91C_BASE_SPI1->SPI_RDR & 0xFFFF);           \
} while (0);

/* Writes the data into the SPI1 transmit register. */
#define EESPI_WRITE(data)     do {                                        \
    while ((AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_TDRE) == 0);        \
    AT91C_BASE_SPI1->SPI_TDR = data & 0xFFFF;                       \
    while ((AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_TXEMPTY) == 0);     \
} while (0);

/* === PROTOTYPES ========================================================== */
void At25_interface_init(void);
void At25_interface_uninit(void);
uint8_t At25_read_status(void);
uint8_t At25_read_data(uint16_t addr);
void at25_reg_write(uint8_t addr, uint8_t data);
uint8_t at25_reg_read(uint8_t addr);

#endif

