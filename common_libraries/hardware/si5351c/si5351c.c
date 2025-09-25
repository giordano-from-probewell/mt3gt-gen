#include "F28x_Project.h"
#include "si5351c.h"
#include "si5351c_registers.h"
#include "my_i2c.h"


int32_t ref_correction = 0;
uint32_t plla_freq = 0;
uint32_t pllb_freq = 0;

struct Si5351Status dev_status;
struct Si5351IntStatus dev_int_status;


si5351c_status_t si5351c_write_register(uint8_t reg, uint8_t value)
{
    DELAY_US(200);
    if (I2CA_WriteToReg(SI5351C_ADDR, reg, (unsigned char *)&value, 1))
    {
        return SI5351C_STATUS_WRITE_ERROR;
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c_read_register(uint8_t reg, uint8_t * value)
{
    DELAY_US(200);
    if (I2CA_ReadFromReg(SI5351C_ADDR, reg, (unsigned char *)value, 1))
    {
        return SI5351C_STATUS_READ_ERROR;
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351_write_sequence(uint8_t addr, uint8_t *buffer, uint8_t length)
{
    uint8_t i = 0;

    for (i=0;i<length;i++)
    {
        DELAY_US(1000);
        if(si5351c_write_register(addr++, buffer[i]))
        {
            return SI5351C_STATUS_SEQ_WRITE_ERROR;
        }
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c_init(void)
{
    if(si5351c_stop_gen())                                  // STEP 1: Disable outputs
        return SI5351C_STATUS_INIT_ERROR;

    if(si5351c_interrupts_off())                            // STEP 2: Power-down all output drivers
        return SI5351C_STATUS_INIT_ERROR;

    //    if(si5351c_write_register(0x02, 0x50))                  // STEP 3: Set interrput masks
    //        return SI5351C_STATUS_INIT_ERROR;

    if(si5351c_write_initial_config())                      // STEP 4: Write initial config to registers
        return SI5351C_STATUS_INIT_ERROR;

    //    if(si5351c_write_register(0x00B1, 0xAC))              // STEP 5: Apply PLLA and PLLB soft reset
    //        return SI5351C_STATUS_INIT_ERROR;


    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c_change_clk_phase(uint8_t value, enum si5351_clock clk)
{

    //    if(si5351c_write_register( (uint8_t) (0xA5+clk), (value&0x7F)) )
    //        return SI5351C_STATUS_CONFIG_ERROR;

    return SI5351C_STATUS_OK;
}


//si5351c_status_t si5351c_write_initial_config(void)
//{
//    //Without pll
//   uint8_t conf[][3] = {
//            { 0x60, 0x000F, 0x00 },     // REG 15 - 7:6 - CLKIN_DIV[1:0] 3 - PLLB_SRC 2 - PLLA_SRC
//
//            { 0x60, 0x0018, 0x55},      // REG 24 - CLK0-3 in HIGH when disabled
//            { 0x60, 0x0019, 0x55},      // REG 25 - CLK4-7 in HIGH when disabled
//
//            { 0x60, 0x001A, 0x00 },     // REG 26 - 7:0 - MSNA_P3[15:8]
//            { 0x60, 0x001B, 0x19 },     // REG 27 - 7:0 - MSNA_P3[7:0]
//            { 0x60, 0x001C, 0x00 },     // REG 28 - 1:0 - MSNA_P1[17:16]
//            { 0x60, 0x001D, 0x0F },     // REG 29 - 7:0 - MSNA_P1[15:8]
//            { 0x60, 0x001E, 0xD1 },     // REG 30 - 7:0 - MSNA_P1[7:0]
//            { 0x60, 0x001F, 0x00 },     // REG 31 - 7:4 - MSNA_P3[19:16] 3:0 - MSNA_P2[19:16]
//            { 0x60, 0x0020, 0x00 },     // REG 32 - 7:0 - MSNA_P2[15:8]
//            { 0x60, 0x0021, 0x17 },     // REG 33 - 7:0 - MSNA_P2[7:0]
//            { 0x60, 0x002A, 0x00 },     // REG 42 - 7:0 - MS0_P3[15:8]
//            { 0x60, 0x002B, 0x01 },     // REG 43 - 7:0 - MS0_P3[7:0]
//            { 0x60, 0x002C, 0x01 },     // REG 44 - 6:4 - R0_DIV[2:0] 3:2 - MS0_DIVBY4[1:0] 1:0 - MS0_P1[17:16]
//            { 0x60, 0x002D, 0xBB },     // REG 45 - 7:0 - MS0_P1[15:8]
//            { 0x60, 0x002E, 0x80 },     // REG 46 - 7:0 - MS0_P1[7:0]
//            { 0x60, 0x002F, 0x00 },     // REG 47 - 7:4 - MS0_P3[19:16] 3:0 - MS0_P2[19:16]
//            { 0x60, 0x0030, 0x00 },     // REG 48 - 7:0 - MS0_P2[15:8]
//            { 0x60, 0x0031, 0x00 },     // REG 49 - 7:0 - MS0_P2[7:0]
//            { 0x60, 0x0032, 0x00 },     // REG 50 - 7:0 - MS1_P3[15:8]
//            { 0x60, 0x0033, 0x02 },     // REG 51 - 7:0 - MS1_P3[7:0]
//            { 0x60, 0x0034, 0x00 },     // REG 52 - 6:4 - R1_DIV[2:0] 3:2 - MS1_DIVBY4[1:0] 1:0 - MS1_P1[17:16]
//            { 0x60, 0x0035, 0xDC },     // REG 53 - 7:0 - MS1_P1[15:8]
//            { 0x60, 0x0036, 0xC0 },     // REG 54 - 7:0 - MS1_P1[7:0]
//            { 0x60, 0x0037, 0x00 },     // REG 55 - 7:4 - MS1_P3[19:16] 3:0 - MS1_P2[19:16]
//            { 0x60, 0x0038, 0x00 },     // REG 56 - 7:0 - MS1_P2[15:8]
//            { 0x60, 0x0039, 0x00 },     // REG 57 - 7:0 - MS1_P2[7:0]
//            { 0x60, 0x003A, 0x00 },     // REG 58 - 7:0 - MS2_P3[15:8]
//            { 0x60, 0x003B, 0x01 },     // REG 59 - 7:0 - MS2_P3[7:0]
//            { 0x60, 0x003C, 0x00 },     // REG 60 - 6:4 - R2_DIV[2:0] 3:2 - MS2_DIVBY4[1:0] 1:0 - MS2_P1[17:16]
//            { 0x60, 0x003D, 0x0E },     // REG 61 - 7:0 - MS2_P1[15:8]
//            { 0x60, 0x003E, 0x80 },     // REG 62 - 7:0 - MS2_P1[7:0]
//            { 0x60, 0x003F, 0x00 },     // REG 63 - 7:4 - MS2_P3[19:16] 3:0 - MS2_P2[19:16]
//            { 0x60, 0x0040, 0x00 },     // REG 64 - 7:0 - MS2_P2[15:8]
//            { 0x60, 0x0041, 0x00 },     // REG 65 - 7:0 - MS2_P2[7:0]
//            { 0x60, 0x005A, 0x00 },     // REG 90 - 7:0 - MS6_P1[7:0]
//            { 0x60, 0x005B, 0x00 },     // REG 91 - 7:0 - MS7_P1[7:0]
//            { 0x60, 0x0095, 0x00 },     // REG 149 - 7 - SSC_EN 6:0 - SSDN_P2[14:8]
//            { 0x60, 0x0096, 0x00 },     // REG 150 - 7:0 - SSDN_P2[7:0]
//            { 0x60, 0x0097, 0x00 },     // REG 151 - 7 - SSC_MODE 6:0 - SSDN_P3[14:8]
//            { 0x60, 0x0098, 0x00 },     // REG 152 - 7:0 - SSDN_P3[7:0]
//            { 0x60, 0x0099, 0x00 },     // REG 153 - 7:0 - SSDN_P1[7:0]
//            { 0x60, 0x009A, 0x00 },     // REG 154 - 7:4 - SSUDP[11:8] 3:0 - SSDN_P1[11:8]
//            { 0x60, 0x009B, 0x00 },     // REG 155 - 7:0 - SSUDP[7:0]
//            { 0x60, 0x00A2, 0x00 },     // REG 162 - 7:0 - VCXO_Param[7:0]
//            { 0x60, 0x00A3, 0x00 },     // REG 163 - 7:0 - VCXO_Param[15:8]
//            { 0x60, 0x00A4, 0x00 },     // REG 164 - 5:0 - VCXO_Param[21:16]
//            { 0x60, 0x00A5, 0x00 },     // REG 165 - 6:0 - CLK0_PHOFF[6:0]
//            { 0x60, 0x00A6, 0x00 },     // REG 166 - 6:0 - CLK1_PHOFF[6:0]
//            { 0x60, 0x00A7, 0x00 },     // REG 167 - 6:0 - CLK2_PHOFF[6:0]
//            { 0x60, 0x00B7, 0x92 },     // REG 183 - 7:6 - XTAL_CL[1:0]
//
//            { 0x60, 0x00B1, 0xAC },     // REG 177 - 7 - PLLB_RST 5 - PLLA_RST
//
//            { 0x60, 0x0010, 0x0F },     // REG 16 - CLK0 Powered Up
//            { 0x60, 0x0011, 0x0F },     // REG 17 - CLK1 Powered Up
//            { 0x60, 0x0012, 0x0F },     // REG 18 - CLK2 Powered Up
//            { 0x60, 0x0013, 0x8C },     // REG 19 - CLK3 Powered Down
//            { 0x60, 0x0014, 0x8C },     // REG 20 - CLK4 Powered Down
//            { 0x60, 0x0015, 0x8C },     // REG 21 - CLK5 Powered Down
//            { 0x60, 0x0016, 0x8C },     // REG 22 - CLK6 Powered Down
//            { 0x60, 0x0017, 0x8C },     // REG 23 - CLK7 Powered Down
//        };
//
//test PLL vilson
//        uint8_t conf[][3] = {
//
//                 { 0x60, 0x0002, 0x4B },
//                 { 0x60, 0x0003, 0x00 },
//                 { 0x60, 0x0004, 0x20 },
//                 { 0x60, 0x0007, 0x01 },
//                 { 0x60, 0x000F, 0x04 },
//                 { 0x60, 0x0010, 0x8C },
//                 { 0x60, 0x0011, 0x8C },
//                 { 0x60, 0x0012, 0x0F },
//                 { 0x60, 0x0013, 0x8C },
//                 { 0x60, 0x0014, 0x8C },
//                 { 0x60, 0x0015, 0x8C },
//                 { 0x60, 0x0016, 0x8C },
//                 { 0x60, 0x0017, 0x8C },
//                 { 0x60, 0x001A, 0x3D },
//                 { 0x60, 0x001B, 0x09 },
//                 { 0x60, 0x001C, 0x00 },
//                 { 0x60, 0x001D, 0x2A },
//                 { 0x60, 0x001E, 0xD3 },
//                 { 0x60, 0x001F, 0x00 },
//                 { 0x60, 0x0020, 0x25 },
//                 { 0x60, 0x0021, 0x95 },
//                 { 0x60, 0x003A, 0x00 },
//                 { 0x60, 0x003B, 0x01 },
//                 { 0x60, 0x003C, 0x00 },
//                 { 0x60, 0x003D, 0x1A },
//                 { 0x60, 0x003E, 0x80 },
//                 { 0x60, 0x003F, 0x00 },
//                 { 0x60, 0x0040, 0x00 },
//                 { 0x60, 0x0041, 0x00 },
//                 { 0x60, 0x005A, 0x00 },
//                 { 0x60, 0x005B, 0x00 },
//                 { 0x60, 0x0095, 0x00 },
//                 { 0x60, 0x0096, 0x00 },
//                 { 0x60, 0x0097, 0x00 },
//                 { 0x60, 0x0098, 0x00 },
//                 { 0x60, 0x0099, 0x00 },
//                 { 0x60, 0x009A, 0x00 },
//                 { 0x60, 0x009B, 0x00 },
//                 { 0x60, 0x00A2, 0x00 },
//                 { 0x60, 0x00A3, 0x00 },
//                 { 0x60, 0x00A4, 0x00 },
//                 { 0x60, 0x00A7, 0x00 },
//                 { 0x60, 0x00B7, 0x12 },
//             };
//
//
//    uint16_t i = 0;
//
//    for ( i=0; i < sizeof(conf) / sizeof(conf[0]); i++)
//    {
//        DELAY_US(1000);
//        if(si5351c_write_register(conf[i][1], conf[i][2]))
//            return SI5351C_STATUS_CONFIG_ERROR;
//    }
//
//
//    return SI5351C_STATUS_OK;
//}

si5351c_status_t si5351c_write_initial_config(void)
{


#define SI5351C_REVB_REG_CONFIG_NUM_REGS_ADS                52

si5351c_revb_register_t const si5351c_revb_registers_ads[SI5351C_REVB_REG_CONFIG_NUM_REGS_ADS] =
{

 //23MHz
/*
 { 0x0002, 0x53 },
 { 0x0003, 0x00 },
 { 0x0004, 0x20 },
 { 0x0007, 0x01 },
 { 0x000F, 0x00 },
 { 0x0010, 0x0F },
 { 0x0011, 0x8C },
 { 0x0012, 0x0F },
 { 0x0013, 0x8C },
 { 0x0014, 0x8C },
 { 0x0015, 0x8C },
 { 0x0016, 0x8C },
 { 0x0017, 0x8C },
 { 0x001A, 0x00 },
 { 0x001B, 0x19 },
 { 0x001C, 0x00 },
 { 0x001D, 0x0F },
 { 0x001E, 0xF0 },
 { 0x001F, 0x00 },
 { 0x0020, 0x00 },
 { 0x0021, 0x10 },
 { 0x002A, 0x00 },
 { 0x002B, 0x01 },
 { 0x002C, 0x00 },
 { 0x002D, 0x11 },
 { 0x002E, 0x80 },
 { 0x002F, 0x00 },
 { 0x0030, 0x00 },
 { 0x0031, 0x00 },
 { 0x003A, 0x00 },
 { 0x003B, 0x01 },
 { 0x003C, 0x00 },
 { 0x003D, 0x25 },
 { 0x003E, 0x00 },
 { 0x003F, 0x00 },
 { 0x0040, 0x00 },
 { 0x0041, 0x00 },
 { 0x005A, 0x00 },
 { 0x005B, 0x00 },
 { 0x0095, 0x00 },
 { 0x0096, 0x00 },
 { 0x0097, 0x00 },
 { 0x0098, 0x00 },
 { 0x0099, 0x00 },
 { 0x009A, 0x00 },
 { 0x009B, 0x00 },
 { 0x00A2, 0x00 },
 { 0x00A3, 0x00 },
 { 0x00A4, 0x00 },
 { 0x00A5, 0x00 },
 { 0x00A7, 0x00 },
 { 0x00B7, 0x92 },
*/
 //22MHz
/* { 0x0002, 0x53 },
  { 0x0003, 0x00 },
  { 0x0004, 0x20 },
  { 0x0007, 0x01 },
  { 0x000F, 0x00 },
  { 0x0010, 0x0F },
  { 0x0011, 0x8C },
  { 0x0012, 0x0F },
  { 0x0013, 0x8C },
  { 0x0014, 0x8C },
  { 0x0015, 0x8C },
  { 0x0016, 0x8C },
  { 0x0017, 0x8C },
  { 0x001A, 0x00 },
  { 0x001B, 0x01 },
  { 0x001C, 0x00 },
  { 0x001D, 0x0E },
  { 0x001E, 0x80 },
  { 0x001F, 0x00 },
  { 0x0020, 0x00 },
  { 0x0021, 0x00 },
  { 0x002A, 0x00 },
  { 0x002B, 0x02 },
  { 0x002C, 0x00 },
  { 0x002D, 0x10 },
  { 0x002E, 0xC0 },
  { 0x002F, 0x00 },
  { 0x0030, 0x00 },
  { 0x0031, 0x00 },
  { 0x003A, 0x00 },
  { 0x003B, 0x01 },
  { 0x003C, 0x00 },
  { 0x003D, 0x23 },
  { 0x003E, 0x80 },
  { 0x003F, 0x00 },
  { 0x0040, 0x00 },
  { 0x0041, 0x00 },
  { 0x005A, 0x00 },
  { 0x005B, 0x00 },
  { 0x0095, 0x00 },
  { 0x0096, 0x00 },
  { 0x0097, 0x00 },
  { 0x0098, 0x00 },
  { 0x0099, 0x00 },
  { 0x009A, 0x00 },
  { 0x009B, 0x00 },
  { 0x00A2, 0x00 },
  { 0x00A3, 0x00 },
  { 0x00A4, 0x00 },
  { 0x00A5, 0x00 },
  { 0x00A7, 0x00 },
  { 0x00B7, 0x92 },
*/

 //20MHz
 { 0x0002, 0x53 },
  { 0x0003, 0x00 },
  { 0x0004, 0x20 },
  { 0x0007, 0x01 },
  { 0x000F, 0x00 },
  { 0x0010, 0x0F },
  { 0x0011, 0x8C },
  { 0x0012, 0x0F },
  { 0x0013, 0x8C },
  { 0x0014, 0x8C },
  { 0x0015, 0x8C },
  { 0x0016, 0x8C },
  { 0x0017, 0x8C },
  { 0x001A, 0x00 },
  { 0x001B, 0x01 },
  { 0x001C, 0x00 },
  { 0x001D, 0x10 },
  { 0x001E, 0x00 },
  { 0x001F, 0x00 },
  { 0x0020, 0x00 },
  { 0x0021, 0x00 },
  { 0x002A, 0x00 },
  { 0x002B, 0x01 },
  { 0x002C, 0x00 },
  { 0x002D, 0x14 },
  { 0x002E, 0x80 },
  { 0x002F, 0x00 },
  { 0x0030, 0x00 },
  { 0x0031, 0x00 },
  { 0x003A, 0x00 },
  { 0x003B, 0x01 },
  { 0x003C, 0x00 },
  { 0x003D, 0x2B },
  { 0x003E, 0x00 },
  { 0x003F, 0x00 },
  { 0x0040, 0x00 },
  { 0x0041, 0x00 },
  { 0x005A, 0x00 },
  { 0x005B, 0x00 },
  { 0x0095, 0x00 },
  { 0x0096, 0x00 },
  { 0x0097, 0x00 },
  { 0x0098, 0x00 },
  { 0x0099, 0x00 },
  { 0x009A, 0x00 },
  { 0x009B, 0x00 },
  { 0x00A2, 0x00 },
  { 0x00A3, 0x00 },
  { 0x00A4, 0x00 },
  { 0x00A5, 0x00 },
  { 0x00A7, 0x00 },
  { 0x00B7, 0x92 },

 //19MHz
/*
 { 0x0002, 0x53 },
  { 0x0003, 0x00 },
  { 0x0004, 0x20 },
  { 0x0007, 0x01 },
  { 0x000F, 0x00 },
  { 0x0010, 0x0F },
  { 0x0011, 0x8C },
  { 0x0012, 0x0F },
  { 0x0013, 0x8C },
  { 0x0014, 0x8C },
  { 0x0015, 0x8C },
  { 0x0016, 0x8C },
  { 0x0017, 0x8C },
  { 0x001A, 0x00 },
  { 0x001B, 0x19 },
  { 0x001C, 0x00 },
  { 0x001D, 0x0F },
  { 0x001E, 0xDC },
  { 0x001F, 0x00 },
  { 0x0020, 0x00 },
  { 0x0021, 0x04 },
  { 0x002A, 0x00 },
  { 0x002B, 0x01 },
  { 0x002C, 0x00 },
  { 0x002D, 0x15 },
  { 0x002E, 0x80 },
  { 0x002F, 0x00 },
  { 0x0030, 0x00 },
  { 0x0031, 0x00 },
  { 0x003A, 0x00 },
  { 0x003B, 0x01 },
  { 0x003C, 0x00 },
  { 0x003D, 0x2D },
  { 0x003E, 0x00 },
  { 0x003F, 0x00 },
  { 0x0040, 0x00 },
  { 0x0041, 0x00 },
  { 0x005A, 0x00 },
  { 0x005B, 0x00 },
  { 0x0095, 0x00 },
  { 0x0096, 0x00 },
  { 0x0097, 0x00 },
  { 0x0098, 0x00 },
  { 0x0099, 0x00 },
  { 0x009A, 0x00 },
  { 0x009B, 0x00 },
  { 0x00A2, 0x00 },
  { 0x00A3, 0x00 },
  { 0x00A4, 0x00 },
  { 0x00A5, 0x00 },
  { 0x00A7, 0x00 },
  { 0x00B7, 0x92 },*/

// 15.729MHz
/*
 { 0x0002, 0x53 },
 { 0x0003, 0x00 },
 { 0x0004, 0x20 },
 { 0x0007, 0x01 },
 { 0x000F, 0x00 },
 { 0x0010, 0x0F },
 { 0x0011, 0x8C },
 { 0x0012, 0x0F },
 { 0x0013, 0x8C },
 { 0x0014, 0x8C },
 { 0x0015, 0x8C },
 { 0x0016, 0x8C },
 { 0x0017, 0x8C },
 { 0x001A, 0x31 },
 { 0x001B, 0x2D },
 { 0x001C, 0x00 },
 { 0x001D, 0x0F },
 { 0x001E, 0xEE },
 { 0x001F, 0x10 },
 { 0x0020, 0x4B },
 { 0x0021, 0x2A },
 { 0x002A, 0x00 },
 { 0x002B, 0x01 },
 { 0x002C, 0x00 },
 { 0x002D, 0x1A },
 { 0x002E, 0x80 },
 { 0x002F, 0x00 },
 { 0x0030, 0x00 },
 { 0x0031, 0x00 },
 { 0x003A, 0x00 },
 { 0x003B, 0x01 },
 { 0x003C, 0x00 },
 { 0x003D, 0x37 },
 { 0x003E, 0x00 },
 { 0x003F, 0x00 },
 { 0x0040, 0x00 },
 { 0x0041, 0x00 },
 { 0x005A, 0x00 },
 { 0x005B, 0x00 },
 { 0x0095, 0x00 },
 { 0x0096, 0x00 },
 { 0x0097, 0x00 },
 { 0x0098, 0x00 },
 { 0x0099, 0x00 },
 { 0x009A, 0x00 },
 { 0x009B, 0x00 },
 { 0x00A2, 0x00 },
 { 0x00A3, 0x00 },
 { 0x00A4, 0x00 },
 { 0x00A5, 0x00 },
 { 0x00A7, 0x00 },
 { 0x00B7, 0x92 },
*/
};



    uint16_t i = 0;

    for ( i=0; i < SI5351C_REVB_REG_CONFIG_NUM_REGS_ADS; i++)
    {
        DELAY_US(1000);
        if(si5351c_write_register(si5351c_revb_registers_ads[i].address, si5351c_revb_registers_ads[i].value))
            return SI5351C_STATUS_CONFIG_ERROR;
    }


    return SI5351C_STATUS_OK;
}


si5351c_status_t si5351c_stop_gen(void)
{
    if(si5351c_write_register(0x03, 0xFF))
    {
        return SI5351C_STATUS_CMD_STOP_ERROR;
    }
    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c_start_gen(void)
{
    DELAY_US(1000);
    if(si5351c_write_register(0x03, 0x00))
    {
        return SI5351C_STATUS_CMD_STOP_ERROR;
    }
    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c_interrupts_off(void)
{
    uint8_t regs[8] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};


    if(si5351_write_sequence(0x10, &regs[0], 8))
    {
        return SI5351C_STATUS_INT_OFF_ERROR;
    }
    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351_clock_enable(enum si5351_clock clk, uint8_t enable)
{
    uint8_t reg_val;

    DELAY_US(1000);
    if(si5351c_read_register(SI5351_OUTPUT_ENABLE_CTRL, &reg_val))
    {
        return SI5351C_STATUS_CLK_ENA_ERROR;
    }

    if(enable == 1)
    {
        reg_val &= ~(1<<(uint8_t)clk);
    }
    else
    {
        reg_val |= (1<<(uint8_t)clk);
    }

    DELAY_US(1000);
    if(si5351c_write_register(SI5351_OUTPUT_ENABLE_CTRL, reg_val))
    {
        return SI5351C_STATUS_CLK_ENA_ERROR;
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351_update_sys_status(struct Si5351Status *status)
{
    uint8_t reg_val = 0;

    if(si5351c_read_register(SI5351_DEVICE_STATUS, &reg_val))
    {
        return SI5351C_STATUS_SYS_READ_ERROR;
    }

    /* Parse the register */
    status->SYS_INIT = (reg_val >> 7) & 0x01;
    status->LOL_B = (reg_val >> 6) & 0x01;
    status->LOL_A = (reg_val >> 5) & 0x01;
    status->LOS = (reg_val >> 4) & 0x01;
    status->REVID = reg_val & 0x03;

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351_update_int_status(struct Si5351IntStatus *int_status)
{
    uint8_t reg_val = 0;

    if(si5351c_read_register(SI5351_DEVICE_STATUS, &reg_val))
    {
        return SI5351C_STATUS_INT_READ_ERROR;
    }

    /* Parse the register */
    int_status->SYS_INIT_STKY = (reg_val >> 7) & 0x01;
    int_status->LOL_B_STKY = (reg_val >> 6) & 0x01;
    int_status->LOL_A_STKY = (reg_val >> 5) & 0x01;
    int_status->LOS_STKY = (reg_val >> 4) & 0x01;

    return SI5351C_STATUS_OK;
}



void si5351_set_freq(uint32_t freq, uint64_t pll_freq, enum si5351_clock clk)
{
    struct Si5351RegSet ms_reg, pll_reg;
    enum si5351_pll target_pll;
    uint8_t si5351_buff[30] = {0};
    enum si5351_r0div r0div = SI5351_R0_DIV1;

    /* Check if input freq is < 4kHz */
    if (freq < 4000)
        freq = 4000;

    /* Loop until input freq times R0_DIV is > 500kHz */
    if (freq < 500000)
    {
        for (r0div=SI5351_R0_DIV1; r0div<SI5351_R0_SIZE; r0div++)
        {
            if (freq*(1<<r0div) > 500000)
            {
                break;
            }
        }
        freq *= (1<<r0div);             // Update new valid frequency (it will be divided by R0 Output divider)
    }

    /* Calculate the synth parameters */
    /* If pll_freq is 0, let the algorithm pick a PLL frequency */
    if(pll_freq == 0)
    {
        pll_freq = multisynth_calc(freq, &ms_reg);
    }
    else
    {
        multisynth_recalc(freq, pll_freq, &ms_reg);
    }

    /* Determine which PLL to use */
    /* CLK0 gets PLLA, CLK1 gets PLLB */
    /* CLK2 gets PLLB if necessary */
    /* Only good for Si5351A3 variant at the moment */
    if(clk == SI5351_CLK0)
    {
        target_pll = SI5351_PLLA;
        plla_freq = pll_freq;
    }
    else if(clk == SI5351_CLK1)
    {
        target_pll = SI5351_PLLB;
        pllb_freq = pll_freq;
    }
    else
    {
        /* need to account for CLK2 set before CLK1 */
        if(pllb_freq == 0)
        {
            target_pll = SI5351_PLLB;
            pllb_freq = pll_freq;
        }
        else
        {
            target_pll = SI5351_PLLB;
            pll_freq = pllb_freq;
            multisynth_recalc(freq, pll_freq, &ms_reg);
        }
    }

    pll_calc(pll_freq, &pll_reg, ref_correction);

    /* Derive the register values to write */
    uint8_t i = 0;
    uint8_t temp;

    /* PLL parameters first */


    /* Registers 26-27 */
    temp = ((pll_reg.p3 >> 8) & 0xFF);
    si5351_buff[i++] = temp;

    temp = (uint8_t)(pll_reg.p3  & 0xFF);
    si5351_buff[i++] = temp;

    /* Register 28 */
    temp = (uint8_t)((pll_reg.p1 >> 16) & 0x03);
    si5351_buff[i++] = temp;

    /* Registers 29-30 */
    temp = (uint8_t)((pll_reg.p1 >> 8) & 0xFF);
    si5351_buff[i++] = temp;

    temp = (uint8_t)(pll_reg.p1  & 0xFF);
    si5351_buff[i++] = temp;

    /* Register 31 */
    temp = (uint8_t)((pll_reg.p3 >> 12) & 0xF0);
    temp += (uint8_t)((pll_reg.p2 >> 16) & 0x0F);
    si5351_buff[i++] = temp;

    /* Registers 32-33 */
    temp = (uint8_t)((pll_reg.p2 >> 8) & 0xFF);
    si5351_buff[i++] = temp;

    temp = (uint8_t)(pll_reg.p2  & 0xFF);
    si5351_buff[i++] = temp;

    /* Write the parameters */
    if(target_pll == SI5351_PLLA)
    {
        si5351_write_sequence(SI5351_PLLA_PARAMETERS, &si5351_buff[0], i);
    }
    else if(target_pll == SI5351_PLLB)
    {
        si5351_write_sequence(SI5351_PLLB_PARAMETERS, &si5351_buff[0], i);
    }

    i = 0;              // Reset buffer position

    /* Registers 42-43 */
    temp = (uint8_t)((ms_reg.p3 >> 8) & 0xFF);
    si5351_buff[i++] = temp;

    temp = (uint8_t)(ms_reg.p3  & 0xFF);
    si5351_buff[i++] = temp;

    /* Register 44 */
    /* TODO: add code for output divider */
    temp = (uint8_t)((ms_reg.p1 >> 16) & 0x03);
    si5351_buff[i++] = temp + (r0div<<4);

    /* Registers 45-46 */
    temp = (uint8_t)((ms_reg.p1 >> 8) & 0xFF);
    si5351_buff[i++] = temp;

    temp = (uint8_t)(ms_reg.p1  & 0xFF);
    si5351_buff[i++] = temp;

    /* Register 47 */
    temp = (uint8_t)((ms_reg.p3 >> 12) & 0xF0);
    temp += (uint8_t)((ms_reg.p2 >> 16) & 0x0F);
    si5351_buff[i++] = temp;

    /* Registers 48-49 */
    temp = (uint8_t)((ms_reg.p2 >> 8) & 0xFF);
    si5351_buff[i++] = temp;

    temp = (uint8_t)(ms_reg.p2  & 0xFF);
    si5351_buff[i++] = temp;

    /* Write the parameters */
    switch(clk)
    {
    case SI5351_CLK0:
        si5351_write_sequence(SI5351_CLK0_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK1:
        si5351_write_sequence(SI5351_CLK1_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK2:
        si5351_write_sequence(SI5351_CLK2_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK3:
        si5351_write_sequence(SI5351_CLK3_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK4:
        si5351_write_sequence(SI5351_CLK4_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK5:
        si5351_write_sequence(SI5351_CLK5_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK6:
        si5351_write_sequence(SI5351_CLK6_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    case SI5351_CLK7:
        si5351_write_sequence(SI5351_CLK7_PARAMETERS, &si5351_buff[0], i);
        si5351_set_ms_source(clk, target_pll);
        break;
    }

}

uint64_t multisynth_calc(uint32_t freq, struct Si5351RegSet *reg)
{
    uint32_t pll_freq;
    uint64_t lltmp;
    uint32_t a, b, c, p1, p2, p3;
    uint8_t divby4;

    /* Multisynth bounds checking */
    if (freq > SI5351_MULTISYNTH_MAX_FREQ)
        freq = SI5351_MULTISYNTH_MAX_FREQ;
    if (freq < SI5351_MULTISYNTH_MIN_FREQ)
        freq = SI5351_MULTISYNTH_MIN_FREQ;

    divby4 = 0;
    if (freq > SI5351_MULTISYNTH_DIVBY4_FREQ)
        divby4 = 1;

    /* Find largest integer divider for max */
    /* VCO frequency and given target frequency */
    if (divby4 == 0)
    {
        lltmp = SI5351_PLL_VCO_MAX;
        do_div(lltmp, freq);
        a = (uint32_t)lltmp;
    }
    else
        a = 4;

    b = 0;
    c = 1;
    pll_freq = a * freq;

    /* Recalculate output frequency by fOUT = fIN / (a + b/c) */
    lltmp  = pll_freq;
    lltmp *= c;
    do_div(lltmp, a * c + b);
    freq  = (unsigned long)lltmp;

    /* Calculate parameters */
    if (divby4)
    {
        p3 = 1;
        p2 = 0;
        p1 = 0;
    }
    else
    {
        p3  = c;
        p2  = (128 * b) % c;
        p1  = 128 * a;
        p1 += (128 * b / c);
        p1 -= 512;
    }

    reg->p1 = p1;
    reg->p2 = p2;
    reg->p3 = p3;

    return pll_freq;
}

uint32_t multisynth_recalc(uint32_t freq, uint32_t pll_freq, struct Si5351RegSet *reg)
{
    uint64_t lltmp;
    uint32_t rfrac, denom, a, b, c, p1, p2, p3;
    uint8_t divby4;

    /* Multisynth bounds checking */
    if (freq > SI5351_MULTISYNTH_MAX_FREQ)
        freq = SI5351_MULTISYNTH_MAX_FREQ;
    if (freq < SI5351_MULTISYNTH_MIN_FREQ)
        freq = SI5351_MULTISYNTH_MIN_FREQ;

    divby4 = 0;
    if (freq > SI5351_MULTISYNTH_DIVBY4_FREQ)
        divby4 = 1;

    /* Determine integer part of feedback equation */
    a = pll_freq / freq;

    /* TODO: need to check if it is ok */
    if (a < SI5351_MULTISYNTH_A_MIN)
        freq = pll_freq / SI5351_MULTISYNTH_A_MIN;
    if (a > SI5351_MULTISYNTH_A_MAX)
        freq = pll_freq / SI5351_MULTISYNTH_A_MAX;

    /* find best approximation for b/c */
    denom = 1000L * 1000L;
    lltmp = pll_freq % freq;
    lltmp *= denom;
    do_div(lltmp, freq);
    rfrac = (uint32_t)lltmp;

    b = 0;
    c = 1;
    if (rfrac)
        rational_best_approximation(rfrac, denom, SI5351_MULTISYNTH_B_MAX, SI5351_MULTISYNTH_C_MAX, &b, &c);

    /* Recalculate output frequency by fOUT = fIN / (a + b/c) */
    lltmp  = pll_freq;
    lltmp *= c;
    do_div(lltmp, a * c + b);
    freq  = (unsigned long)lltmp;

    /* Calculate parameters */
    if (divby4)
    {
        p3 = 1;
        p2 = 0;
        p1 = 0;
    }
    else
    {
        p3  = c;
        p2  = (128 * b) % c;
        p1  = 128 * a;
        p1 += (128 * b / c);
        p1 -= 512;
    }

    reg->p1 = p1;
    reg->p2 = p2;
    reg->p3 = p3;

    return freq;
}

uint32_t pll_calc(uint32_t freq, struct Si5351RegSet *reg, int32_t correction)
{
    uint32_t ref_freq = SI5351_XTAL_FREQ;
    uint32_t rfrac, denom, a, b, c, p1, p2, p3;
    uint64_t lltmp;

    /* Factor calibration value into nominal crystal frequency */
    /* Measured in parts-per-ten million */
    ref_freq += (uint32_t)((double)(correction / 10000000.0) * (double)ref_freq);

    /* PLL bounds checking */
    if (freq < SI5351_PLL_VCO_MIN)
        freq = SI5351_PLL_VCO_MIN;
    if (freq > SI5351_PLL_VCO_MAX)
        freq = SI5351_PLL_VCO_MAX;

    /* Determine integer part of feedback equation */
    a = freq / ref_freq;

    if (a < SI5351_PLL_A_MIN)
        freq = ref_freq * SI5351_PLL_A_MIN;
    if (a > SI5351_PLL_A_MAX)
        freq = ref_freq * SI5351_PLL_A_MAX;

    /* find best approximation for b/c = fVCO mod fIN */
    denom = 1000L * 1000L;
    lltmp = freq % ref_freq;
    lltmp *= denom;
    do_div(lltmp, ref_freq);
    rfrac = (uint32_t)lltmp;

    b = 0;
    c = 1;
    if (rfrac)
        rational_best_approximation(rfrac, denom, SI5351_PLL_B_MAX, SI5351_PLL_C_MAX, &b, &c);

    /* calculate parameters */
    p3  = c;
    p2  = (128 * b) % c;
    p1  = 128 * a;
    p1 += (128 * b / c);
    p1 -= 512;

    /* recalculate rate by fIN * (a + b/c) */
    lltmp  = ref_freq;
    lltmp *= b;
    do_div(lltmp, c);

    freq  = (uint32_t)lltmp;
    freq += ref_freq * a;

    reg->p1 = p1;
    reg->p2 = p2;
    reg->p3 = p3;

    return freq;
}

si5351c_status_t si5351_set_ms_source(enum si5351_clock clk, enum si5351_pll pll)
{
    uint8_t reg_val = 0x0c;
    uint8_t reg_val2;


    if(si5351c_read_register(SI5351_CLK0_CTRL + (uint8_t)clk, &reg_val2))
    {
        return SI5351C_STATUS_READ_ERROR;
    }

    if(pll == SI5351_PLLA)
    {
        reg_val &= ~(SI5351_CLK_PLL_SELECT);
    }
    else if(pll == SI5351_PLLB)
    {
        reg_val |= SI5351_CLK_PLL_SELECT;
    }
    si5351c_write_register(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val);

    return SI5351C_STATUS_OK;
}

void rational_best_approximation(
        unsigned long given_numerator, unsigned long given_denominator,
        unsigned long max_numerator, unsigned long max_denominator,
        unsigned long *best_numerator, unsigned long *best_denominator)
{
    unsigned long n, d, n0, d0, n1, d1;
    n = given_numerator;
    d = given_denominator;
    n0 = d1 = 0;
    n1 = d0 = 1;
    for (;;) {
        unsigned long t, a;
        if ((n1 > max_numerator) || (d1 > max_denominator)) {
            n1 = n0;
            d1 = d0;
            break;
        }
        if (d == 0)
            break;
        t = d;
        a = n / d;
        d = n % d;
        n = t;
        t = n0 + a * n1;
        n0 = n1;
        n1 = t;
        t = d0 + a * d1;
        d0 = d1;
        d1 = t;
    }
    *best_numerator = n1;
    *best_denominator = d1;
}






////////////////////////////////////////




si5351c_status_t si5351c2_write_register(uint8_t reg, uint8_t value)
{
    DELAY_US(200);
    if (I2CB_WriteToReg(SI5351C_ADDR, reg, (unsigned char *)&value, 1))
    {
        return SI5351C_STATUS_WRITE_ERROR;
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c2_read_register(uint8_t reg, uint8_t * value)
{
    DELAY_US(200);
    if (I2CB_ReadFromReg(SI5351C_ADDR, reg, (unsigned char *)value, 1))
    {
        return SI5351C_STATUS_READ_ERROR;
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c2_write_sequence(uint8_t addr, uint8_t *buffer, uint8_t length)
{
    uint8_t i = 0;

    for (i=0;i<length;i++)
    {
        DELAY_US(1000);
        if(si5351c2_write_register(addr++, buffer[i]))
        {
            return SI5351C_STATUS_SEQ_WRITE_ERROR;
        }
    }

    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c2_init(void)
{
    DELAY_US(10);

    if(si5351c2_stop_gen())                                  // STEP 1: Disable outputs
        return SI5351C_STATUS_INIT_ERROR;

    if(si5351c2_interrupts_off())                            // STEP 2: Power-down all output drivers
        return SI5351C_STATUS_INIT_ERROR;

    //    if(si5351c_write_register(0x02, 0x50))                  // STEP 3: Set interrput masks
    //        return SI5351C_STATUS_INIT_ERROR;

    if(si5351c2_write_initial_config())                      // STEP 4: Write initial config to registers
        return SI5351C_STATUS_INIT_ERROR;

    //    if(si5351c_write_register(0x00B1, 0xAC))              // STEP 5: Apply PLLA and PLLB soft reset
    //        return SI5351C_STATUS_INIT_ERROR;

    return SI5351C_STATUS_OK;
}


si5351c_status_t si5351c2_write_initial_config(void)
{
    uint16_t i = 0;

    for ( i=0; i < SI5351C_REVB_REG_CONFIG_NUM_REGS; i++)
    {
        DELAY_US(1000);
        if(si5351c2_write_register(si5351c_revb_registers60[i].address, si5351c_revb_registers60[i].value))
            return SI5351C_STATUS_CONFIG_ERROR;
    }

    return SI5351C_STATUS_OK;
}


si5351c_status_t si5351c2_stop_gen(void)
{
    DELAY_US(1000);
    if(si5351c2_write_register(0x03, 0xFF))
    {
        return SI5351C_STATUS_CMD_STOP_ERROR;
    }
    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c2_start_gen(void)
{
    DELAY_US(1000);
    if(si5351c2_write_register(0x03, 0x00))
    {
        return SI5351C_STATUS_CMD_STOP_ERROR;
    }
    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c2_interrupts_off(void)
{
    uint8_t regs[8] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    DELAY_US(1000);

    if(si5351c2_write_sequence(0x10, &regs[0], 8))
    {
        return SI5351C_STATUS_INT_OFF_ERROR;
    }
    return SI5351C_STATUS_OK;
}

si5351c_status_t si5351c2_clock_enable(enum si5351_clock clk, uint8_t enable)
{
    uint8_t reg_val;

    DELAY_US(1000);
    if(si5351c2_read_register(SI5351_OUTPUT_ENABLE_CTRL, &reg_val))
    {
        return SI5351C_STATUS_CLK_ENA_ERROR;
    }

    if(enable == 1)
    {
        reg_val &= ~(1<<(uint8_t)clk);
    }
    else
    {
        reg_val |= (1<<(uint8_t)clk);
    }

    DELAY_US(1000);
    if(si5351c2_write_register(SI5351_OUTPUT_ENABLE_CTRL, reg_val))
    {
        return SI5351C_STATUS_CLK_ENA_ERROR;
    }

    return SI5351C_STATUS_OK;
}
