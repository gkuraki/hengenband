﻿/* File: z-rand.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */


/* Purpose: a simple random number generator -BEN- */

#if defined(WINDOWS)
#include <Windows.h>
#endif

#include "z-rand.h"




/*
 * Angband 2.7.9 introduced a new (optimized) random number generator,
 * based loosely on the old "random.c" from Berkeley but with some major
 * optimizations and algorithm changes.  See below for more details.
 *
 * Code by myself (benh@phial.com) and Randy (randy@stat.tamu.edu).
 *
 * This code provides (1) a "decent" RNG, based on the "BSD-degree-63-RNG"
 * used in Angband 2.7.8, but rather optimized, and (2) a "simple" RNG,
 * based on the simple "LCRNG" currently used in Angband, but "corrected"
 * to give slightly better values.  Both of these are available in two
 * flavors, first, the simple "mod" flavor, which is fast, but slightly
 * biased at high values, and second, the simple "div" flavor, which is
 * less fast (and potentially non-terminating) but which is not biased
 * and is much less subject to low-bit-non-randomness problems.
 *
 * You can select your favorite flavor by proper definition of the
 * "randint0()" macro in the "defines.h" file.
 *
 * Note that, in Angband 2.8.0, the "state" table will be saved in the
 * savefile, so a special "initialization" phase will be necessary.
 *
 * Note the use of the "simple" RNG, first you activate it via
 * "Rand_quick = TRUE" and "Rand_value = seed" and then it is used
 * automatically used instead of the "complex" RNG, and when you are
 * done, you de-activate it via "Rand_quick = FALSE" or choose a new
 * seed via "Rand_value = seed".
 *
 *
 * RNG algorithm was fully rewritten. Upper comment is OLD.
 */


/*
 * Currently unused
 */
u16b Rand_place;

/*
 * Current "state" table for the RNG
 * Only index 0 to 3 are used
 */
u32b Rand_state[RAND_DEG] = {
	123456789,
	362436069,
	521288629,
	88675123,
};


typedef struct {
	u32b dw[2];
} u64b;

static u64b u64b_xor(u64b a, u64b b)
{
	u64b result;

	result.dw[0] = a.dw[0] ^ b.dw[0];
	result.dw[1] = a.dw[1] ^ b.dw[1];

	return result;
}

static u64b u64b_shiftl(u64b x, int k)
{
	u64b result;

	if (k < 32) {
		result.dw[1] = (x.dw[1] << k) | (x.dw[0] >> (32 - k));
		result.dw[0] = (x.dw[0] << k);
	}
	else {
		result.dw[1] = (x.dw[0] << (k - 32));
		result.dw[0] = 0;
	}

	return result;
}

static u64b u64b_rotl(u64b x, int k)
{
	u64b result;

	if (k < 32) {
		result.dw[0] = (x.dw[0] << k) | (x.dw[1] >> (32 - k));
		result.dw[1] = (x.dw[1] << k) | (x.dw[0] >> (32 - k));
	}
	else {
		result.dw[0] = (x.dw[0] >> (64 - k)) | (x.dw[1] << (k - 32));
		result.dw[1] = (x.dw[1] >> (64 - k)) | (x.dw[0] << (k - 32));
	}

	return result;
}

static u64b u64b_add(u64b a, u64b b)
{
	u64b result;

	result.dw[0] = a.dw[0] + b.dw[0];
	result.dw[1] = a.dw[1] + b.dw[1];

	if (result.dw[0] < a.dw[0])
		result.dw[1] ++;

	return result;
}

/*
 * Initialize Xorshift Algorithm state
 */
static void Rand_Xorshift_seed(u32b seed, u32b* state)
{
	int i;

	/* Initialize Xorshift Algorithm RNG */
	for (i = 1; i <= 4; ++ i) {
		seed = 1812433253UL * (seed ^ (seed >> 30)) + i;
		state[i-1] = seed;
	}
}

#if 0
/*
 * Xorshift Algorithm
 */
static u32b Rand_Xorshift(u32b* state)
{
	u32b t = state[0] ^ (state[0] << 11);

	state[0] = state[1];
	state[1] = state[2];
	state[2] = state[3];

	state[3] = (state[3] ^ (state[3] >> 19)) ^ (t ^ (t >> 8));

	return state[3];
}
#endif

/*
 * Xoroshiro128+ Algorithm
 */
static u32b Rand_Xoroshiro128plus(u32b* state)
{
	const u64b s0 = *((u64b*)state);
	u64b s1 = *((u64b*)state + 1);
	const u64b result = u64b_add(s0, s1);

	s1 = u64b_xor(s0, s1);
	*((u64b*)state) = u64b_xor(u64b_xor(u64b_rotl(s0, 55), s1), u64b_shiftl(s1, 14));
	*((u64b*)state + 1) = u64b_rotl(s1, 36);

	return result.dw[0];
}

static const u32b Rand_Xorshift_max = 0xFFFFFFFF;

/*
 * Initialize the RNG using a new seed
 */
void Rand_state_set(u32b seed)
{
	Rand_Xorshift_seed(seed, Rand_state);
}

void Rand_state_init(void)
{
#ifdef RNG_DEVICE

	FILE *fp = fopen(RNG_DEVICE, "r");
	
	do {
		fread(Rand_state, sizeof(Rand_state[0]), 4, fp);
	} while ((Rand_state[0] | Rand_state[1] | Rand_state[2] | Rand_state[3]) == 0);
	
	fclose(fp);

#elif defined(WINDOWS)

	HCRYPTPROV hProvider;

	CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, 0);

	do {
		CryptGenRandom(hProvider, sizeof(Rand_state[0]) * 4, (BYTE*)Rand_state);
	} while ((Rand_state[0] | Rand_state[1] | Rand_state[2] | Rand_state[3]) == 0);

	CryptReleaseContext(hProvider, 0);	

#else

	/* Basic seed */
	u32b seed = (time(NULL));
#ifdef SET_UID
	/* Mutate the seed on Unix machines */
	seed = ((seed >> 3) * (getpid() << 1));
#endif
	/* Seed the RNG */
	Rand_state_set(seed);

#endif
}

/*
 * Backup the RNG state
 */
void Rand_state_backup(u32b* backup_state)
{
	int i;

	for (i = 0; i < 4; ++ i) {
		backup_state[i] = Rand_state[i];
	}
}

/*
 * Restore the RNG state
 */
void Rand_state_restore(u32b* backup_state)
{
	int i;

	for (i = 0; i < 4; ++ i) {
		Rand_state[i] = backup_state[i];
	}
}


/*
 * Extract a "random" number from 0 to m-1, via "division"
 */
static s32b Rand_div_impl(s32b m, u32b* state)
{
	u32b scaling;
	u32b past;
	u32b ret;

	/* Hack -- simple case */
	if (m <= 1) return (0);

	scaling = Rand_Xorshift_max / m;
	past = scaling * m;

	do {
		ret = Rand_Xoroshiro128plus(state);
	} while (ret >= past);

	return ret / scaling;
}

s32b Rand_div(s32b m)
{
	return Rand_div_impl(m, Rand_state);
}




/*
 * The number of entries in the "randnor_table"
 */
#define RANDNOR_NUM	256

/*
 * The standard deviation of the "randnor_table"
 */
#define RANDNOR_STD	64

/*
 * The normal distribution table for the "randnor()" function (below)
 */
static s16b randnor_table[RANDNOR_NUM] =
{
	206,     613,    1022,    1430,		1838,	 2245,	  2652,	   3058,
	3463,    3867,    4271,    4673,	5075,	 5475,	  5874,	   6271,
	6667,    7061,    7454,    7845,	8234,	 8621,	  9006,	   9389,
	9770,   10148,   10524,   10898,   11269,	11638,	 12004,	  12367,
	12727,   13085,   13440,   13792,   14140,	14486,	 14828,	  15168,
	15504,   15836,   16166,   16492,   16814,	17133,	 17449,	  17761,
	18069,   18374,   18675,   18972,   19266,	19556,	 19842,	  20124,
	20403,   20678,   20949,   21216,   21479,	21738,	 21994,	  22245,

	22493,   22737,   22977,   23213,   23446,	23674,	 23899,	  24120,
	24336,   24550,   24759,   24965,   25166,	25365,	 25559,	  25750,
	25937,   26120,   26300,   26476,   26649,	26818,	 26983,	  27146,
	27304,   27460,   27612,   27760,   27906,	28048,	 28187,	  28323,
	28455,   28585,   28711,   28835,   28955,	29073,	 29188,	  29299,
	29409,   29515,   29619,   29720,   29818,	29914,	 30007,	  30098,
	30186,   30272,   30356,   30437,   30516,	30593,	 30668,	  30740,
	30810,   30879,   30945,   31010,   31072,	31133,	 31192,	  31249,

	31304,   31358,   31410,   31460,   31509,	31556,	 31601,	  31646,
	31688,   31730,   31770,   31808,   31846,	31882,	 31917,	  31950,
	31983,   32014,   32044,   32074,   32102,	32129,	 32155,	  32180,
	32205,   32228,   32251,   32273,   32294,	32314,	 32333,	  32352,
	32370,   32387,   32404,   32420,   32435,	32450,	 32464,	  32477,
	32490,   32503,   32515,   32526,   32537,	32548,	 32558,	  32568,
	32577,   32586,   32595,   32603,   32611,	32618,	 32625,	  32632,
	32639,   32645,   32651,   32657,   32662,	32667,	 32672,	  32677,

	32682,   32686,   32690,   32694,   32698,	32702,	 32705,	  32708,
	32711,   32714,   32717,   32720,   32722,	32725,	 32727,	  32729,
	32731,   32733,   32735,   32737,   32739,	32740,	 32742,	  32743,
	32745,   32746,   32747,   32748,   32749,	32750,	 32751,	  32752,
	32753,   32754,   32755,   32756,   32757,	32757,	 32758,	  32758,
	32759,   32760,   32760,   32761,   32761,	32761,	 32762,	  32762,
	32763,   32763,   32763,   32764,   32764,	32764,	 32764,	  32765,
	32765,   32765,   32765,   32766,   32766,	32766,	 32766,	  32767,
};



/*
 * Generate a random integer number of NORMAL distribution
 *
 * The table above is used to generate a pseudo-normal distribution,
 * in a manner which is much faster than calling a transcendental
 * function to calculate a true normal distribution.
 *
 * Basically, entry 64*N in the table above represents the number of
 * times out of 32767 that a random variable with normal distribution
 * will fall within N standard deviations of the mean.  That is, about
 * 68 percent of the time for N=1 and 95 percent of the time for N=2.
 *
 * The table above contains a "faked" final entry which allows us to
 * pretend that all values in a normal distribution are strictly less
 * than four standard deviations away from the mean.  This results in
 * "conservative" distribution of approximately 1/32768 values.
 *
 * Note that the binary search takes up to 16 quick iterations.
 */
s16b randnor(int mean, int stand)
{
	s16b tmp;
	s16b offset;

	s16b low = 0;
	s16b high = RANDNOR_NUM;

	/* Paranoia */
	if (stand < 1) return (s16b)(mean);

	/* Roll for probability */
	tmp = (s16b)randint0(32768);

	/* Binary Search */
	while (low < high)
	{
		int mid = (low + high) >> 1;

		/* Move right if forced */
		if (randnor_table[mid] < tmp)
		{
			low = mid + 1;
		}

		/* Move left otherwise */
		else
		{
			high = (s16b)mid;
		}
	}

	/* Convert the index into an offset */
	offset = (long)stand * (long)low / RANDNOR_STD;

	/* One half should be negative */
	if (randint0(100) < 50) return (mean - offset);

	/* One half should be positive */
	return (mean + offset);
}



/*
 * Generates damage for "2d6" style dice rolls
 */
s16b damroll(int num, int sides)
{
	int i, sum = 0;
	for (i = 0; i < num; i++) sum += randint1(sides);
	return (s16b)(sum);
}


/*
 * Same as above, but always maximal
 */
s16b maxroll(int num, int sides)
{
	return (num * sides);
}


/*
 * Given a numerator and a denominator, supply a properly rounded result,
 * using the RNG to smooth out remainders.  -LM-
 */
s32b div_round(s32b n, s32b d)
{
        s32b tmp;

        /* Refuse to divide by zero */
        if (!d) return (n);

        /* Division */
        tmp = n / d;

        /* Rounding */
        if ((ABS(n) % ABS(d)) > randint0(ABS(d)))
        {
                /* Increase the absolute value */
                if (n * d > 0L) tmp += 1L;
                else            tmp -= 1L;
        }

        /* Return */
        return (tmp);
}




/*
 * Extract a "random" number from 0 to m-1, using the RNG.
 *
 * This function should be used when generating random numbers in
 * "external" program parts like the main-*.c files.  It preserves
 * the current RNG state to prevent influences on game-play.
 *
 * Could also use rand() from <stdlib.h> directly. XXX XXX XXX
 */
s32b Rand_external(s32b m)
{
	static bool initialized = FALSE;
	static u32b Rand_state_external[4];

	if (!initialized)
	{
		/* Initialize with new seed */
		u32b seed = (u32b)time(NULL);
		Rand_Xorshift_seed(seed, Rand_state_external);
		initialized = TRUE;
	}

	return Rand_div_impl(m, Rand_state_external);
}
