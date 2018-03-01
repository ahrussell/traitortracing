#include <string>
#include "relic_compat.h"

extern "C" {
#include <relic/relic.h>
}

/*
 * Functions adapted from RELIC
 */

void fp2_copy_const(fp2_t c, const fp2_t a) {
    fp_copy(c[0], a[0]);
    fp_copy(c[1], a[1]);
}

void ep2_copy_const(ep2_t r, const ep2_t p) {
    fp2_copy_const(r->x, p->x);
    fp2_copy_const(r->y, p->y);
    fp2_copy_const(r->z, p->z);
    r->norm = p->norm;
}

void fp6_copy_const(fp6_t c, const fp6_t a) {
    fp2_copy_const(c[0], a[0]);
    fp2_copy_const(c[1], a[1]);
    fp2_copy_const(c[2], a[2]);
}

void fp12_copy_const(fp12_t c, const fp12_t a) {
    fp6_copy_const(c[0], a[0]);
    fp6_copy_const(c[1], a[1]);
}

std::string dig_to_str(dig_t a, int pad) {
    char buf[64];

#if DIGIT == 64
	if (pad) {
        snprintf(buf, sizeof(buf), "%.16" PRIX64, (uint64_t)a);
	} else {
        snprintf(buf, sizeof(buf), "%" PRIX64, (uint64_t)a);
	}
#elif DIGIT == 32
	if (pad) {
        snprintf(buf, sizeof(buf), "%.8" PRIX32, (uint32_t)a);
	} else {
        snprintf(buf, sizeof(buf), "%" PRIX32, (uint32_t)a);
	}
#elif DIGIT == 16
	if (pad) {
        snprintf(buf, sizeof(buf), "%.4" PRIX16, (uint16_t)a);
	} else {
        snprintf(buf, sizeof(buf), "%" PRIX16, (uint16_t)a);
	}
#else
	if (pad) {
        snprintf(buf, sizeof(buf), "%.2" PRIX8, (uint8_t)a);
	} else {
        snprintf(buf, sizeof(buf), "%" PRIX8, (uint8_t)a);
	}
#endif

    return std::string(buf);
}

std::string fp2_to_str(const fp2_t a) {
    std::string s = "";

    s += fp_to_str(a[0]);
    s += "\n";
    s += fp_to_str(a[1]);
    s += "\n";

    return s;
}

std::string fp6_to_str(const fp6_t a) {
    std::string s = "";

    s += fp2_to_str(a[0]);
    s += fp2_to_str(a[1]);
    s += fp2_to_str(a[2]);

    return s;
}

std::string fp12_to_str(const fp12_t a) {
    std::string s = "";

    s += fp6_to_str(a[0]);
    s += fp6_to_str(a[1]);

    return s;
}

std::string fp_to_str(const fp_t a) {
    int i;
	bn_t t;

    char dig_str[100];
    std::string s = "";

	bn_null(t);

	TRY {
		bn_new(t);

#if FP_RDC == MONTY
		if (a != fp_prime_get()) {
			fp_prime_back(t, a);
		} else {
			bn_read_raw(t, a, FP_DIGS);
		}
#else
		bn_read_raw(t, a, FP_DIGS);
#endif

		for (i = FP_DIGS - 1; i > 0; i--) {
			if (i >= t->used) {
                s += dig_to_str(0, 1);
			} else {
                s += dig_to_str(t->dp[i], 1);
			}

            s += " ";
		}
        s += dig_to_str(t->dp[0], 1);

	} CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(t);
        return s;
	}

    return s;
}

std::string bn_to_str(const bn_t a) {
    int i;
    char buf[128];

    if (a->sign == BN_NEG) {
        snprintf(buf, sizeof(buf), "-");
    }
    if (a->used == 0) {
        snprintf(buf, sizeof(buf), "0");
    } else {
#if WORD == 64
        snprintf(buf, sizeof(buf), "%" PRIX64, (uint64_t)a->dp[a->used - 1]);
        for (i = a->used - 2; i >= 0; i--) {
            snprintf(buf, sizeof(buf), "%.*" PRIX64, (int)(2 * (BN_DIGIT / 8)),
                    (uint64_t)a->dp[i]);
        }
#else
        snprintf(buf, sizeof(buf), "%" PRIX32, (uint32_t)a->dp[a->used - 1]);
        for (i = a->used - 2; i >= 0; i--) {
            snprintf(buf, sizeof(buf), "%.*" PRIX32, (int)(2 * (BN_DIGIT / 8)),
                    (uint32_t)a->dp[i]);
        }
#endif
    }

    return std::string(buf) + "\n";
}