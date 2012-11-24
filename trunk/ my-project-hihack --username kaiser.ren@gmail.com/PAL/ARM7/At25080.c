/**
 * @file at25080.c
 *
 * @brief at25080 specific functionality
 *
 * This file implements at25080 board specific functionality.
 *
 *
 *  @author
 *  kaiser.ren
 */
/* === Includes ============================================================ */
#include "pal.h"
#include "pal_boardtypes.h"
#include "pal_config.h"
#include "At25080.h"

/**
 * @brief Initializes the transceiver interface
 *
 * This function initializes the transceiver interface.
 * This board uses SPI1.
 */
void At25_interface_init(void)
{
	/*
     * The PIO control is disabled for the SPI pins MISO, MOSI, SCK and the
     * transceiver interrupt pin and the SPI module control for these pins
     * is enabled.
     *
     * Please note that these SPI pins from SPI1 require setting of
     * Peripheral A.
     */
    AT91C_BASE_PIOA->PIO_BSR = (MO | MI | ESLK);
    AT91C_BASE_PIOA->PIO_PDR = (MO | MI | ESLK);

    //AT91C_BASE_PIOA->PIO_ASR = TRX_INTERRUPT_PIN;
    //AT91C_BASE_PIOA->PIO_PDR = TRX_INTERRUPT_PIN;

    /*
     * Select line will be used as a GPIO. The controller recognizes 1 cycle
     * of SPI transaction as 8 bit, hence deactivates the chip select after 1
     * cycle. But the transceiver needs the chip select to be active for two
     * cycles (In one cycle the transceiver gets to know about the address of
     * register or memory location and the kind of operation to be performed
     * on that memory location. And in the second cycle its performs the
     * specified operation). To achieve this, the chip select line will be
     * manually pulled low and high (after the transaction). Hence the SEL line
     * is configured as PIO and the SPI control of SEL is disabled.
     */
    /* Set SEL as output pin. */
    AT91C_BASE_PIOA->PIO_OER = CS;
    AT91C_BASE_PIOA->PIO_PER = CS;

    /*
     * Used peripheral interface is SPI0.
     * The clock to the utilized SPI 0 peripheral is enabled.
     */
    AT91C_BASE_PMC->PMC_PCER = _BV(AT91C_ID_SPI1);

    /* The controller is configured to be master. */
    AT91C_BASE_SPI1->SPI_MR = (AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | (0x0E00));

    /*
     * SPI mode 0 (clock polarity = 0 and clock phase = 1) is selected. The
     * transaction width is set to 8 bits. The SCBR register of the SPI module
     * is written with the divider required for generation of the baud rate. It
     * is calculated as follows. Baud rate = MCK / SPI_BR_DIVIDER.
     */
    AT91C_BASE_SPI1->SPI_CSR[0] = (AT91C_SPI_NCPHA |
                            (AT91C_SPI_BITS & AT91C_SPI_BITS_8) |
                            (AT91C_SPI_SCBR & (SPI_BR_DIVIDER << SCBR_FIELD_POS_IN_CSR_REG)));

    /* The SPI is enabled. */
    AT91C_BASE_SPI1->SPI_CR = AT91C_SPI_SPIEN;
}

/**
 * @brief Initializes the transceiver interface
 *
 * This function initializes the transceiver interface.
 * This board uses SPI1.
 */
void At25_interface_uninit(void)
{
  	 AT91C_BASE_PIOA->PIO_OER = (MO | ESLK);
	 AT91C_BASE_PIOA->PIO_IFER = (MI);
    AT91C_BASE_PIOA->PIO_PER = (MO | MI | ESLK);
	 AT91C_BASE_PIOA->PIO_SODR = (MO |ESLK);

    /* Set SEL as output pin. */
    AT91C_BASE_PIOA->PIO_OER = CS;
    AT91C_BASE_PIOA->PIO_PER = CS;
	 AT91C_BASE_PIOA->PIO_SODR = CS;
	
	 /* Clear SPI_MR. */
	 AT91C_BASE_SPI1->SPI_MR = 0;
	 AT91C_BASE_SPI1->SPI_CSR[0] = 0;

    /* The SPI is disabled. */
    AT91C_BASE_SPI1->SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
}

uint8_t At25_read_data(uint16_t addr)
{
  	uint8_t register_value;
	
  	ENTER_CRITICAL_REGION();

    /* Start SPI transaction by pulling SEL low */
    EE_LOW();

	 /* write DATA READ COMMAND */
	 EESPI_WRITE(EE_DATA_READ);
	
    /* Send the Read command byte */
    EESPI_WRITE((uint8_t)(addr >> 8));
	
	 EESPI_WRITE((uint8_t)(addr));

    /* Write the byte in the transceiver data register */
    EESPI_WRITE(SPI_DUMMY_VALUE);
	
	 /* Read the byte received */
    EESPI_READ(register_value);

    /* Stop the SPI transaction by setting SEL high */
    EE_HIGH();

    LEAVE_CRITICAL_REGION();
	
	 return register_value;
}


void at25_reg_write(uint8_t addr, uint8_t data)
{
    ENTER_CRITICAL_REGION();

    /* Start SPI transaction by pulling SEL low */
    EE_LOW();

    /* Send the Read command byte */
    EESPI_WRITE(addr);

    /* Write the byte in the transceiver data register */
    EESPI_WRITE(data);

    /* Stop the SPI transaction by setting SEL high */
    EE_HIGH();

    LEAVE_CRITICAL_REGION();
}

uint8_t at25_reg_read(uint8_t addr)
{
    uint8_t register_value;

    ENTER_CRITICAL_REGION();

    /* Start SPI transaction by pulling SEL low */
    EE_LOW();

    /* Send the write command byte */
    EESPI_WRITE(addr);

    /*
     * Done to clear the RDRF bit in the SPI status register, which will be set
     * as a result of reception of some data from the transceiver as a result
     * of SPI write operation done above.
     */
    EESPI_READ(register_value);

    /* Do dummy write for initiating SPI read */
    EESPI_WRITE(SPI_DUMMY_VALUE);

    /* Read the byte received */
    EESPI_READ(register_value);

    /* Stop the SPI transaction by setting SEL high */
    EE_HIGH();

    LEAVE_CRITICAL_REGION();

    return register_value;
}
//EOF

