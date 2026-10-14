#include <unity.h>
#include <string.h>
#include "src/cnc.h"

static void ensure_initialized(void)
{
	static bool initialized;
	if (!initialized)
	{
		cnc_init();
		system_menu_init();
		initialized = true;
	}

	// every test starts from a clean menu state
	g_system_menu.flags = 0;
	g_system_menu.current_multiplier = 0;
}

void setUp(void)
{
	ensure_initialized();
}

void tearDown(void)
{
}

static const system_menu_item_t *find_menu_item(uint8_t menu_id, const char *label)
{
	MENU_LOOP(g_system_menu.menu_entry, page)
	{
		if (page->menu_id == menu_id)
		{
			for (system_menu_index_t *index = page->items_index; index != NULL; index = index->next)
			{
				if (strcmp(index->menu_item->label, label) == 0)
				{
					return index->menu_item;
				}
			}
			return NULL;
		}
	}
	return NULL;
}

// runs the action sequence that enters edit+modify mode and decrements one step
static float edit_and_decrement(system_menu_item_t *item)
{
	system_menu_action_edit(SYSTEM_MENU_ACTION_SELECT, item);
	system_menu_action_edit(SYSTEM_MENU_ACTION_SELECT, item);
	system_menu_action_edit(SYSTEM_MENU_ACTION_PREV, item);
	return *((float *)item->argptr);
}

static void test_io_menu_step_and_dir_invert_are_bound_to_distinct_settings(void)
{
	system_menu_item_t *step = (system_menu_item_t *)find_menu_item(SYSTEM_MENU_ID_IO_SETTIGNS, STR_STEP_INV);
	system_menu_item_t *dir = (system_menu_item_t *)find_menu_item(SYSTEM_MENU_ID_IO_SETTIGNS, STR_DIR_INV);

	TEST_ASSERT_NOT_NULL_MESSAGE(step, "`Step inv` item was not found in the IO settings menu");
	TEST_ASSERT_NOT_NULL_MESSAGE(dir, "`Dir inv` item was not found in the IO settings menu");
	TEST_ASSERT_EQUAL_PTR_MESSAGE((void *)&g_settings.rt.step_invert_mask, step->argptr, "`Step inv` is not bound to step_invert_mask");
	TEST_ASSERT_EQUAL_PTR_MESSAGE((void *)&g_settings.rt.dir_invert_mask, dir->argptr, "`Dir inv` is not bound to dir_invert_mask");
}

static void test_negative_float_editing_is_gated_to_signed_settings(void)
{
	system_menu_item_t *g64 = (system_menu_item_t *)find_menu_item(SYSTEM_MENU_ID_MISC_SETTINGS, STR_G64_FACT);
	TEST_ASSERT_NOT_NULL_MESSAGE(g64, "`G64 fact` item was not found in the misc settings menu");
	TEST_ASSERT_EQUAL_UINT8(VAR_TYPE_FLOAT_SIGNED, (uint8_t)VARG_CONST(g64->action_arg));

	g_settings.g64_angle_factor = 0.0f;
	TEST_ASSERT_TRUE_MESSAGE(edit_and_decrement(g64) < 0.0f, "a signed float setting could not be driven below zero");

	system_menu_item_t *arc = (system_menu_item_t *)find_menu_item(SYSTEM_MENU_ID_MISC_SETTINGS, STR_ARC_TOL);
	TEST_ASSERT_NOT_NULL_MESSAGE(arc, "`Arc tol` item was not found in the misc settings menu");
	TEST_ASSERT_EQUAL_UINT8(VAR_TYPE_FLOAT, (uint8_t)VARG_CONST(arc->action_arg));

	g_settings.arc_tolerance = 0.0f;
	TEST_ASSERT_TRUE_MESSAGE(edit_and_decrement(arc) >= 0.0f, "an unsigned float setting was allowed to go below zero");
}

#ifdef ENABLE_LASER_PPI
static void test_laser_ppi_setting_id_is_bound_to_laser_ppi(void)
{
	bool found = false;
	uint8_t count = settings_count();
	for (uint8_t i = 0; i < count; i++)
	{
		setting_id_t setting = {0};
		rom_memcpy(&setting, &g_settings_id_table[i], sizeof(setting_id_t));
		if (setting.id == 33)
		{
			found = true;
			TEST_ASSERT_EQUAL_PTR((void *)&g_settings.laser_ppi, setting.memptr);
			TEST_ASSERT_EQUAL_UINT8(SETTING_TYPE_UINT16, SETTING_TYPE_MASK(setting.type));
		}
	}
	TEST_ASSERT_TRUE_MESSAGE(found, "setting id 33 was not found in the settings table");
}
#endif

#if defined(ENABLE_BACKLASH_COMPENSATION) && (AXIS_COUNT > 5)
static void test_c_axis_backlash_menu_item_is_present(void)
{
	TEST_ASSERT_NOT_NULL_MESSAGE(find_menu_item(SYSTEM_MENU_ID_STEPPER_SETTINGS, STR_BACKLASH("C")), "`C backlash` item was not found in the axis settings menu");
}
#endif

#ifdef H_MAPPING_EEPROM_STORE_ENABLED
static void test_hmap_menu_items_have_distinct_labels(void)
{
	const struct
	{
		const char *label;
		void *argptr;
	} expected[] = {
		{STR_HMAP_X_START, &g_settings.hmap_x},
		{STR_HMAP_Y_START, &g_settings.hmap_y},
		{STR_HMAP_X_SIZE, &g_settings.hmap_x_offset},
		{STR_HMAP_Y_SIZE, &g_settings.hmap_y_offset},
	};

	for (uint8_t i = 0; i < (uint8_t)(sizeof(expected) / sizeof(expected[0])); i++)
	{
		const system_menu_item_t *item = find_menu_item(SYSTEM_MENU_ID_MISC_SETTINGS, expected[i].label);
		TEST_ASSERT_NOT_NULL_MESSAGE(item, expected[i].label);
		TEST_ASSERT_EQUAL_PTR(expected[i].argptr, item->argptr);
	}
}
#endif

#if ENCODERS > 0
static void test_encoder_pulse_invert_default_is_independent_from_dir(void)
{
	settings_reset(false);
	TEST_ASSERT_EQUAL_UINT8(DEFAULT_ENCODERS_PULSE_INV_MASK, g_settings.rt.encoders_pulse_invert_mask);
	TEST_ASSERT_EQUAL_UINT8(DEFAULT_ENCODERS_DIR_INV_MASK, g_settings.rt.encoders_dir_invert_mask);
}
#endif

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_io_menu_step_and_dir_invert_are_bound_to_distinct_settings);
	RUN_TEST(test_negative_float_editing_is_gated_to_signed_settings);
#if ENCODERS > 0
	RUN_TEST(test_encoder_pulse_invert_default_is_independent_from_dir);
#endif
#ifdef ENABLE_LASER_PPI
	RUN_TEST(test_laser_ppi_setting_id_is_bound_to_laser_ppi);
#endif
#if defined(ENABLE_BACKLASH_COMPENSATION) && (AXIS_COUNT > 5)
	RUN_TEST(test_c_axis_backlash_menu_item_is_present);
#endif
#ifdef H_MAPPING_EEPROM_STORE_ENABLED
	RUN_TEST(test_hmap_menu_items_have_distinct_labels);
#endif
	return UNITY_END();
}
