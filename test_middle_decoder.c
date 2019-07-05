/*
 * test_middle_decoder.c
 *
 *  Created on: 5/7/2019
 *      Author: mario
 */


#include <stdint.h>
#include "lvgl.h"


/* ****************************************************************************************** */
/* DEFINES                                                                                    */
/* ****************************************************************************************** */



/* ****************************************************************************************** */
/* DEFINE TYPES (enum, struct...)                                                             */
/* ****************************************************************************************** */



/* ****************************************************************************************** */
/* GLOBAL VARIABLES                                                                           */
/* ****************************************************************************************** */

static uint8_t        decodedImageBitmap[1250];   /**< Frame buffer to store bitmaps of decoded icons. */
static lv_img_dsc_t   decodedImage;               /**< Decoded image info. */


/* ****************************************************************************************** */
/* STATIC FUNCTION DECLARATIONS                                                               */
/* ****************************************************************************************** */


/* ****************************************************************************************** */
/* CONSTANTS                                                                                  */
/* ****************************************************************************************** */

/*************************/
/*      RLE ICONS        */
/*************************/
static const uint8_t numero_01_50[] = {
0x04, 0x5a, 0xff, 0xff, 0x02, 0x49, 0xff, 0xff, 0xff, 0x48, 0xef, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xdf,
0xff, 0xc6, 0xff, 0xff, 0xaf, 0xd6, 0x02, 0xff, 0xff, 0x67, 0x04, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06,
0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff,
0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff,
0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06, 0xff, 0xff, 0x06,
0xff, 0xff, 0xff,
};

static const uint8_t numero_02_50[] = {
0x04, 0x7a, 0xdf, 0xff, 0xfe, 0xb6, 0x08, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc5, 0x04, 0x5c, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xfe, 0x50, 0x35, 0xff, 0xff, 0xeb, 0x98, 0x9c, 0xff, 0xff, 0xfd, 0x40, 0x3b, 0xfe, 0x74, 0x05, 0x5d, 0xff,
0xff, 0x80, 0x36, 0xa4, 0x08, 0x4e, 0xff, 0xfd, 0x0f, 0x7f, 0xff, 0xf5, 0x0e, 0x4f, 0xff, 0xf6, 0x0f, 0xef, 0xff, 0x70,
0xfe, 0xff, 0xf6, 0x0e, 0x4f, 0xff, 0xf5, 0x0e, 0x7f, 0xff, 0xe0, 0xfb, 0xff, 0xfb, 0x0e, 0x5f, 0xff, 0xf6, 0x0e, 0xcf,
0xff, 0xc0, 0xe8, 0xff, 0xff, 0x50, 0xd6, 0xff, 0xff, 0x80, 0xd5, 0xef, 0xff, 0xb0, 0xd4, 0xef, 0xff, 0xd4, 0x0c, 0x4d,
0xff, 0xfe, 0x50, 0xc4, 0xdf, 0xff, 0xf5, 0x0c, 0x4d, 0xff, 0xff, 0x50, 0xc4, 0xdf, 0xff, 0xf5, 0x0c, 0x4d, 0xff, 0xff,
0x50, 0xc4, 0xef, 0xff, 0xe5, 0x0c, 0x5e, 0xff, 0xfe, 0x40, 0xc5, 0xef, 0xff, 0xf9, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
0x01, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x10,
};


const lv_img_dsc_t RLE_ICONS[2] = {

    { .header = { .cf = LV_IMG_CF_USER_ENCODED_0, .always_zero = 0, .w = 10,  .h = 30},  .data_size = 103,  .data = numero_01_50 },
    { .header = { .cf = LV_IMG_CF_USER_ENCODED_0, .always_zero = 0, .w = 20,  .h = 30},  .data_size = 173,  .data = numero_02_50 },
};


/***************************/
/*      4-BIT INDEXED      */
/***************************/

const LV_ATTRIBUTE_MEM_ALIGN uint8_t header_indexed_16_map[] = {
  0x24, 0x22, 0x24, 0xff,   /*Color of index 0*/
  0x3c, 0x3a, 0x3c, 0xff,   /*Color of index 1*/
  0x2c, 0x2e, 0x2c, 0xff,   /*Color of index 2*/
  0x44, 0x46, 0x44, 0xff,   /*Color of index 3*/
  0x2c, 0x2a, 0x2c, 0xff,   /*Color of index 4*/
  0x44, 0x42, 0x44, 0xff,   /*Color of index 5*/
  0x34, 0x36, 0x34, 0xff,   /*Color of index 6*/
  0x24, 0x26, 0x24, 0xff,   /*Color of index 7*/
  0x3c, 0x3e, 0x3c, 0xff,   /*Color of index 8*/
  0x34, 0x32, 0x34, 0xff,   /*Color of index 9*/
  0x4c, 0x4a, 0x4c, 0xff,   /*Color of index 10*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 11*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 12*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 13*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 14*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 15*/

  0x3a,
  0x35,
  0x53,
  0x58,
  0x85,
  0x81,
  0x18,
  0x16,
  0x61,
  0x69,
  0x96,
  0x92,
  0x29,
  0x24,
  0x42,
  0x47,
  0x74,
  0x70,
};


const LV_ATTRIBUTE_MEM_ALIGN uint8_t box_indexed_16_map[] = {
  0x24, 0x22, 0x24, 0xff,   /*Color of index 0*/
  0x3c, 0x3a, 0x3c, 0xff,   /*Color of index 1*/
  0x2c, 0x2e, 0x2c, 0xff,   /*Color of index 2*/
  0x44, 0x46, 0x44, 0xff,   /*Color of index 3*/
  0x2c, 0x2a, 0x2c, 0xff,   /*Color of index 4*/
  0x44, 0x42, 0x44, 0xff,   /*Color of index 5*/
  0x34, 0x36, 0x34, 0xff,   /*Color of index 6*/
  0x24, 0x26, 0x24, 0xff,   /*Color of index 7*/
  0x3c, 0x3e, 0x3c, 0xff,   /*Color of index 8*/
  0x34, 0x32, 0x34, 0xff,   /*Color of index 9*/
  0x4c, 0x4a, 0x4c, 0xff,   /*Color of index 10*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 11*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 12*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 13*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 14*/
  0xff, 0xff, 0xff, 0xff,   /*Color of index 15*/

  0xaa, 0xaa,
  0xaa, 0x3a,
  0x3a, 0xaa,
  0xa3, 0xa3,
  0x3a, 0x3a,
  0x33, 0xa3,
  0xa3, 0x33,
  0x33, 0x33,
  0x33, 0x33,
  0x33, 0x53,
  0x53, 0x33,
  0x35, 0x35,
  0x53, 0x53,
  0x55, 0x35,
  0x35, 0x55,
  0x55, 0x55,
  0x55, 0x55,
  0x55, 0x85,
  0x85, 0x55,
  0x58, 0x58,
  0x85, 0x85,
  0x88, 0x58,
  0x58, 0x88,
  0x88, 0x88,
  0x88, 0x88,
  0x88, 0x18,
  0x18, 0x88,
  0x81, 0x81,
  0x18, 0x18,
  0x11, 0x81,
  0x81, 0x11,
  0x11, 0x11,
  0x11, 0x11,
  0x11, 0x61,
  0x61, 0x11,
  0x16, 0x16,
  0x61, 0x61,
  0x66, 0x16,
  0x16, 0x66,
  0x66, 0x66,
  0x66, 0x66,
  0x66, 0x96,
  0x96, 0x66,
  0x69, 0x69,
  0x96, 0x96,
  0x99, 0x69,
  0x69, 0x99,
  0x99, 0x99,
  0x99, 0x99,
  0x99, 0x29,
  0x29, 0x99,
  0x92, 0x92,
  0x29, 0x29,
  0x22, 0x92,
  0x92, 0x22,
  0x22, 0x22,
  0x22, 0x22,
  0x22, 0x42,
  0x42, 0x22,
  0x24, 0x24,
  0x42, 0x42,
  0x44, 0x24,
  0x24, 0x44,
  0x44, 0x44,
  0x44, 0x44,
  0x44, 0x74,
  0x74, 0x44,
  0x47, 0x47,
  0x74, 0x74,
  0x77, 0x47,
  0x47, 0x77,
  0x77, 0x77,
  0x77, 0x77,
  0x77, 0x07,
};


const lv_img_dsc_t LVGL_GRADIENT_image[2] = {
    { .header = { .cf = LV_IMG_CF_INDEXED_4BIT, .always_zero = 0, .w = 2, .h = 18, }, .data_size =  82, .data = header_indexed_16_map, },

    { .header = { .cf = LV_IMG_CF_INDEXED_4BIT, .always_zero = 0, .w = 4, .h = 74, }, .data_size = 212, .data = box_indexed_16_map, },
};
/* ****************************************************************************************** */
/* EXTERN FUNCTION DEFINITIONS                                                                */
/* ****************************************************************************************** */

/** De-code icons.
 *
 * Convert run length stored icons to normal 4bpp.
 * @note: This function should be called right after ICON_getP.
 *
 * @param[in]   pIconInfo   Pointer to icon's information.
 * @param[out]  pIcon       Pointer to the buffer.
 */
lv_res_t ICON_LVGL_runLength (lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc) {

  (void) decoder;

  lv_img_dsc_t* pImg = (lv_img_dsc_t*)dsc->src;

  /* RLE is only applied to user_encoded_0 images.
   *
   * Once RLE is applied, LittlevGL should treat image as alpha_4bit
   */
  if (pImg->header.cf == LV_IMG_CF_USER_ENCODED_0) {
    dsc->header.cf = LV_IMG_CF_ALPHA_4BIT;
  } else {
    return LV_RES_INV;
  }

  /**************************************/
  /* Make de-compression for RLE icons. */
  /**************************************/

  /* Initialize decodedImage structure */
  memset(decodedImageBitmap, 0, sizeof(decodedImageBitmap));
  decodedImage.header = pImg->header;
  decodedImage.data = decodedImageBitmap;

  /* Run length calculus */
  uint8_t*        pIcon           = &decodedImageBitmap[0];
  const uint8_t*  pData           = pImg->data;
  uint32_t        iconCount       = 0;

  uint32_t        nibble1_input   = 0;
  uint32_t        nibble2_input   = 0;

  uint32_t        zeroCount       = 0;
  bool          upperNibble     = true;
  bool          skip            = false;

  for (uint32_t i = 0; i < pImg->data_size; i++){
    nibble1_input = (*pData & 0xF0) >> 4;
    nibble2_input = (*pData & 0X0F);
    zeroCount = 0;

    if(skip == false){ //If last nibble_2 = 0
      //MSN
      if (nibble1_input == 0 ) {
        zeroCount = nibble2_input;
      }else{
        if(upperNibble == true){
        *pIcon |= (nibble1_input << 4);
        upperNibble = false;
        }else {
          *pIcon |= nibble1_input;
          pIcon++;
          iconCount++;
          upperNibble = true;
        }
      }
    }

    //LSB
    if((nibble2_input != 0) && (zeroCount == 0)){
      if(upperNibble == true){
        *pIcon |= (nibble2_input << 4);
        upperNibble = false;
      }else {
        *pIcon |= nibble2_input;
        pIcon++;
        iconCount++;
        upperNibble = true;
      }
      skip = false;
    }else if(zeroCount == 0){
      zeroCount = ((*(pData+1)) & 0xF0) >> 4;
      skip = true;
    }


    //Write zeros
    for(uint32_t j = 0; j < zeroCount;j++){
      if(upperNibble == true){
        *pIcon &= 0x0F;
        upperNibble = false;
      }else {
        *pIcon &= 0xF0;
        pIcon++;
        iconCount++;
        upperNibble = true;
      }
    }
    pData++;
  }

  decodedImage.data_size = iconCount;

  dsc->src = (const void*)&decodedImage;

  lv_img_decoder_built_in_open(decoder, dsc);

  return LV_RES_OK;
}


/* ****************************************************************************************** */
/* STATIC FUNCTION DEFINITIONS                                                                */
/* ****************************************************************************************** */

