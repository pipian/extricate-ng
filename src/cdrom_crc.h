/**
 * \file cdrom_crc.h
 * Functions and types for CRC checks.
 *
 * Generated on Mon Sep  3 08:16:17 2012,
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
#ifndef _EXTRICATE_CDROM_CRC_H
#define _EXTRICATE_CDROM_CRC_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * The definition of the used algorithm.
 *****************************************************************************/
#define CRC_ALGO_TABLE_DRIVEN 1


/**
 * The type of the CD-ROM CRC values.
 *
 * This type must be big enough to contain at least 32 bits.
 *****************************************************************************/
typedef uint32_t cdrom_crc_t;


/**
 * Reflect all bits of a \a data word of \a data_len bytes.
 *
 * \param data         The data word to be reflected.
 * \param data_len     The width of \a data expressed in number of bits.
 * \return             The reflected data.
 *****************************************************************************/
cdrom_crc_t cdrom_crc_reflect(cdrom_crc_t data, size_t data_len);


/**
 * Calculate the initial CD-ROM crc value.
 *
 * \return     The initial crc value.
 *****************************************************************************/
static inline cdrom_crc_t cdrom_crc_init(void)
{
    return 0x00000000;
}


/**
 * Update the CD-ROM crc value with new data.
 *
 * \param crc      The current crc value.
 * \param data     Pointer to a buffer of \a data_len bytes.
 * \param data_len Number of bytes in the \a data buffer.
 * \return         The updated crc value.
 *****************************************************************************/
cdrom_crc_t cdrom_crc_update(cdrom_crc_t crc, const unsigned char *data,
			     size_t data_len);


/**
 * Calculate the final CD-ROM crc value.
 *
 * \param crc  The current crc value.
 * \return     The final crc value.
 *****************************************************************************/
static inline cdrom_crc_t cdrom_crc_finalize(cdrom_crc_t crc)
{
    return crc ^ 0x00000000;
}


#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif

#endif      /* _EXTRICATE_CDROM_CRC_H */
