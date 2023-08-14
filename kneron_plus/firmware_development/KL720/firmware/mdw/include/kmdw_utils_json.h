/**
 * @file        kmdw_utils_json.h
 * @brief       write json file stream to the specified DDR space
 *
 * @copyright   Copyright (c) 2022 Kneron Inc. All rights reserved.
 */

#ifndef __KMDW_UTILS_JSON_H__
#define __KMDW_UTILS_JSON_H__

#include <stdint.h>
#include <stdarg.h>
#include "project.h"

/**
 * @brief return code of most kmdw_utils_json APIs.
 */
enum KMDW_UTILS_JSON_RETURN_CODE
{
    KMDW_UTILS_JSON_SUCCESS = 0,
    KMDW_UTILS_JSON_ERROR_INVALID_OBJECT_LABEL = -1,
    KMDW_UTILS_JSON_ERROR_INVALID_ARRAY_LABEL = -2,
    KMDW_UTILS_JSON_ERROR_INVALID_OPERATON = -3,
    KMDW_UTILS_JSON_ERROR_INVALID_SYMBOL = -4,
    KMDW_UTILS_JSON_ERROR_DEPTH_EXCEED_LIMIT = -5,
    KMDW_UTILS_JSON_ERROR_SIZE_EXCEED_LIMIT = -6,
    KMDW_UTILS_JSON_ERROR_BUFFER_SIZE_TOO_SMALL = -7,
    KMDW_UTILS_JSON_ERROR_OTHER = -8,
};

/**
 * @brief other type of json value.
 */
typedef enum
{
    KMDW_UTILS_JSON_TRUE = 0,
    KMDW_UTILS_JSON_FALSE = 1,
    KMDW_UTILS_JSON_NULL = 2,
} kmdw_utils_json_other_type;

/**
 * @brief record the state of writing json file stream.
 */
typedef struct {
    uint32_t base_addr;     /**< base DDR address to write json file stream*/
    uint32_t size;          /**< DDR space size */
    uint32_t offset;        /**< starts from base_addr */
    uint32_t current_scope; /**< object + array depth */
} __attribute__((aligned(4))) kmdw_utils_json_state_t;

/**
 * @brief initialize kmdw_utils_json
 *
 * @param[in] base_addr starting address of the DDR to write json file stream
 * @param[in] size DDR space size
 * @return error code
 *
 */
int kmdw_utils_json_init(uint32_t base_addr, uint32_t size);

/**
 * @brief get json file stream length
 *
 * @param[out] length length of json file stream
 * @return error code
 *
 */
int kmdw_utils_json_get_length(uint32_t *length);

// ------------------------------------------ json object ------------------------------------------

// return object label
// actually only used with the root scope; ie, the json file stream starts with '{'
/**
 * @brief create object
 *
 * @param[out] object_label returned object label
 * @return error code
 *
 */
int kmdw_utils_json_create_object(uint32_t *object_label);

/**
 * @brief create object and add it to another object
 *
 * @param[in] src_object_label label of the object to add to
 * @param[in] key object key
 * @param[in] key_len key length
 * @param[out] dst_object_label label of the created object
 * @return error code
 *
 */
int kmdw_utils_json_create_and_add_object_to_object(uint32_t src_object_label, char *key, int key_len, uint32_t *dst_object_label);

/**
 * @brief create array and add it to an object
 *
 * @param[in] object_label label of the object to add to
 * @param[in] key object key
 * @param[in] key_len key length
 * @param[out] array_label label of the created array
 * @return error code
 *
 */
int kmdw_utils_json_create_and_add_array_to_object(uint32_t object_label, char *key, int key_len, uint32_t *array_label);

/**
 * @brief add an integer to object
 *
 * @param[in] object_label label of the object to add to
 * @param[in] key object key
 * @param[in] key_len key length
 * @param[in] int_val an integer
 * @return error code
 *
 */
int kmdw_utils_json_add_int_to_object(uint32_t object_label, char *key, int key_len, int int_val);

/**
 * @brief add a float to object
 *
 * @param[in] object_label label of the object to add to
 * @param[in] key object key
 * @param[in] key_len key length
 * @param[in] float_val a float
 * @return error code
 *
 */
int kmdw_utils_json_add_float_to_object(uint32_t object_label, char *key, int key_len, float float_val);

/**
 * @brief add a string to object
 *
 * @param[in] object_label label of the object to add to
 * @param[in] key object key
 * @param[in] key_len key length
 * @param[in] str a string
 * @param[in] str_len length of the string
 * @return error code
 *
 */
int kmdw_utils_json_add_string_to_object(uint32_t object_label, char *key, int key_len, char *str, int str_len);

/**
 * @brief add an other type value (true, false, null) to object
 *
 * @param[in] object_label label of the object to add to
 * @param[in] key object key
 * @param[in] key_len key length
 * @param[in] type an other type value
 * @return error code
 *
 */
int kmdw_utils_json_add_other_type_to_object(uint32_t object_label, char *key, int key_len, kmdw_utils_json_other_type type);

/**
 * @brief close object
 *
 * @param[out] object_label label of the object to be closed
 * @return error code
 *
 */
int kmdw_utils_json_close_object(uint32_t object_label);

// ------------------------------------------ json array ------------------------------------------

// return array label
// actually only used with the root scope; ie, the json file stream starts with '['
/**
 * @brief create array
 *
 * @param[out] array_label returned array label
 * @return error code
 *
 */
int kmdw_utils_json_create_array(uint32_t *array_label);

/**
 * @brief create array and append it to another array
 *
 * @param[in] src_array_label label of the array to append to
 * @param[out] dst_array_label label of the created array
 * @return error code
 *
 */
int kmdw_utils_json_create_and_append_array_to_array(uint32_t src_array_label, uint32_t *dst_array_label);

/**
 * @brief create object and append it to an array
 *
 * @param[in] array_label label of the array to append to
 * @param[out] object_label label of the created object
 * @return error code
 *
 */
int kmdw_utils_json_create_and_append_object_to_array(uint32_t array_label, uint32_t *object_label);

/**
 * @brief append an integer to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] int_val an integer
 * @return error code
 *
 */
int kmdw_utils_json_append_int_to_array(uint32_t array_label, int int_val);

/**
 * @brief append several integers to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] int_arr an integer array
 * @param[in] count number of the integer
 * @return error code
 *
 */
int kmdw_utils_json_append_ints_to_array(uint32_t array_label, int *int_arr, int count);

/**
 * @brief append an float to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] float_val an float
 * @return error code
 *
 */
int kmdw_utils_json_append_float_to_array(uint32_t array_label, float float_val);

/**
 * @brief append several floats to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] float_arr an float array
 * @param[in] count number of the float
 * @return error code
 *
 */
int kmdw_utils_json_append_floats_to_array(uint32_t array_label, float *float_arr, int count);

/**
 * @brief append a string to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] str a string
 * @param[in] str_len length of the string
 * @return error code
 *
 */
int kmdw_utils_json_append_string_to_array(uint32_t array_label, char *str, int str_len);

/**
 * @brief append several strings to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] str_arr a string
 * @param[in] count number of the string
 * @return error code
 *
 */
int kmdw_utils_json_append_strings_to_array(uint32_t array_label, char **str_arr, int count);

/**
 * @brief append an other type value (true, false, null) to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] type an other type value
 * @return error code
 *
 */
int kmdw_utils_json_append_other_type_to_array(uint32_t array_label, kmdw_utils_json_other_type type);

/**
 * @brief append several other type values (true, false, null) to array
 *
 * @param[in] array_label label of the array to append to
 * @param[in] type an other type
 * @param[in] count number of the other type values
 * @return error code
 *
 */
int kmdw_utils_json_append_other_types_to_array(uint32_t array_label, kmdw_utils_json_other_type type, int count);

/**
 * @brief append several values to array, specify the types of value as used in printf, note that other type (true, false, null) is specified as %b
 *
 * @param[in] array_label label of the array to append to
 * @param[in] fmt input format of the values: %d - integer, %f: float, %s: string, %b: other type
 * @return error code
 *
 */
int kmdw_utils_json_append_values_to_array(uint32_t array_label, char *fmt, ...);

/**
 * @brief close array
 *
 * @param[out] array_label label of the array to be closed
 * @return error code
 *
 */
int kmdw_utils_json_close_array(uint32_t array_label);

// --------------------------------------------------------------------------------------------------------------------

#endif
