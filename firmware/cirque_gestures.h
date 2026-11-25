#pragma once

#include <stdbool.h>
#include <stdint.h>

enum cirque_gesture_mode {
    CIRQUE_IDLE,
    CIRQUE_POINTER,
    CIRQUE_RING,
};

struct cirque_gesture_state {
    enum cirque_gesture_mode mode;
    uint32_t started_ms;
    int32_t start_x, start_y;
    int32_t last_x, last_y;
    int32_t remainder_x, remainder_y;
    int32_t turn;
    bool tap_candidate;
    bool scrolled;
};

struct cirque_gesture_report {
    int32_t dx, dy, wheel;
    bool tap;
};

enum {
    CIRQUE_RING_START = 750,
    CIRQUE_RING_INNER = 620,
    CIRQUE_TAP_RADIUS = 44,
    CIRQUE_TAP_TIME_MS = 200,
    CIRQUE_CURSOR_DIVISOR = 6,
    CIRQUE_ANGLE_SCALE = 4096,
    CIRQUE_ANGLE_PER_TICK = 820,
};

static inline int32_t cirque_clamp(int32_t value, int32_t lower, int32_t upper) {
    return value < lower ? lower : (value > upper ? upper : value);
}

static inline int32_t cirque_radius_squared(int32_t x, int32_t y) {
    return x * x + y * y;
}

static inline int32_t cirque_pointer_delta(int32_t delta, int32_t *remainder) {
    const int32_t total = delta + *remainder;
    *remainder = total % CIRQUE_CURSOR_DIVISOR;
    return cirque_clamp(total / CIRQUE_CURSOR_DIVISOR, -127, 127);
}

static inline struct cirque_gesture_report cirque_gesture_sample(
    struct cirque_gesture_state *state, bool touching, uint16_t raw_x, uint16_t raw_y,
    uint32_t now_ms) {
    struct cirque_gesture_report report = {0};

    if (!touching) {
        report.tap = state->mode != CIRQUE_IDLE && state->tap_candidate &&
                     now_ms - state->started_ms <= CIRQUE_TAP_TIME_MS;
        state->mode = CIRQUE_IDLE;
        return report;
    }

    // Scale Pinnacle's specified active coordinate range to a square centered at zero.
    const int32_t x = (cirque_clamp(raw_x, 128, 1920) - 1024) * 1024 / 896;
    const int32_t y = (cirque_clamp(raw_y, 64, 1472) - 768) * 1024 / 704;
    const int32_t radius = cirque_radius_squared(x, y);

    if (state->mode == CIRQUE_IDLE) {
        *state = (struct cirque_gesture_state) {
            .mode = radius >= CIRQUE_RING_START * CIRQUE_RING_START ? CIRQUE_RING : CIRQUE_POINTER,
            .started_ms = now_ms,
            .start_x = x,
            .start_y = y,
            .last_x = x,
            .last_y = y,
            .tap_candidate = true,
        };
        return report;
    }

    const bool was_tap_candidate = state->tap_candidate;
    if (cirque_radius_squared(x - state->start_x, y - state->start_y) >
        CIRQUE_TAP_RADIUS * CIRQUE_TAP_RADIUS) {
        state->tap_candidate = false;
    }

    if (state->mode == CIRQUE_RING) {
        // Allow a gesture started at the rim to move inward before it emits a wheel tick.
        if (radius < CIRQUE_RING_INNER * CIRQUE_RING_INNER) {
            if (!state->scrolled) {
                state->mode = CIRQUE_POINTER;
            }
        } else {
            const int32_t dot = state->last_x * x + state->last_y * y;
            if (dot > CIRQUE_RING_INNER * CIRQUE_RING_INNER / 2) {
                const int32_t cross = state->last_x * y - state->last_y * x;
                state->turn += (int32_t)((int64_t)cross * CIRQUE_ANGLE_SCALE / dot);
                const int32_t ticks = state->turn / CIRQUE_ANGLE_PER_TICK;
                if (ticks != 0) {
                    // Positive wheel values scroll up. Positive angles run clockwise (Y down).
                    report.wheel = -ticks;
                    state->turn %= CIRQUE_ANGLE_PER_TICK;
                    state->tap_candidate = false;
                    state->scrolled = true;
                }
            }
        }
    } else if (!state->tap_candidate) {
        const int32_t old_x = was_tap_candidate ? state->start_x : state->last_x;
        const int32_t old_y = was_tap_candidate ? state->start_y : state->last_y;
        report.dx = cirque_pointer_delta(x - old_x, &state->remainder_x);
        report.dy = cirque_pointer_delta(y - old_y, &state->remainder_y);
    }

    state->last_x = x;
    state->last_y = y;
    return report;
}
