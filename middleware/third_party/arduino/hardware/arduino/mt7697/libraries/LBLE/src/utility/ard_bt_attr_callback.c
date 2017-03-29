#include "ard_bt_attr_callback.h"

// table for user-registered user data and its allocation cursor
static void* g_userdata[ARD_BT_MAX_ATTR_CALLBACK_NUM] = {0};
static uint32_t g_callback_cursor = 0;

// macro to define trampoline functions
#define DEFINE_TRAMPOLINE_FUNC(INDEX) static uint32_t attribute_callback_##INDEX(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset){\
	return ard_bt_callback_trampoline(rw, handle, data, size, offset, g_userdata[INDEX]);\
}

// now we generate the table - it MUST have the same number as ARD_BT_MAX_ATTR_CALLBACK_NUM
DEFINE_TRAMPOLINE_FUNC(0)
DEFINE_TRAMPOLINE_FUNC(1)
DEFINE_TRAMPOLINE_FUNC(2)
DEFINE_TRAMPOLINE_FUNC(3)
DEFINE_TRAMPOLINE_FUNC(4)
DEFINE_TRAMPOLINE_FUNC(5)
DEFINE_TRAMPOLINE_FUNC(6)
DEFINE_TRAMPOLINE_FUNC(7)
DEFINE_TRAMPOLINE_FUNC(8)
DEFINE_TRAMPOLINE_FUNC(9)
DEFINE_TRAMPOLINE_FUNC(10)
DEFINE_TRAMPOLINE_FUNC(11)
DEFINE_TRAMPOLINE_FUNC(12)
DEFINE_TRAMPOLINE_FUNC(13)
DEFINE_TRAMPOLINE_FUNC(14)
DEFINE_TRAMPOLINE_FUNC(15)
DEFINE_TRAMPOLINE_FUNC(16)
DEFINE_TRAMPOLINE_FUNC(17)
DEFINE_TRAMPOLINE_FUNC(18)
DEFINE_TRAMPOLINE_FUNC(19)
DEFINE_TRAMPOLINE_FUNC(20)
DEFINE_TRAMPOLINE_FUNC(21)
DEFINE_TRAMPOLINE_FUNC(22)
DEFINE_TRAMPOLINE_FUNC(23)
DEFINE_TRAMPOLINE_FUNC(24)
DEFINE_TRAMPOLINE_FUNC(25)
DEFINE_TRAMPOLINE_FUNC(26)
DEFINE_TRAMPOLINE_FUNC(27)
DEFINE_TRAMPOLINE_FUNC(28)
DEFINE_TRAMPOLINE_FUNC(29)

// now insert these TRAMPOLINE functions into a static table
static void* g_trampoline_callbacks[ARD_BT_MAX_ATTR_CALLBACK_NUM] = 
{
	attribute_callback_0,
	attribute_callback_1,
	attribute_callback_2,
	attribute_callback_3,
	attribute_callback_4,
	attribute_callback_5,
	attribute_callback_6,
	attribute_callback_7,
	attribute_callback_8,
	attribute_callback_9,
	attribute_callback_10,
	attribute_callback_11,
	attribute_callback_12,
	attribute_callback_13,
	attribute_callback_14,
	attribute_callback_15,
	attribute_callback_16,
	attribute_callback_17,
	attribute_callback_18,
	attribute_callback_19,
	attribute_callback_20,
	attribute_callback_21,
	attribute_callback_22,
	attribute_callback_23,
	attribute_callback_24,
	attribute_callback_25,
	attribute_callback_26,
	attribute_callback_27,
	attribute_callback_28,
	attribute_callback_29,
};

bt_gatts_rec_callback_t ard_bt_alloc_callback_slot(void* user_data)
{
	// note that currently we don't have a mechanism to 
	// free allocated user callback slots.
	if(g_callback_cursor >= ARD_BT_MAX_ATTR_CALLBACK_NUM)
	{
		return NULL;
	}

	g_userdata[g_callback_cursor] = user_data;
	return g_trampoline_callbacks[g_callback_cursor++];
}
