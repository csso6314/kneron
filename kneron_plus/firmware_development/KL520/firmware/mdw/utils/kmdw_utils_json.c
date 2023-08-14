// #define DEBUG_PRINT

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "kmdw_utils_json.h"
#include "kmdw_console.h"

#ifdef DEBUG_PRINT
#define dbg_print(__format__, ...) kmdw_printf(__format__, ##__VA_ARGS__)
#else
#define dbg_print(__format__, ...)
#endif

// max json file tree depth
#define SCOPE_DEPTH_MAX 10

// max length to write a int value as a string
#define KMDW_UTILS_JSON_INT_MAX_STRING_LENGTH 11

// max length to write a float value as a string
#define KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH 20

static int s_scope_stack[SCOPE_DEPTH_MAX];
static int *s_scope_stack_ptr = &s_scope_stack[0];

#define push(s_scope_stack_ptr, n) (*((s_scope_stack_ptr)++) = (n))
#define top(s_scope_stack_ptr) (*(s_scope_stack_ptr - 1))
#define pop(s_scope_stack_ptr) (*--(s_scope_stack_ptr))

#define is_empty(s_scope_stack_ptr) ((s_scope_stack_ptr) == (&s_scope_stack[0]))
#define is_full(s_scope_stack_ptr) ((s_scope_stack_ptr) == (&s_scope_stack[SCOPE_DEPTH_MAX]))
#define clear(s_scope_stack_ptr) ((s_scope_stack_ptr) = (&s_scope_stack[0]))

/*
  === json buffer layout in DDR ===

                           base_addr -> |-------------------------------------|
       (reference position of offset)   |                                     |
                                        |                                     |
                                        |                                     |
                                        |           json file stream          |
                                        |                                     |
                                        |                                     |
                                        |                                     |
                                        |-------------------------------------|
                                        |       kmdw_utils_json_state_t       | sizeof(kmdw_utils_json_state_t)
                               size ->  |-------------------------------------|

*/

// If embedding the whole s_scope_stack and s_scope_stack_ptr to DDR, we can write multiple json file streams at the same time. Switching among them
// many times before all of them are written to DDR is possible.

static kmdw_utils_json_state_t *s_state = NULL; // point to the position of current json state

static const char *kmdw_utils_json_other_type_strings[] = { "true", "false", "null" };

static int _kmdw_utils_json_check_object_label(uint32_t object_label)
{
    if ('{' != *(char *)(s_state->base_addr + object_label))
    {
        kmdw_printf("[%s] error ! object_label %d is invalid\n", __func__, object_label);
        return KMDW_UTILS_JSON_ERROR_INVALID_OBJECT_LABEL;
    }
    else if (object_label != s_state->current_scope)
    {
        kmdw_printf("[%s] error ! invalid operation, object_label %d is not the current scope %d\n", __func__, object_label, s_state->current_scope);
        return KMDW_UTILS_JSON_ERROR_INVALID_OPERATON;
    }
    else
        return KMDW_UTILS_JSON_SUCCESS;
}

static int _kmdw_utils_json_check_array_label(uint32_t array_label)
{
    if ('[' != *(char *)(s_state->base_addr + array_label))
    {
        kmdw_printf("[%s] error ! array_label %d is invalid\n", __func__, array_label);
        return KMDW_UTILS_JSON_ERROR_INVALID_ARRAY_LABEL;
    }
    else if (array_label != s_state->current_scope)
    {
        kmdw_printf("[%s] error ! invalid operation, array_label %d is not the current scope %d\n", __func__, array_label, s_state->current_scope);
        return KMDW_UTILS_JSON_ERROR_INVALID_OPERATON;
    }
    else
        return KMDW_UTILS_JSON_SUCCESS;
}

static int _kmdw_utils_json_check_if_size_exceed_limit(uint32_t str_len)
{
    // Check if the buffer space is enough to write str_len byte to DDR
    if (s_state->offset + str_len + sizeof(kmdw_utils_json_state_t)> s_state->size)
    {
        kmdw_printf("[%s] error ! size exceed limit\n", __func__);
        return KMDW_UTILS_JSON_ERROR_SIZE_EXCEED_LIMIT;
    }

    return KMDW_UTILS_JSON_SUCCESS;
}

// symbol == '{', start of an object
// symbol == '[', start of an array
static int _kmdw_utils_json_append_comma_if_needed(char symbol)
{
    if (symbol != *(char *)(s_state->base_addr + s_state->offset - 1)) // Not the first key-value pair in the object or the array, need to add ','
    {
        int ret = _kmdw_utils_json_check_if_size_exceed_limit(1);
        if (KMDW_UTILS_JSON_SUCCESS != ret)
            return ret;

        *(char *)(s_state->base_addr + s_state->offset) = ',';
        s_state->offset += 1;
    }

    return KMDW_UTILS_JSON_SUCCESS;
}

// symbol == '{' add object to object
// symbol == '[' add array to object
static int _kmdw_utils_json_create_and_add_object_or_array_to_object(uint32_t src_label, char *key, int key_len, uint32_t *dst_label, char symbol)
{
    if (is_full(s_scope_stack_ptr))
        return KMDW_UTILS_JSON_ERROR_DEPTH_EXCEED_LIMIT;

    int ret = _kmdw_utils_json_check_object_label(src_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('{');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // 4: length of "":{ or "":[
    int str_len = 4 + key_len;

    ret = _kmdw_utils_json_check_if_size_exceed_limit(str_len + 1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it
    snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1, "\"%s\":%c", key, symbol);

    // Update dst_label for output and record current scope
    s_state->current_scope = *dst_label = s_state->offset + str_len - 1;

    // Push current scope to stack
    push(s_scope_stack_ptr, s_state->current_scope);

    s_state->offset += str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

static int _kmdw_utils_json_close_object_or_array(uint32_t label, char symbol)
{
    int ret;

    if ('}' == symbol)
    {
        ret = _kmdw_utils_json_check_object_label(label);
        if (KMDW_UTILS_JSON_SUCCESS != ret)
            return ret;
    }
    else if (']' == symbol)
    {
        ret = _kmdw_utils_json_check_array_label(label);
        if (KMDW_UTILS_JSON_SUCCESS != ret)
            return ret;
    }
    else
        return KMDW_UTILS_JSON_ERROR_INVALID_SYMBOL;

    ret = _kmdw_utils_json_check_if_size_exceed_limit(1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    *(char *)(s_state->base_addr + s_state->offset) = symbol;

    s_state->offset += 1;

    // Get previous scope from stack
    if (!is_empty(s_scope_stack_ptr))
    {
        // The popped one should be the same as current scope
        // pop means to close this scope
        if (s_state->current_scope != pop(s_scope_stack_ptr))
            return KMDW_UTILS_JSON_ERROR_OTHER;

        // The top one should be previous scope if current scope is not the first pushed scope (root)
        // don't pop it, this object or array label is still active, just access its value
        if (!is_empty(s_scope_stack_ptr))
            s_state->current_scope = top(s_scope_stack_ptr);
        // else
            // Should be the end of this JSON file stream !
    }
    else
        return KMDW_UTILS_JSON_ERROR_OTHER;

    return KMDW_UTILS_JSON_SUCCESS;
}

// kmdw_utils_json does not allocate DDR, only use the allocated DDR space by others
int kmdw_utils_json_init(uint32_t base_addr, uint32_t size)
{
    // Buffer size should be larger than the size of kmdw_utils_json_state_t, just a simple test
    if (size < sizeof(kmdw_utils_json_state_t))
        return KMDW_UTILS_JSON_ERROR_BUFFER_SIZE_TOO_SMALL;

    // Put kmdw_utils_json_state_t in the tail of the DDR block
    s_state = (kmdw_utils_json_state_t *)(base_addr + size - sizeof(kmdw_utils_json_state_t));
    s_state->base_addr = base_addr;
    s_state->size = size - sizeof(kmdw_utils_json_state_t); // cannot write to the space for kmdw_utils_json_state_t
    s_state->offset = 0;
    s_state->current_scope = 0;

    clear(s_scope_stack_ptr);

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_get_length(uint32_t *length)
{
    *length = s_state->offset;
    return KMDW_UTILS_JSON_SUCCESS;
}

// return object label
int kmdw_utils_json_create_object(uint32_t *object_label)
{
    if (is_full(s_scope_stack_ptr))
        return KMDW_UTILS_JSON_ERROR_DEPTH_EXCEED_LIMIT;

    int ret = _kmdw_utils_json_check_if_size_exceed_limit(1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    *(char *)(s_state->base_addr + s_state->offset) = '{';

    // Update object_label for output and record current scope
    s_state->current_scope = *object_label = s_state->offset;

    // Push current scope to stack
    push(s_scope_stack_ptr, s_state->current_scope);

    s_state->offset += 1;

    return KMDW_UTILS_JSON_SUCCESS;
}

// return object label
int kmdw_utils_json_create_and_add_object_to_object(uint32_t src_object_label, char *key, int key_len, uint32_t *dst_object_label)
{
    return _kmdw_utils_json_create_and_add_object_or_array_to_object(src_object_label, key, key_len, dst_object_label, '{');
}

// return array label
int kmdw_utils_json_create_and_add_array_to_object(uint32_t object_label, char *key, int key_len, uint32_t *array_label)
{
    return _kmdw_utils_json_create_and_add_object_or_array_to_object(object_label, key, key_len, array_label, '[');
}

int kmdw_utils_json_add_int_to_object(uint32_t object_label, char *key, int key_len, int int_val)
{
    int ret = _kmdw_utils_json_check_object_label(object_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('{');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // 3: length of "":
    int str_len = 3 + key_len;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it
    // snprintf to NULL in order to know how many bytes will be write to DDR
    int key_int_val_pair_str_len = snprintf(NULL, str_len + 1 + KMDW_UTILS_JSON_INT_MAX_STRING_LENGTH, "\"%s\":%d", key, int_val);

    ret = _kmdw_utils_json_check_if_size_exceed_limit(key_int_val_pair_str_len);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    key_int_val_pair_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1 + KMDW_UTILS_JSON_INT_MAX_STRING_LENGTH, "\"%s\":%d", key, int_val);

    s_state->offset += (uint32_t)key_int_val_pair_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_add_float_to_object(uint32_t object_label, char *key, int key_len, float float_val)
{
    int ret = _kmdw_utils_json_check_object_label(object_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('{');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // 3: length of "":
    int str_len = 3 + key_len;

    float int_part;
    modff(float_val, &int_part);
    int key_float_val_pair_str_len;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it
    // If the float number is actually an integer, limit its fractional part length
    // snprintf to NULL in order to know how many bytes will be write to DDR
    if (int_part == float_val)
        key_float_val_pair_str_len = snprintf(NULL, str_len + 1 + KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "\"%s\":%.1f", key, float_val);
    else
        key_float_val_pair_str_len = snprintf(NULL, str_len + 1 + KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "\"%s\":%g", key, float_val);

    ret = _kmdw_utils_json_check_if_size_exceed_limit(key_float_val_pair_str_len);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    if (int_part == float_val)
        key_float_val_pair_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1 + KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "\"%s\":%.1f", key, float_val);
    else
        key_float_val_pair_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1 + KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "\"%s\":%g", key, float_val);

    s_state->offset += (uint32_t)key_float_val_pair_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_add_string_to_object(uint32_t object_label, char *key, int key_len, char *str, int str_len)
{
    int ret = _kmdw_utils_json_check_object_label(object_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('{');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    int origin_str_len = str_len;

    // 5: length of "":""
    str_len += 5 + key_len;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it

    ret = _kmdw_utils_json_check_if_size_exceed_limit(str_len + 1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    int key_string_val_pair_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1, "\"%s\":\"%.*s\"", key, origin_str_len, str);

    s_state->offset += (uint32_t)key_string_val_pair_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_add_other_type_to_object(uint32_t object_label, char *key, int key_len, kmdw_utils_json_other_type type)
{
    if (KMDW_UTILS_JSON_TRUE > (int)type || KMDW_UTILS_JSON_NULL < (int)type)
        return KMDW_UTILS_JSON_ERROR_OTHER;

    int ret = _kmdw_utils_json_check_object_label(object_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('{');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // 3: length of "":
    int str_len = 3 + key_len;

    int key_other_type_val_pair_str_len = 0;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it
    str_len += strlen(kmdw_utils_json_other_type_strings[type]);

    ret = _kmdw_utils_json_check_if_size_exceed_limit(str_len + 1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    key_other_type_val_pair_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1, "\"%s\":%s", key, kmdw_utils_json_other_type_strings[type]);

    s_state->offset += (uint32_t)key_other_type_val_pair_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_close_object(uint32_t object_label)
{
    return _kmdw_utils_json_close_object_or_array(object_label, '}');
}

// return array label
int kmdw_utils_json_create_array(uint32_t *array_label)
{
    if (is_full(s_scope_stack_ptr))
        return KMDW_UTILS_JSON_ERROR_DEPTH_EXCEED_LIMIT;

    int ret = _kmdw_utils_json_check_if_size_exceed_limit(1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    *(char *)(s_state->base_addr + s_state->offset) = '[';

    // Update array_label for output and record current scope
    s_state->current_scope = *array_label = s_state->offset;

    // Push current scope to stack
    push(s_scope_stack_ptr, s_state->current_scope);

    s_state->offset += 1;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_create_and_append_array_to_array(uint32_t src_array_label, uint32_t *dst_array_label)
{
    if (is_full(s_scope_stack_ptr))
        return KMDW_UTILS_JSON_ERROR_DEPTH_EXCEED_LIMIT;

    int ret = _kmdw_utils_json_check_array_label(src_array_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('[');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    *(char *)(s_state->base_addr + s_state->offset) = '[';

    // Update dst_array_label for output and record current scope
    s_state->current_scope = *dst_array_label = s_state->offset;

    // Push current scope to stack
    push(s_scope_stack_ptr, s_state->current_scope);

    s_state->offset += 1;

    return 0;
}

int kmdw_utils_json_create_and_append_object_to_array(uint32_t array_label, uint32_t *object_label)
{
    int ret = _kmdw_utils_json_check_array_label(array_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('[');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    return kmdw_utils_json_create_object(object_label);
}

int kmdw_utils_json_append_int_to_array(uint32_t array_label, int int_val)
{
    int ret = _kmdw_utils_json_check_array_label(array_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('[');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // snprintf to NULL in order to know how many bytes will be write to DDR
    int int_val_str_len = snprintf(NULL, KMDW_UTILS_JSON_INT_MAX_STRING_LENGTH, "%d", int_val);

    ret = _kmdw_utils_json_check_if_size_exceed_limit(int_val_str_len);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    int_val_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), KMDW_UTILS_JSON_INT_MAX_STRING_LENGTH, "%d", int_val);

    s_state->offset += (uint32_t)int_val_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_ints_to_array(uint32_t array_label, int *int_arr, int count)
{
    int status = KMDW_UTILS_JSON_SUCCESS;

    for (int i = 0; i < count; i++)
    {
        status = kmdw_utils_json_append_int_to_array(array_label, int_arr[i]);
        if (KMDW_UTILS_JSON_SUCCESS != status)
            return status;
    }

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_float_to_array(uint32_t array_label, float float_val)
{
    int ret = _kmdw_utils_json_check_array_label(array_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('[');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    float int_part;
    modff(float_val, &int_part);
    int float_val_str_len;

    // If the float number is actually an integer, limit its fractional part length
    // snprintf to NULL in order to know how many bytes will be write to DDR
    if (int_part == float_val)
        float_val_str_len = snprintf(NULL, KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "%.1f", float_val);
    else
        float_val_str_len = snprintf(NULL, KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "%g", float_val);

    ret = _kmdw_utils_json_check_if_size_exceed_limit(float_val_str_len);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    if (int_part == float_val)
        float_val_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "%.1f", float_val);
    else
        float_val_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), KMDW_UTILS_JSON_FLOAT_MAX_STRING_LENGTH, "%g", float_val);

    s_state->offset += (uint32_t)float_val_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_floats_to_array(uint32_t array_label, float *float_arr, int count)
{
    int status = KMDW_UTILS_JSON_SUCCESS;

    for (int i = 0; i < count; i++)
    {
        status = kmdw_utils_json_append_float_to_array(array_label, float_arr[i]);
        if (KMDW_UTILS_JSON_SUCCESS != status)
            return status;
    }

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_string_to_array(uint32_t array_label, char *str, int str_len)
{
    int ret = _kmdw_utils_json_check_array_label(array_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('[');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    int origin_str_len = str_len;

    // 2: length of ""
    str_len += 2;

    ret = _kmdw_utils_json_check_if_size_exceed_limit(str_len + 1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it
    snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1, "\"%.*s\"", origin_str_len, str);

    s_state->offset += (uint32_t)str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_strings_to_array(uint32_t array_label, char **str_arr, int count)
{
    int status = KMDW_UTILS_JSON_SUCCESS;

    for (int i = 0; i < count; i++)
    {
        status = kmdw_utils_json_append_string_to_array(array_label, str_arr[i], strlen(str_arr[i]));
        if (KMDW_UTILS_JSON_SUCCESS != status)
            return status;
    }

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_other_type_to_array(uint32_t array_label, kmdw_utils_json_other_type type)
{
    if (KMDW_UTILS_JSON_TRUE > (int)type || KMDW_UTILS_JSON_NULL < (int)type)
        return KMDW_UTILS_JSON_ERROR_OTHER;

    int ret = _kmdw_utils_json_check_array_label(array_label);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    ret = _kmdw_utils_json_append_comma_if_needed('[');
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    int str_len = 0;

    int other_type_val_str_len = 0;

    // Since a terminating null character is automatically appended after the content written by snprintf,
    // reserve the space for it
    str_len += strlen(kmdw_utils_json_other_type_strings[type]);

    ret = _kmdw_utils_json_check_if_size_exceed_limit(str_len + 1);
    if (KMDW_UTILS_JSON_SUCCESS != ret)
        return ret;

    other_type_val_str_len = snprintf((char *)(s_state->base_addr + s_state->offset), str_len + 1, "%s", kmdw_utils_json_other_type_strings[type]);

    s_state->offset += (uint32_t)other_type_val_str_len;

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_other_types_to_array(uint32_t array_label, kmdw_utils_json_other_type type, int count)
{
    int status = KMDW_UTILS_JSON_SUCCESS;

    for (int i = 0; i < count; i++)
    {
        status = kmdw_utils_json_append_other_type_to_array(array_label, type);
        if (KMDW_UTILS_JSON_SUCCESS != status)
            return status;
    }

    return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_append_values_to_array(uint32_t array_label, char *fmt, ...)
{
    va_list va_args;
    int d = 0;
    float f = 0.0f;
    char *s = NULL;

    va_start(va_args, fmt);

    while (*fmt != '\0')
    {
        switch(*fmt)
        {
        case '%':
            ++fmt;
            switch(*fmt)
            {
            case 'd': // get %d
                d = (int)va_arg(va_args, int);
                kmdw_utils_json_append_int_to_array(array_label, d);
                break;
            case 'f': // get %f
                // To avoid warning of promoting float to double
                f = (float)va_arg(va_args, double);
                kmdw_utils_json_append_float_to_array(array_label, f);
                break;
            case 's': // get %s
                s = va_arg(va_args, char *);
                kmdw_utils_json_append_string_to_array(array_label, s, strlen(s));
                break;
            case 'b': // get true, false, null (specific type)
                d = va_arg(va_args, int);
                kmdw_utils_json_append_other_type_to_array(array_label, (kmdw_utils_json_other_type)d);
                break;
            }
            ++fmt;
            break;
        default:
            break;
        }
    }

    va_end(va_args);

	return KMDW_UTILS_JSON_SUCCESS;
}

int kmdw_utils_json_close_array(uint32_t array_label)
{
    return _kmdw_utils_json_close_object_or_array(array_label, ']');
}
