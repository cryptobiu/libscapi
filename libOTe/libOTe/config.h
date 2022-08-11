#pragma once


#define OTE_RANDOM_ORACLE 1
#define OTE_DAVIE_MEYER_AES 2


// build the library with simplest OT enabled
/* #undef ENABLE_SIMPLESTOT */

// build the library with Silent OT Extension enabled
/* #undef ENABLE_SILENTOT */

// Silent OT requires bit poly mul
#ifdef ENABLE_SILENTOT
	#define ENABLE_BITPOLYMUL ON
#endif

// build the library with Kyber OT enabled
/* #undef ENABLE_KYBEROT */


// build the library with simplest OT enabled
#define OTE_KOS_HASH OTE_DAVIE_MEYER_AES


// build the library where KOS is round optimized.
/* #undef OTE_KOS_FIAT_SHAMIR */
