#pragma once

#include "common.h"

typedef enum {
    UP, 
    DOWN, 
    LEFT, 
    RIGHT,

    ATTACK_1,
    ATTACK_2, 
    ATTACK_3, 
    ATTACK_4, 
    ATTACK_5, 
    ATTACK_6, 
    ATTACK_7, 
    ATTACK_8, 

    MACRO_1,
    MACRO_2,
    MACRO_3,
    MACRO_4,
    MACRO_5,

    MENU,

    EXTRA_1,
    EXTRA_2,
    EXTRA_3,
    EXTRA_4,
    EXTRA_5,
    EXTRA_6,
    EXTRA_7,
    EXTRA_8,
} VirtualButton;

typedef enum {
    // No cleaning
    SOCD_NATURAL = 1,

    // Directions cancel eachother
    SOCD_NEUTRAL,

    // Left + Right cancel eachother, Up + Down results in Up
    SOCD_ABSOLUTE,

    // Left + Right results in the last input, Up + Down results in Up
    SOCD_LAST_INPUT,
} SocdType;

void press(VirtualButton button);
void release(VirtualButton button);
void release_all(void);
void toggle(VirtualButton button);

void send_inputs(SocdType socd);