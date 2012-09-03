/**
 * \file cdrom_crc.c
 * Functions and types for CD-ROM CRC checks.
 *
 * Generated on Mon Sep  3 08:13:21 2012,
 * by pycrc v0.7.9, http://www.tty1.net/pycrc/
 * using the configuration:
 *    Width        = 32
 *    Poly         = 0x8001801b
 *    XorIn        = 0x00000000
 *    ReflectIn    = True
 *    XorOut       = 0x00000000
 *    ReflectOut   = True
 *    Algorithm    = table-driven
 *****************************************************************************/
#include "cdrom_crc.h"     /* include the header file generated with pycrc */
#include <stdlib.h>
#include <stdint.h>

/**
 * Static table used for the table_driven implementation.
 *****************************************************************************/
static const cdrom_crc_t cdrom_crc_table[256] = {
    0x00000000, 0x90910101, 0x91210201, 0x01b00300,
    0x92410401, 0x02d00500, 0x03600600, 0x93f10701,
    0x94810801, 0x04100900, 0x05a00a00, 0x95310b01,
    0x06c00c00, 0x96510d01, 0x97e10e01, 0x07700f00,
    0x99011001, 0x09901100, 0x08201200, 0x98b11301,
    0x0b401400, 0x9bd11501, 0x9a611601, 0x0af01700,
    0x0d801800, 0x9d111901, 0x9ca11a01, 0x0c301b00,
    0x9fc11c01, 0x0f501d00, 0x0ee01e00, 0x9e711f01,
    0x82012001, 0x12902100, 0x13202200, 0x83b12301,
    0x10402400, 0x80d12501, 0x81612601, 0x11f02700,
    0x16802800, 0x86112901, 0x87a12a01, 0x17302b00,
    0x84c12c01, 0x14502d00, 0x15e02e00, 0x85712f01,
    0x1b003000, 0x8b913101, 0x8a213201, 0x1ab03300,
    0x89413401, 0x19d03500, 0x18603600, 0x88f13701,
    0x8f813801, 0x1f103900, 0x1ea03a00, 0x8e313b01,
    0x1dc03c00, 0x8d513d01, 0x8ce13e01, 0x1c703f00,
    0xb4014001, 0x24904100, 0x25204200, 0xb5b14301,
    0x26404400, 0xb6d14501, 0xb7614601, 0x27f04700,
    0x20804800, 0xb0114901, 0xb1a14a01, 0x21304b00,
    0xb2c14c01, 0x22504d00, 0x23e04e00, 0xb3714f01,
    0x2d005000, 0xbd915101, 0xbc215201, 0x2cb05300,
    0xbf415401, 0x2fd05500, 0x2e605600, 0xbef15701,
    0xb9815801, 0x29105900, 0x28a05a00, 0xb8315b01,
    0x2bc05c00, 0xbb515d01, 0xbae15e01, 0x2a705f00,
    0x36006000, 0xa6916101, 0xa7216201, 0x37b06300,
    0xa4416401, 0x34d06500, 0x35606600, 0xa5f16701,
    0xa2816801, 0x32106900, 0x33a06a00, 0xa3316b01,
    0x30c06c00, 0xa0516d01, 0xa1e16e01, 0x31706f00,
    0xaf017001, 0x3f907100, 0x3e207200, 0xaeb17301,
    0x3d407400, 0xadd17501, 0xac617601, 0x3cf07700,
    0x3b807800, 0xab117901, 0xaaa17a01, 0x3a307b00,
    0xa9c17c01, 0x39507d00, 0x38e07e00, 0xa8717f01,
    0xd8018001, 0x48908100, 0x49208200, 0xd9b18301,
    0x4a408400, 0xdad18501, 0xdb618601, 0x4bf08700,
    0x4c808800, 0xdc118901, 0xdda18a01, 0x4d308b00,
    0xdec18c01, 0x4e508d00, 0x4fe08e00, 0xdf718f01,
    0x41009000, 0xd1919101, 0xd0219201, 0x40b09300,
    0xd3419401, 0x43d09500, 0x42609600, 0xd2f19701,
    0xd5819801, 0x45109900, 0x44a09a00, 0xd4319b01,
    0x47c09c00, 0xd7519d01, 0xd6e19e01, 0x46709f00,
    0x5a00a000, 0xca91a101, 0xcb21a201, 0x5bb0a300,
    0xc841a401, 0x58d0a500, 0x5960a600, 0xc9f1a701,
    0xce81a801, 0x5e10a900, 0x5fa0aa00, 0xcf31ab01,
    0x5cc0ac00, 0xcc51ad01, 0xcde1ae01, 0x5d70af00,
    0xc301b001, 0x5390b100, 0x5220b200, 0xc2b1b301,
    0x5140b400, 0xc1d1b501, 0xc061b601, 0x50f0b700,
    0x5780b800, 0xc711b901, 0xc6a1ba01, 0x5630bb00,
    0xc5c1bc01, 0x5550bd00, 0x54e0be00, 0xc471bf01,
    0x6c00c000, 0xfc91c101, 0xfd21c201, 0x6db0c300,
    0xfe41c401, 0x6ed0c500, 0x6f60c600, 0xfff1c701,
    0xf881c801, 0x6810c900, 0x69a0ca00, 0xf931cb01,
    0x6ac0cc00, 0xfa51cd01, 0xfbe1ce01, 0x6b70cf00,
    0xf501d001, 0x6590d100, 0x6420d200, 0xf4b1d301,
    0x6740d400, 0xf7d1d501, 0xf661d601, 0x66f0d700,
    0x6180d800, 0xf111d901, 0xf0a1da01, 0x6030db00,
    0xf3c1dc01, 0x6350dd00, 0x62e0de00, 0xf271df01,
    0xee01e001, 0x7e90e100, 0x7f20e200, 0xefb1e301,
    0x7c40e400, 0xecd1e501, 0xed61e601, 0x7df0e700,
    0x7a80e800, 0xea11e901, 0xeba1ea01, 0x7b30eb00,
    0xe8c1ec01, 0x7850ed00, 0x79e0ee00, 0xe971ef01,
    0x7700f000, 0xe791f101, 0xe621f201, 0x76b0f300,
    0xe541f401, 0x75d0f500, 0x7460f600, 0xe4f1f701,
    0xe381f801, 0x7310f900, 0x72a0fa00, 0xe231fb01,
    0x71c0fc00, 0xe151fd01, 0xe0e1fe01, 0x7070ff00
};

/**
 * Reflect all bits of a \a data word of \a data_len bytes.
 *
 * \param data         The data word to be reflected.
 * \param data_len     The width of \a data expressed in number of bits.
 * \return             The reflected data.
 *****************************************************************************/
cdrom_crc_t cdrom_crc_reflect(cdrom_crc_t data, size_t data_len)
{
    unsigned int i;
    cdrom_crc_t ret;

    ret = data & 0x01;
    for (i = 1; i < data_len; i++) {
        data >>= 1;
        ret = (ret << 1) | (data & 0x01);
    }
    return ret;
}


/**
 * Update the crc value with new data.
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param data_len Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 *****************************************************************************/
cdrom_crc_t cdrom_crc_update(cdrom_crc_t crc, const unsigned char *data,
			     size_t data_len)
{
    unsigned int tbl_idx;

    while (data_len--) {
        tbl_idx = (crc ^ *data) & 0xff;
        crc = (cdrom_crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffffff;

        data++;
    }
    return crc & 0xffffffff;
}



