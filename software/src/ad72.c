/**
 * Battery management system, Walking Machine, Ecole de Technologie Superieure
 * Copyright (C) 2013 Francois Killeen
 * AD72
 */

// C Standard
#include <stdint.h>
// ChibiOS
#include "hal.h"
#include "ch.h"
// Local
#include "ad72.h"

#define CRC_DATA_WRITE 21
#define CRC_DATA_READ 22

/**
 * AD7280A checksum Computed on Bits[D31:D11] for ad72 write operation,
 * and Bits[D31:D10] for ad72 read operation.
 * @param packet The adress of the packet in which the data is located
 * @param crc The type of CRC (WRITE_REGISTER,READ_REGISTER,READ_CONVERSION)
 * @return 8 Bit CRC
 */
uint8_t do_crc8(ad7280a_packet_t *packet, crc_type_t crc) {
  uint8_t xor_1=0,xor_2=0,xor_3=0,xor_4=0,xor_5=0;
  uint8_t CRC_0=0,CRC_1=0,CRC_2=0,CRC_3=0,CRC_4=0,CRC_5=0,CRC_6=0,CRC_7=0;
  uint8_t num_bits=0,crc_out=0;
  int i;
  uint32_t data_in;
  // Data in is relative to the type of CRC
  switch(crc) {
    case WRITE_REGISTER:
      num_bits = CRC_DATA_WRITE;
      data_in = packet->crc_w_data.data;
      break;
    case READ_CONVERSION:
      num_bits = CRC_DATA_READ;
      data_in = packet->crc_r_data.data;
      break;
    case READ_REGISTER:
      num_bits = CRC_DATA_READ;
      data_in = packet->crc_r_data.data;
      break;
  }
  // Main CRC execution
  for (i=num_bits; i>=0; i--) {
    xor_5 = CRC_4 ^ CRC_7;
    xor_4 = CRC_2 ^ CRC_7;
    xor_3 = CRC_1 ^ CRC_7;
    xor_2 = CRC_0 ^ CRC_7;
    xor_1 = ((data_in & (1 << i)) >> i) ^ CRC_7;
    CRC_7 = CRC_6;
    CRC_6 = CRC_5;
    CRC_5 = xor_5;
    CRC_4 = CRC_3;
    CRC_3 = xor_4;
    CRC_2 = xor_3;
    CRC_1 = xor_2;
    CRC_0 = xor_1;
    }
  // Packing up the CRC
  crc_out = CRC_0;
  crc_out |= (CRC_1 << 1);
  crc_out |= (CRC_2 << 2);
  crc_out |= (CRC_3 << 3);
  crc_out |= (CRC_4 << 4);
  crc_out |= (CRC_5 << 5);
  crc_out |= (CRC_6 << 6);
  crc_out |= (CRC_7 << 7);
  return crc_out;
}

/**
 * Verifies that the received CRC is correct
 * @param ad72 Addres pointing to the AD7280A device
 * @return 1 if CRC is good 0 if not
 */
uint8_t crc_conv_check(ad7280a_t *ad72) {
  crc_type_t crc_type = CRC_DATA_READ;
  ad7280a_packet_t packet;
  packet.packed = ad72->rxbuf;
  uint8_t received_crc = packet.r_conversion.crc;

  if(received_crc != do_crc8(&packet,crc_type)) {
    return 0;
  }

  return 1;
}

// Low speed SPI configuration BR[2..0] = 111 (Lowest speed)
// CPHA=1, CPOL=0, MSb first, 8 bits Data Frame
static const SPIConfig ls_spicfg = {
  NULL,   // No callback
  GPIOC,  // CS Port ID
  SPI1_CS_PIN,  // CS Pin  ID

  // SPI Speed configurations (Lowest speed)
  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0 |

  // SPI Phase configuration
  SPI_CR1_CPHA |

  // SPI Data frame configuration (16 bits)
  SPI_CR1_DFF
};

uint32_t spi_exchange(ad7280a_t *ad72) {
  uint8_t i;

  ad72->rxbuf = 0;

  uint16_t rx_split_buf[2] = {0,0};
  uint16_t tx_split_buf[2] = {0,0};

  tx_split_buf[0] = ad72->txbuf >> 16;
  tx_split_buf[1] = ad72->txbuf;



  // Spi Exchange
  spiAcquireBus(&SPID1);                             /* Acquire ownership of the bus.    */
  spiSelect(&SPID1);                                 /* Slave Select assertion.          */
  spiExchange(&SPID1, 2, tx_split_buf, rx_split_buf);/* Atomic transfer operations.      */
  spiUnselect(&SPID1);                               /* Slave Select de-assertion.       */
  spiReleaseBus(&SPID1);                             /* Ownership release.               */

  // Short sleep to allow proper data transfer
  chThdSleepMilliseconds(ad72->delay_ms);

  // Merge the four bytes received
  ad72->rxbuf = rx_split_buf[1];
  ad72->rxbuf = ad72->rxbuf << 16;
  ad72->rxbuf |= rx_split_buf[0];

  return ad72->rxbuf;
}

// Power up the ad7280a by putting the powerdown pin to high
void power_up_ad7280a(ad7280a_t *ad72) {
  palSetPad(GPIOB,1);
  ad72->on_off = 1;
}

// Power down the ad7280a by putting the powerdown pin to low
void power_down_ad7280a(ad7280a_t *ad72) {
  palClearPad(GPIOB,1);
  ad72->on_off = 0;
}

// Initialize the ad7280
uint8_t init_ad7280a(ad7280a_t *ad72) {

  ad72->delay_ms = 10;
  ad72->txbuf = 0;
  ad72->rxbuf = 0;
  ad72->cellbalance = 0;
  power_up_ad7280a(ad72);

  spiStart(&SPID1, &ls_spicfg);                      /* Setup transfer parameters.       */

  // set Bit D2 and Bit D0 of the control register to 1
  // and set Bit D1 of the control
  (ad72->txbuf) = 0x01C2B6E2;
  spi_exchange(ad72);

  // Write the register address corresponding to the lower byte
  // of the control register to the read register on all parts.
  (ad72->txbuf) = 0x038716CA;
  spi_exchange(ad72);

  // Apply ad72 CS low pulse that frames 32 SCLKs
  // (This is used to verify that the ad7280a has received and
  // locked his unique address)
  (ad72->txbuf) = 0xF800030A;
  spi_exchange(ad72);

  return 1;
}

// Write to ad72 specific register
void bus_write(ad7280a_t *ad72, uint8_t reg, uint32_t data, uint32_t write_all) {
  ad7280a_packet_t packet;
  crc_type_t crc = WRITE_REGISTER;
  // Pack the data in the appropriate bitfields
  packet.w_register.bit_pattern = 2;
  packet.w_register.reserved = 0;
  packet.w_register.write_all = write_all;
  packet.w_register.register_address = reg;
  packet.w_register.data = data;
  packet.w_register.device_address = 0;
  packet.w_register.crc = do_crc8(&packet,crc);
  (ad72->txbuf) = packet.packed;
  spi_exchange(ad72);
}

// Read cell voltage (choose from 1 to 6)
uint32_t ad7280a_read_cell(uint8_t cell,ad7280a_t *ad72) {
  ad7280a_packet_t packet;
  packet.packed = 0;

  // 1.On ecrit le numero de registre de la cellule
  bus_write(ad72, AD7280A_READ, ((cell-1) << 2) , 0);

  // 2. Turn off the read operation
  bus_write(ad72, AD7280A_CONTROL, AD7280A_CONTROL_CONV_INPUT_6CELL_6ADC
            | AD7280A_CONTROL_CONV_INPUT_READ_DISABLE, 1);

  // 3.Control Register Settings
  bus_write(ad72, AD7280A_CONTROL, AD7280A_CONTROL_CONV_INPUT_6CELL_6ADC
            | AD7280A_CONTROL_CONV_INPUT_READ_6VOLT_6ADC
            | AD7280A_CONTROL_CONV_START_FORMAT_CNVST
            | AD7280A_CONTROL_CONV_AVG_SINGLE, 0);

  // 4.Program the CNVST control register to 0x02 to allow ad72 single pulse
  bus_write(ad72, AD7280A_CNVST_CONTROL, AD7280A_CNVST_CTRL_SINGLE, 0);

  // 5.Initiate conversions through the falling edge of CNVST.
  palClearPad(GPIOB,0);

  // 5.1 Allow sufficient time for all conversions to be completed
  chThdSleepMilliseconds(ad72->delay_ms);

  // 5.2 Latch the CNVST back to one
  palSetPad(GPIOB,0);

  // 6 Gate the CNVST, this prevents unintentional conversions
  bus_write(ad72, AD7280A_CNVST_CONTROL, AD7280A_CNVST_CTRL_GATED, 0);

  //7. Apply 32 SCLKS to have the data in the receive buffer
  (ad72->txbuf) = AD7280A_RETRANSMIT_SCLKS; // (NODATA)
  spi_exchange(ad72);
  packet.packed = (ad72->rxbuf);

  return packet.r_conversion.conversion_data;
}

// Read ad72 Single Configuration Register
uint32_t ad7280a_read_register(uint8_t address,ad7280a_t *ad72) {

  ad7280a_packet_t packet;
  packet.packed = 0;

  // 1. Turn off the read operation
  bus_write(ad72, AD7280A_CONTROL, AD7280A_CONTROL_CONV_INPUT_6CELL_6ADC
            | AD7280A_CONTROL_CONV_INPUT_READ_DISABLE, 1);

  // 2. Turn on the read operation on the addressed part
  bus_write(ad72, AD7280A_CONTROL, AD7280A_CONTROL_CONV_INPUT_6CELL_6ADC
            | AD7280A_CONTROL_CONV_INPUT_READ_6VOLT_6ADC, 0);

  // 3. Write the register address to the read register
  bus_write(ad72, AD7280A_READ, address << 2 , 0);

  // 4. On applique 32 SCLKs pour avoir la lecture dans le rxbuf
  (ad72->txbuf) = AD7280A_RETRANSMIT_SCLKS; // (aucun data/registres)
  spi_exchange(ad72);
  packet.packed = (ad72->rxbuf);

  return packet.r_register.data;
}

// Read thermistor (choose therm from 1 to 2)
uint32_t ad7280a_read_therm(uint8_t therm,ad7280a_t *ad72) {
  return ad7280a_read_cell((therm+6),ad72);
}

// Activate cell balance (choose from 1 to 6)
void ad7280a_balance_cell_on(uint8_t cell, ad7280a_t *ad72) {
  ad72->cellbalance |= (1<<(cell+1));
  bus_write(ad72, AD7280A_CELL_BALANCE, ad72->cellbalance , 0);
}

// Deactivate cell balance (choose from 1 to 6)
void ad7280a_balance_cell_off(uint8_t cell, ad7280a_t *ad72) {
  ad72->cellbalance &= ~(1<<(cell+1));
  bus_write(ad72, AD7280A_CELL_BALANCE, ad72->cellbalance, 0);
}
