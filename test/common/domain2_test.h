#ifndef DOMAIN2_TEST_H
#define DOMAIN2_TEST_H

#include "grbl_test.h"
#include <math.h>
#include <stdlib.h>

#define D2_POSITION_TOLERANCE 0.06f
#define D2_PATH_TOLERANCE 0.18f
#define D2_MAX_SAMPLES 512U
#define D2_MOTION_TIMEOUT_MS 15000U

typedef struct
{
	uint32_t time_ms;
	float x;
	float y;
	float z;
	float feed;
	char state[12];
} d2_sample_t;

typedef struct
{
	d2_sample_t samples[D2_MAX_SAMPLES];
	size_t count;
	bool saw_run;
	bool saw_idle;
} d2_trace_t;

static __attribute__((unused)) bool d2_parse_status(const char *text, d2_sample_t *sample)
{
	const char *begin = strrchr(text, '<');
	if (!begin)
	{
		return false;
	}
	const char *separator = strchr(begin, '|');
	if (!separator || separator - begin < 2 || separator - begin >= (ptrdiff_t)sizeof(sample->state))
	{
		return false;
	}
	memcpy(sample->state, begin + 1, (size_t)(separator - begin - 1));
	sample->state[separator - begin - 1] = '\0';

	const char *position = strstr(begin, "MPos:");
	if (!position || sscanf(position + 5, "%f,%f,%f", &sample->x, &sample->y, &sample->z) != 3)
	{
		return false;
	}
	sample->feed = 0.0f;
	const char *feed = strstr(begin, "|FS:");
	if (feed)
	{
		(void)sscanf(feed + 4, "%f", &sample->feed);
	}
	sample->time_ms = mcu_millis();
	return true;
}

static __attribute__((unused)) bool d2_query_status(d2_sample_t *sample)
{
	grbl_test_clear_output();
	if (!mcu_unit_test_inject("?") || !grbl_test_wait_for(">", 500U))
	{
		return false;
	}
	grbl_test_snapshot();
	bool parsed = d2_parse_status(grbl_test_transcript, sample);
	mcu_unit_test_advance_time(5000U);
	return parsed;
}

static __attribute__((unused)) void d2_assert_position(const d2_sample_t *position, float x, float y, float z, float tolerance)
{
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tolerance, x, position->x, "unexpected machine X");
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tolerance, y, position->y, "unexpected machine Y");
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(tolerance, z, position->z, "unexpected machine Z");
}

static __attribute__((unused)) d2_sample_t d2_position(void)
{
	d2_sample_t position = {0};
	bool parsed = false;
	for (uint8_t attempt = 0; attempt < 5U && !parsed; ++attempt)
	{
		parsed = d2_query_status(&position);
		sched_yield();
	}
	TEST_ASSERT_TRUE_MESSAGE(parsed, "could not parse MPos status report");
	return position;
}

static __attribute__((unused)) void d2_wait_idle_timeout(uint32_t timeout_ms)
{
	uint32_t start = mcu_millis();
	uint32_t idle_since = 0U;
	d2_sample_t status;
	do
	{
		if (d2_query_status(&status) && !strcmp(status.state, "Idle"))
		{
			if (!idle_since)
				idle_since = mcu_millis();
			if ((uint32_t)(mcu_millis() - idle_since) >= 20U)
				return;
		}
		else
			idle_since = 0U;
		sched_yield();
	} while ((uint32_t)(mcu_millis() - start) < timeout_ms);
	TEST_FAIL_MESSAGE("motion did not reach Idle before timeout");
}

static __attribute__((unused)) bool d2_same_position(const d2_sample_t *a, const d2_sample_t *b)
{
	return fabsf(a->x - b->x) < 0.0005f && fabsf(a->y - b->y) < 0.0005f && fabsf(a->z - b->z) < 0.0005f;
}

static __attribute__((unused)) void d2_trace_motion(const char *command, d2_trace_t *trace)
{
	memset(trace, 0, sizeof(*trace));
	d2_sample_t initial = {0};
	TEST_ASSERT_TRUE_MESSAGE(d2_query_status(&initial), "could not capture position before motion");
	grbl_test_command_ok(command);
	uint32_t start = mcu_millis();
	d2_sample_t previous = {0};
	bool have_previous = false;
	do
	{
		d2_sample_t current;
		if (d2_query_status(&current))
		{
			trace->saw_run |= !strcmp(current.state, "Run");
			if (!have_previous || !d2_same_position(&previous, &current) || strcmp(previous.state, current.state))
			{
				if (trace->count < D2_MAX_SAMPLES)
				{
					trace->samples[trace->count++] = current;
				}
				previous = current;
				have_previous = true;
			}
			bool position_changed = !d2_same_position(&initial, &current);
			if (!strcmp(current.state, "Idle") && (trace->saw_run || position_changed))
			{
				trace->saw_idle = true;
				return;
			}
		}
		sched_yield();
	} while ((uint32_t)(mcu_millis() - start) < D2_MOTION_TIMEOUT_MS);
	TEST_FAIL_MESSAGE("timed out while sampling motion");
}

static __attribute__((unused)) void d2_assert_trace_endpoint(const d2_trace_t *trace, float x, float y, float z)
{
	TEST_ASSERT_TRUE_MESSAGE(trace->saw_idle, "trace never reached Idle");
	TEST_ASSERT_GREATER_THAN_MESSAGE(0U, trace->count, "trace contains no status samples");
	d2_assert_position(&trace->samples[trace->count - 1U], x, y, z, D2_POSITION_TOLERANCE);
}

static __attribute__((unused)) void d2_assert_line(const d2_trace_t *trace, d2_sample_t start, d2_sample_t end)
{
	float vx = end.x - start.x, vy = end.y - start.y, vz = end.z - start.z;
	float length2 = vx * vx + vy * vy + vz * vz;
	TEST_ASSERT_TRUE_MESSAGE(length2 > 0.0f, "line validator requires a non-zero move");
	float prior = -D2_PATH_TOLERANCE;
	for (size_t i = 0; i < trace->count; ++i)
	{
		float wx = trace->samples[i].x - start.x;
		float wy = trace->samples[i].y - start.y;
		float wz = trace->samples[i].z - start.z;
		float projection = (wx * vx + wy * vy + wz * vz) / length2;
		float px = start.x + projection * vx;
		float py = start.y + projection * vy;
		float pz = start.z + projection * vz;
		float distance = sqrtf((trace->samples[i].x - px) * (trace->samples[i].x - px) +
							   (trace->samples[i].y - py) * (trace->samples[i].y - py) +
							   (trace->samples[i].z - pz) * (trace->samples[i].z - pz));
		TEST_ASSERT_TRUE_MESSAGE(distance <= D2_PATH_TOLERANCE, "linear interpolation departed from commanded line");
		TEST_ASSERT_TRUE_MESSAGE(projection >= prior - 0.02f, "linear motion reversed progress");
		TEST_ASSERT_TRUE_MESSAGE(projection >= -0.02f && projection <= 1.02f, "linear motion overshot segment");
		prior = projection;
	}
}

static __attribute__((unused)) void d2_assert_arc(const d2_trace_t *trace, int axis_u, int axis_v, int linear_axis,
												  float center_u, float center_v, float radius, bool clockwise, float linear_start, float linear_end)
{
	float previous_angle = 0.0f;
	float total_angle = 0.0f;
	bool have_angle = false;
	for (size_t i = 0; i < trace->count; ++i)
	{
		const float p[3] = {trace->samples[i].x, trace->samples[i].y, trace->samples[i].z};
		float du = p[axis_u] - center_u, dv = p[axis_v] - center_v;
		float r = sqrtf(du * du + dv * dv);
		TEST_ASSERT_FLOAT_WITHIN_MESSAGE(D2_PATH_TOLERANCE, radius, r, "arc radial error exceeds tolerance");
		float angle = atan2f(dv, du);
		if (have_angle)
		{
			float delta = angle - previous_angle;
			while (delta > (float)M_PI)
				delta -= 2.0f * (float)M_PI;
			while (delta < -(float)M_PI)
				delta += 2.0f * (float)M_PI;
			if (fabsf(delta) > 0.001f)
			{
				TEST_ASSERT_TRUE_MESSAGE(clockwise ? delta <= 0.03f : delta >= -0.03f, "arc traveled in wrong angular direction");
				total_angle += delta;
			}
		}
		previous_angle = angle;
		have_angle = true;
		if (linear_axis >= 0)
		{
			float lo = fminf(linear_start, linear_end) - D2_PATH_TOLERANCE;
			float hi = fmaxf(linear_start, linear_end) + D2_PATH_TOLERANCE;
			TEST_ASSERT_TRUE_MESSAGE(p[linear_axis] >= lo && p[linear_axis] <= hi, "helix linear axis overshot");
		}
	}
	TEST_ASSERT_TRUE_MESSAGE(clockwise ? total_angle < -0.2f : total_angle > 0.2f, "arc trace lacked directed intermediate motion");
}

static void d2_normalize(void)
{
	grbl_test_command_ok("$X");
	grbl_test_command_ok("$20=0");
	grbl_test_command_ok("G21G90G17G94G54G49G92.1M5M9");
	grbl_test_command_ok("G53G0X0Y0Z0");
	d2_wait_idle_timeout(GRBL_TEST_TIMEOUT_MS);
}

static void d2_set_up(void)
{
	grbl_test_start();
	d2_normalize();
}

static void d2_tear_down(void)
{
	test_io_set_callback(TEST_IO_PROBE, NULL);
	grbl_test_stop();
}

#define D2_TEST_RUN(x) \
	grbl_test_start();       \
	RUN_TEST(x);       \
	grbl_test_stop()
#define D2_UNITY_MAIN(...)  \
	int main(void)          \
	{                       \
		UNITY_BEGIN();      \
		__VA_ARGS__;        \
		return UNITY_END(); \
	}

#endif
