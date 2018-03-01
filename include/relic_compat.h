/*
 * relic_compat.h
 * Some utility functions that are adapted from the RELIC library
 */

#pragma once

#include <string>
#include <inttypes.h>
#include <stdio.h>

extern "C" {
#include <relic/relic.h>
}

void fp2_copy_const(fp2_t c, const fp2_t a);
void ep2_copy_const(ep2_t r, const ep2_t p);
void fp6_copy_const(fp6_t c, const fp6_t a);
void fp12_copy_const(fp12_t c, const fp12_t a);

std::string dig_to_str(dig_t a, int pad);
std::string fp2_to_str(const fp2_t a);
std::string fp6_to_str(const fp6_t a);
std::string fp12_to_str(const fp12_t a);
std::string fp_to_str(const fp_t a);
std::string bn_to_str(const bn_t a);