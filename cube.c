#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "cube.h"

#ifdef DEBUG

#include <stdio.h>
#define _static
#define _static_inline
#define DBG_LOG(...) fprintf(stderr, __VA_ARGS__)
#define DBG_WARN(condition, ...) if (!(condition)) DBG_LOG(__VA_ARGS__);
#define DBG_ASSERT(condition, retval, ...) \
	if (!(condition)) {                \
		DBG_LOG(__VA_ARGS__);      \
		return retval;             \
	}

#else

#define _static static
#define _static_inline static inline
#define DBG_LOG(...)
#define DBG_WARN(condition, ...)
#define DBG_ASSERT(condition, retval, ...)

#endif

/******************************************************************************
Section: mathematical constants
******************************************************************************/

#define _2p11  2048U
#define _2p12  4096U
#define _3p7   2187U
#define _3p8   6561U
#define _12c4  495U
#define _8c4   70U

_static int64_t binomial[12][12] = {
	{1,  0,  0,   0,   0,   0,   0,   0,   0,  0,  0, 0},
	{1,  1,  0,   0,   0,   0,   0,   0,   0,  0,  0, 0},
	{1,  2,  1,   0,   0,   0,   0,   0,   0,  0,  0, 0},
	{1,  3,  3,   1,   0,   0,   0,   0,   0,  0,  0, 0},
	{1,  4,  6,   4,   1,   0,   0,   0,   0,  0,  0, 0},
	{1,  5, 10,  10,   5,   1,   0,   0,   0,  0,  0, 0},
	{1,  6, 15,  20,  15,   6,   1,   0,   0,  0,  0, 0},
	{1,  7, 21,  35,  35,  21,   7,   1,   0,  0,  0, 0},
	{1,  8, 28,  56,  70,  56,  28,   8,   1,  0,  0, 0},
	{1,  9, 36,  84, 126, 126,  84,  36,   9,  1,  0, 0},
	{1, 10, 45, 120, 210, 252, 210, 120,  45, 10,  1, 0},
	{1, 11, 55, 165, 330, 462, 462, 330, 165, 55, 11, 1},
};

/******************************************************************************
Section: moves definitions and tables
******************************************************************************/

#define U  0U
#define U2 1U
#define U3 2U
#define D  3U
#define D2 4U
#define D3 5U
#define R  6U
#define R2 7U
#define R3 8U
#define L  9U
#define L2 10U
#define L3 11U
#define F  12U
#define F2 13U
#define F3 14U
#define B  15U
#define B2 16U
#define B3 17U

#define UFr 0
#define ULr 1
#define UBr 2
#define URr 3
#define DFr 4
#define DLr 5
#define DBr 6
#define DRr 7
#define RUr 8
#define RFr 9
#define RDr 10
#define RBr 11
#define LUr 12
#define LFr 13
#define LDr 14
#define LBr 15
#define FUr 16
#define FRr 17
#define FDr 18
#define FLr 19
#define BUr 20
#define BRr 21
#define BDr 22
#define BLr 23

#define UFm 24
#define ULm 25
#define UBm 26
#define URm 27
#define DFm 28
#define DLm 29
#define DBm 30
#define DRm 31
#define RUm 32
#define RFm 33
#define RDm 34
#define RBm 35
#define LUm 36
#define LFm 37
#define LDm 38
#define LBm 39
#define FUm 40
#define FRm 41
#define FDm 42
#define FLm 43
#define BUm 44
#define BRm 45
#define BDm 46
#define BLm 47

#define _c_ufr      0U
#define _c_ubl      1U
#define _c_dfl      2U
#define _c_dbr      3U
#define _c_ufl      4U
#define _c_ubr      5U
#define _c_dfr      6U
#define _c_dbl      7U

#define _e_uf       0U
#define _e_ub       1U
#define _e_db       2U
#define _e_df       3U
#define _e_ur       4U
#define _e_ul       5U
#define _e_dl       6U
#define _e_dr       7U
#define _e_fr       8U
#define _e_fl       9U
#define _e_bl       10U
#define _e_br       11U

#define _eoshift    4U
#define _coshift    5U

#define _pbits      0xFU
#define _esepbit1   0x4U
#define _esepbit2   0x8U
#define _csepbit    0x4U
#define _eobit      0x10U
#define _cobits     0xF0U
#define _cobits2    0x60U
#define _ctwist_cw  0x20U
#define _ctwist_ccw 0x40U
#define _eflip      0x10U
#define _error      0xFFU

_static cube_t zero = { .corner = {0}, .edge = {0} };
_static cube_t solved = {
	.corner = {0, 1, 2, 3, 4, 5, 6, 7},
	.edge = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
};

#define zero_fast fastcube( \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#define solved_fast fastcube( \
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)

#define _move_cube_U fastcube( \
    5, 4, 2, 3, 0, 1, 6, 7, 4, 5, 2, 3, 1, 0, 6, 7, 8, 9, 10, 11)
#define _move_cube_U2 fastcube( \
    1, 0, 2, 3, 5, 4, 6, 7, 1, 0, 2, 3, 5, 4, 6, 7, 8, 9, 10, 11)
#define _move_cube_U3 fastcube( \
    4, 5, 2, 3, 1, 0, 6, 7, 5, 4, 2, 3, 0, 1, 6, 7, 8, 9, 10, 11)
#define _move_cube_D fastcube( \
    0, 1, 7, 6, 4, 5, 2, 3, 0, 1, 7, 6, 4, 5, 2, 3, 8, 9, 10, 11)
#define _move_cube_D2 fastcube( \
    0, 1, 3, 2, 4, 5, 7, 6, 0, 1, 3, 2, 4, 5, 7, 6, 8, 9, 10, 11)
#define _move_cube_D3 fastcube( \
    0, 1, 6, 7, 4, 5, 3, 2, 0, 1, 6, 7, 4, 5, 3, 2, 8, 9, 10, 11)
#define _move_cube_R fastcube( \
    70, 1, 2, 69, 4, 32, 35, 7, 0, 1, 2, 3, 8, 5, 6, 11, 7, 9, 10, 4)
#define _move_cube_R2 fastcube( \
    3, 1, 2, 0, 4, 6, 5, 7, 0, 1, 2, 3, 7, 5, 6, 4, 11, 9, 10, 8)
#define _move_cube_R3 fastcube( \
    69, 1, 2, 70, 4, 35, 32, 7, 0, 1, 2, 3, 11, 5, 6, 8, 4, 9, 10, 7)
#define _move_cube_L fastcube( \
    0, 71, 68, 3, 33, 5, 6, 34, 0, 1, 2, 3, 4, 10, 9, 7, 8, 5, 6, 11)
#define _move_cube_L2 fastcube( \
    0, 2, 1, 3, 7, 5, 6, 4, 0, 1, 2, 3, 4, 6, 5, 7, 8, 10, 9, 11)
#define _move_cube_L3 fastcube( \
    0, 68, 71, 3, 34, 5, 6, 33, 0, 1, 2, 3, 4, 9, 10, 7, 8, 6, 5, 11)
#define _move_cube_F fastcube( \
    36, 1, 38, 3, 66, 5, 64, 7, 25, 1, 2, 24, 4, 5, 6, 7, 16, 19, 10, 11)
#define _move_cube_F2 fastcube( \
    2, 1, 0, 3, 6, 5, 4, 7, 3, 1, 2, 0, 4, 5, 6, 7, 9, 8, 10, 11)
#define _move_cube_F3 fastcube( \
    38, 1, 36, 3, 64, 5, 66, 7, 24, 1, 2, 25, 4, 5, 6, 7, 19, 16, 10, 11)
#define _move_cube_B fastcube( \
    0, 37, 2, 39, 4, 67, 6, 65, 0, 27, 26, 3, 4, 5, 6, 7, 8, 9, 17, 18)
#define _move_cube_B2 fastcube( \
    0, 3, 2, 1, 4, 7, 6, 5, 0, 2, 1, 3, 4, 5, 6, 7, 8, 9, 11, 10)
#define _move_cube_B3 fastcube( \
    0, 39, 2, 37, 4, 65, 6, 67, 0, 26, 27, 3, 4, 5, 6, 7, 8, 9, 18, 17)

#define _trans_cube_UFr fastcube( \
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)
#define _trans_cube_UFr_inverse fastcube( \
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)
#define _trans_cube_ULr fastcube( \
    4, 5, 7, 6, 1, 0, 2, 3, 5, 4, 7, 6, 0, 1, 2, 3, 25, 26, 27, 24)
#define _trans_cube_ULr_inverse fastcube( \
    5, 4, 6, 7, 0, 1, 3, 2, 4, 5, 6, 7, 1, 0, 3, 2, 27, 24, 25, 26)
#define _trans_cube_UBr fastcube( \
    1, 0, 3, 2, 5, 4, 7, 6, 1, 0, 3, 2, 5, 4, 7, 6, 10, 11, 8, 9)
#define _trans_cube_UBr_inverse fastcube( \
    1, 0, 3, 2, 5, 4, 7, 6, 1, 0, 3, 2, 5, 4, 7, 6, 10, 11, 8, 9)
#define _trans_cube_URr fastcube( \
    5, 4, 6, 7, 0, 1, 3, 2, 4, 5, 6, 7, 1, 0, 3, 2, 27, 24, 25, 26)
#define _trans_cube_URr_inverse fastcube( \
    4, 5, 7, 6, 1, 0, 2, 3, 5, 4, 7, 6, 0, 1, 2, 3, 25, 26, 27, 24)
#define _trans_cube_DFr fastcube( \
    2, 3, 0, 1, 6, 7, 4, 5, 3, 2, 1, 0, 6, 7, 4, 5, 9, 8, 11, 10)
#define _trans_cube_DFr_inverse fastcube( \
    2, 3, 0, 1, 6, 7, 4, 5, 3, 2, 1, 0, 6, 7, 4, 5, 9, 8, 11, 10)
#define _trans_cube_DLr fastcube( \
    7, 6, 4, 5, 2, 3, 1, 0, 6, 7, 4, 5, 2, 3, 0, 1, 26, 25, 24, 27)
#define _trans_cube_DLr_inverse fastcube( \
    7, 6, 4, 5, 2, 3, 1, 0, 6, 7, 4, 5, 2, 3, 0, 1, 26, 25, 24, 27)
#define _trans_cube_DBr fastcube( \
    3, 2, 1, 0, 7, 6, 5, 4, 2, 3, 0, 1, 7, 6, 5, 4, 11, 10, 9, 8)
#define _trans_cube_DBr_inverse fastcube( \
    3, 2, 1, 0, 7, 6, 5, 4, 2, 3, 0, 1, 7, 6, 5, 4, 11, 10, 9, 8)
#define _trans_cube_DRr fastcube( \
    6, 7, 5, 4, 3, 2, 0, 1, 7, 6, 5, 4, 3, 2, 1, 0, 24, 27, 26, 25)
#define _trans_cube_DRr_inverse fastcube( \
    6, 7, 5, 4, 3, 2, 0, 1, 7, 6, 5, 4, 3, 2, 1, 0, 24, 27, 26, 25)
#define _trans_cube_RUr fastcube( \
    64, 67, 65, 66, 37, 38, 36, 39, 20, 23, 22, 21, 24, 27, 26, 25, 0, 1, 2, 3)
#define _trans_cube_RUr_inverse fastcube( \
    32, 34, 35, 33, 70, 68, 69, 71, 8, 9, 10, 11, 16, 19, 18, 17, 20, 23, 22, 21)
#define _trans_cube_RFr fastcube( \
    38, 37, 36, 39, 64, 67, 66, 65, 24, 27, 26, 25, 23, 20, 21, 22, 19, 16, 17, 18)
#define _trans_cube_RFr_inverse fastcube( \
    36, 39, 38, 37, 66, 65, 64, 67, 25, 26, 27, 24, 21, 22, 23, 20, 16, 19, 18, 17)
#define _trans_cube_RDr fastcube( \
    67, 64, 66, 65, 38, 37, 39, 36, 23, 20, 21, 22, 27, 24, 25, 26, 2, 3, 0, 1)
#define _trans_cube_RDr_inverse fastcube( \
    33, 35, 34, 32, 71, 69, 68, 70, 10, 11, 8, 9, 17, 18, 19, 16, 21, 22, 23, 20)
#define _trans_cube_RBr fastcube( \
    37, 38, 39, 36, 67, 64, 65, 66, 27, 24, 25, 26, 20, 23, 22, 21, 17, 18, 19, 16)
#define _trans_cube_RBr_inverse fastcube( \
    37, 38, 39, 36, 67, 64, 65, 66, 27, 24, 25, 26, 20, 23, 22, 21, 17, 18, 19, 16)
#define _trans_cube_LUr fastcube( \
    65, 66, 64, 67, 36, 39, 37, 38, 21, 22, 23, 20, 26, 25, 24, 27, 1, 0, 3, 2)
#define _trans_cube_LUr_inverse fastcube( \
    34, 32, 33, 35, 68, 70, 71, 69, 9, 8, 11, 10, 19, 16, 17, 18, 22, 21, 20, 23)
#define _trans_cube_LFr fastcube( \
    36, 39, 38, 37, 66, 65, 64, 67, 25, 26, 27, 24, 21, 22, 23, 20, 16, 19, 18, 17)
#define _trans_cube_LFr_inverse fastcube( \
    38, 37, 36, 39, 64, 67, 66, 65, 24, 27, 26, 25, 23, 20, 21, 22, 19, 16, 17, 18)
#define _trans_cube_LDr fastcube( \
    66, 65, 67, 64, 39, 36, 38, 37, 22, 21, 20, 23, 25, 26, 27, 24, 3, 2, 1, 0)
#define _trans_cube_LDr_inverse fastcube( \
    35, 33, 32, 34, 69, 71, 70, 68, 11, 10, 9, 8, 18, 17, 16, 19, 23, 20, 21, 22)
#define _trans_cube_LBr fastcube( \
    39, 36, 37, 38, 65, 66, 67, 64, 26, 25, 24, 27, 22, 21, 20, 23, 18, 17, 16, 19)
#define _trans_cube_LBr_inverse fastcube( \
    39, 36, 37, 38, 65, 66, 67, 64, 26, 25, 24, 27, 22, 21, 20, 23, 18, 17, 16, 19)
#define _trans_cube_FUr fastcube( \
    68, 70, 69, 71, 32, 34, 33, 35, 16, 19, 18, 17, 9, 8, 11, 10, 5, 4, 7, 6)
#define _trans_cube_FUr_inverse fastcube( \
    68, 70, 69, 71, 32, 34, 33, 35, 16, 19, 18, 17, 9, 8, 11, 10, 5, 4, 7, 6)
#define _trans_cube_FRr fastcube( \
    32, 34, 35, 33, 70, 68, 69, 71, 8, 9, 10, 11, 16, 19, 18, 17, 20, 23, 22, 21)
#define _trans_cube_FRr_inverse fastcube( \
    64, 67, 65, 66, 37, 38, 36, 39, 20, 23, 22, 21, 24, 27, 26, 25, 0, 1, 2, 3)
#define _trans_cube_FDr fastcube( \
    70, 68, 71, 69, 34, 32, 35, 33, 19, 16, 17, 18, 8, 9, 10, 11, 7, 6, 5, 4)
#define _trans_cube_FDr_inverse fastcube( \
    69, 71, 68, 70, 33, 35, 32, 34, 17, 18, 19, 16, 11, 10, 9, 8, 4, 5, 6, 7)
#define _trans_cube_FLr fastcube( \
    34, 32, 33, 35, 68, 70, 71, 69, 9, 8, 11, 10, 19, 16, 17, 18, 22, 21, 20, 23)
#define _trans_cube_FLr_inverse fastcube( \
    65, 66, 64, 67, 36, 39, 37, 38, 21, 22, 23, 20, 26, 25, 24, 27, 1, 0, 3, 2)
#define _trans_cube_BUr fastcube( \
    69, 71, 68, 70, 33, 35, 32, 34, 17, 18, 19, 16, 11, 10, 9, 8, 4, 5, 6, 7)
#define _trans_cube_BUr_inverse fastcube( \
    70, 68, 71, 69, 34, 32, 35, 33, 19, 16, 17, 18, 8, 9, 10, 11, 7, 6, 5, 4)
#define _trans_cube_BRr fastcube( \
    35, 33, 32, 34, 69, 71, 70, 68, 11, 10, 9, 8, 18, 17, 16, 19, 23, 20, 21, 22)
#define _trans_cube_BRr_inverse fastcube( \
    66, 65, 67, 64, 39, 36, 38, 37, 22, 21, 20, 23, 25, 26, 27, 24, 3, 2, 1, 0)
#define _trans_cube_BDr fastcube( \
    71, 69, 70, 68, 35, 33, 34, 32, 18, 17, 16, 19, 10, 11, 8, 9, 6, 7, 4, 5)
#define _trans_cube_BDr_inverse fastcube( \
    71, 69, 70, 68, 35, 33, 34, 32, 18, 17, 16, 19, 10, 11, 8, 9, 6, 7, 4, 5)
#define _trans_cube_BLr fastcube( \
    33, 35, 34, 32, 71, 69, 68, 70, 10, 11, 8, 9, 17, 18, 19, 16, 21, 22, 23, 20)
#define _trans_cube_BLr_inverse fastcube( \
    67, 64, 66, 65, 38, 37, 39, 36, 23, 20, 21, 22, 27, 24, 25, 26, 2, 3, 0, 1)
#define _trans_cube_UFm fastcube( \
    4, 5, 6, 7, 0, 1, 2, 3, 0, 1, 2, 3, 5, 4, 7, 6, 9, 8, 11, 10)
#define _trans_cube_UFm_inverse fastcube( \
    4, 5, 6, 7, 0, 1, 2, 3, 0, 1, 2, 3, 5, 4, 7, 6, 9, 8, 11, 10)
#define _trans_cube_ULm fastcube( \
    0, 1, 3, 2, 5, 4, 6, 7, 4, 5, 6, 7, 0, 1, 2, 3, 24, 27, 26, 25)
#define _trans_cube_ULm_inverse fastcube( \
    0, 1, 3, 2, 5, 4, 6, 7, 4, 5, 6, 7, 0, 1, 2, 3, 24, 27, 26, 25)
#define _trans_cube_UBm fastcube( \
    5, 4, 7, 6, 1, 0, 3, 2, 1, 0, 3, 2, 4, 5, 6, 7, 11, 10, 9, 8)
#define _trans_cube_UBm_inverse fastcube( \
    5, 4, 7, 6, 1, 0, 3, 2, 1, 0, 3, 2, 4, 5, 6, 7, 11, 10, 9, 8)
#define _trans_cube_URm fastcube( \
    1, 0, 2, 3, 4, 5, 7, 6, 5, 4, 7, 6, 1, 0, 3, 2, 26, 25, 24, 27)
#define _trans_cube_URm_inverse fastcube( \
    1, 0, 2, 3, 4, 5, 7, 6, 5, 4, 7, 6, 1, 0, 3, 2, 26, 25, 24, 27)
#define _trans_cube_DFm fastcube( \
    6, 7, 4, 5, 2, 3, 0, 1, 3, 2, 1, 0, 7, 6, 5, 4, 8, 9, 10, 11)
#define _trans_cube_DFm_inverse fastcube( \
    6, 7, 4, 5, 2, 3, 0, 1, 3, 2, 1, 0, 7, 6, 5, 4, 8, 9, 10, 11)
#define _trans_cube_DLm fastcube( \
    3, 2, 0, 1, 6, 7, 5, 4, 7, 6, 5, 4, 2, 3, 0, 1, 27, 24, 25, 26)
#define _trans_cube_DLm_inverse fastcube( \
    2, 3, 1, 0, 7, 6, 4, 5, 6, 7, 4, 5, 3, 2, 1, 0, 25, 26, 27, 24)
#define _trans_cube_DBm fastcube( \
    7, 6, 5, 4, 3, 2, 1, 0, 2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9)
#define _trans_cube_DBm_inverse fastcube( \
    7, 6, 5, 4, 3, 2, 1, 0, 2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9)
#define _trans_cube_DRm fastcube( \
    2, 3, 1, 0, 7, 6, 4, 5, 6, 7, 4, 5, 3, 2, 1, 0, 25, 26, 27, 24)
#define _trans_cube_DRm_inverse fastcube( \
    3, 2, 0, 1, 6, 7, 5, 4, 7, 6, 5, 4, 2, 3, 0, 1, 27, 24, 25, 26)
#define _trans_cube_RUm fastcube( \
    68, 71, 69, 70, 33, 34, 32, 35, 21, 22, 23, 20, 25, 26, 27, 24, 0, 1, 2, 3)
#define _trans_cube_RUm_inverse fastcube( \
    70, 68, 69, 71, 32, 34, 35, 33, 8, 9, 10, 11, 19, 16, 17, 18, 23, 20, 21, 22)
#define _trans_cube_RFm fastcube( \
    34, 33, 32, 35, 68, 71, 70, 69, 25, 26, 27, 24, 22, 21, 20, 23, 19, 16, 17, 18)
#define _trans_cube_RFm_inverse fastcube( \
    66, 65, 64, 67, 36, 39, 38, 37, 25, 26, 27, 24, 22, 21, 20, 23, 19, 16, 17, 18)
#define _trans_cube_RDm fastcube( \
    71, 68, 70, 69, 34, 33, 35, 32, 22, 21, 20, 23, 26, 25, 24, 27, 2, 3, 0, 1)
#define _trans_cube_RDm_inverse fastcube( \
    71, 69, 68, 70, 33, 35, 34, 32, 10, 11, 8, 9, 18, 17, 16, 19, 22, 21, 20, 23)
#define _trans_cube_RBm fastcube( \
    33, 34, 35, 32, 71, 68, 69, 70, 26, 25, 24, 27, 21, 22, 23, 20, 17, 18, 19, 16)
#define _trans_cube_RBm_inverse fastcube( \
    67, 64, 65, 66, 37, 38, 39, 36, 27, 24, 25, 26, 23, 20, 21, 22, 18, 17, 16, 19)
#define _trans_cube_LUm fastcube( \
    69, 70, 68, 71, 32, 35, 33, 34, 20, 23, 22, 21, 27, 24, 25, 26, 1, 0, 3, 2)
#define _trans_cube_LUm_inverse fastcube( \
    68, 70, 71, 69, 34, 32, 33, 35, 9, 8, 11, 10, 16, 19, 18, 17, 21, 22, 23, 20)
#define _trans_cube_LFm fastcube( \
    32, 35, 34, 33, 70, 69, 68, 71, 24, 27, 26, 25, 20, 23, 22, 21, 16, 19, 18, 17)
#define _trans_cube_LFm_inverse fastcube( \
    64, 67, 66, 65, 38, 37, 36, 39, 24, 27, 26, 25, 20, 23, 22, 21, 16, 19, 18, 17)
#define _trans_cube_LDm fastcube( \
    70, 69, 71, 68, 35, 32, 34, 33, 23, 20, 21, 22, 24, 27, 26, 25, 3, 2, 1, 0)
#define _trans_cube_LDm_inverse fastcube( \
    69, 71, 70, 68, 35, 33, 32, 34, 11, 10, 9, 8, 17, 18, 19, 16, 20, 23, 22, 21)
#define _trans_cube_LBm fastcube( \
    35, 32, 33, 34, 69, 70, 71, 68, 27, 24, 25, 26, 23, 20, 21, 22, 18, 17, 16, 19)
#define _trans_cube_LBm_inverse fastcube( \
    65, 66, 67, 64, 39, 36, 37, 38, 26, 25, 24, 27, 21, 22, 23, 20, 17, 18, 19, 16)
#define _trans_cube_FUm fastcube( \
    64, 66, 65, 67, 36, 38, 37, 39, 16, 19, 18, 17, 8, 9, 10, 11, 4, 5, 6, 7)
#define _trans_cube_FUm_inverse fastcube( \
    32, 34, 33, 35, 68, 70, 69, 71, 16, 19, 18, 17, 8, 9, 10, 11, 4, 5, 6, 7)
#define _trans_cube_FRm fastcube( \
    36, 38, 39, 37, 66, 64, 65, 67, 9, 8, 11, 10, 16, 19, 18, 17, 21, 22, 23, 20)
#define _trans_cube_FRm_inverse fastcube( \
    37, 38, 36, 39, 64, 67, 65, 66, 20, 23, 22, 21, 27, 24, 25, 26, 1, 0, 3, 2)
#define _trans_cube_FDm fastcube( \
    66, 64, 67, 65, 38, 36, 39, 37, 19, 16, 17, 18, 9, 8, 11, 10, 6, 7, 4, 5)
#define _trans_cube_FDm_inverse fastcube( \
    33, 35, 32, 34, 69, 71, 68, 70, 17, 18, 19, 16, 10, 11, 8, 9, 5, 4, 7, 6)
#define _trans_cube_FLm fastcube( \
    38, 36, 37, 39, 64, 66, 67, 65, 8, 9, 10, 11, 19, 16, 17, 18, 23, 20, 21, 22)
#define _trans_cube_FLm_inverse fastcube( \
    36, 39, 37, 38, 65, 66, 64, 67, 21, 22, 23, 20, 25, 26, 27, 24, 0, 1, 2, 3)
#define _trans_cube_BUm fastcube( \
    65, 67, 64, 66, 37, 39, 36, 38, 17, 18, 19, 16, 10, 11, 8, 9, 5, 4, 7, 6)
#define _trans_cube_BUm_inverse fastcube( \
    34, 32, 35, 33, 70, 68, 71, 69, 19, 16, 17, 18, 9, 8, 11, 10, 6, 7, 4, 5)
#define _trans_cube_BRm fastcube( \
    39, 37, 36, 38, 65, 67, 66, 64, 10, 11, 8, 9, 18, 17, 16, 19, 22, 21, 20, 23)
#define _trans_cube_BRm_inverse fastcube( \
    39, 36, 38, 37, 66, 65, 67, 64, 22, 21, 20, 23, 26, 25, 24, 27, 2, 3, 0, 1)
#define _trans_cube_BDm fastcube( \
    67, 65, 66, 64, 39, 37, 38, 36, 18, 17, 16, 19, 11, 10, 9, 8, 7, 6, 5, 4)
#define _trans_cube_BDm_inverse fastcube( \
    35, 33, 34, 32, 71, 69, 70, 68, 18, 17, 16, 19, 11, 10, 9, 8, 7, 6, 5, 4)
#define _trans_cube_BLm fastcube( \
    37, 39, 38, 36, 67, 65, 64, 66, 11, 10, 9, 8, 17, 18, 19, 16, 20, 23, 22, 21)
#define _trans_cube_BLm_inverse fastcube( \
    38, 37, 39, 36, 67, 64, 66, 65, 23, 20, 21, 22, 24, 27, 26, 25, 3, 2, 1, 0)

_static char *cornerstr[] = {
	[_c_ufr] = "UFR",
	[_c_ubl] = "UBL",
	[_c_dfl] = "DFL",
	[_c_dbr] = "DBR",
	[_c_ufl] = "UFL",
	[_c_ubr] = "UBR",
	[_c_dfr] = "DFR",
	[_c_dbl] = "DBL"
};

_static char *cornerstralt[] = {
	[_c_ufr] = "URF",
	[_c_ubl] = "ULB",
	[_c_dfl] = "DLF",
	[_c_dbr] = "DRB",
	[_c_ufl] = "ULF",
	[_c_ubr] = "URB",
	[_c_dfr] = "DRF",
	[_c_dbl] = "DLB"
};

_static char *edgestr[] = {
	[_e_uf] = "UF",
	[_e_ub] = "UB",
	[_e_db] = "DB",
	[_e_df] = "DF",
	[_e_ur] = "UR",
	[_e_ul] = "UL",
	[_e_dl] = "DL",
	[_e_dr] = "DR",
	[_e_fr] = "FR",
	[_e_fl] = "FL",
	[_e_bl] = "BL",
	[_e_br] = "BR"
};

_static char *movestr[] = {
	[U]  = "U",
	[U2] = "U2",
	[U3] = "U'",
	[D]  = "D",
	[D2] = "D2",
	[D3] = "D'",
	[R]  = "R",
	[R2] = "R2",
	[R3] = "R'",
	[L]  = "L",
	[L2] = "L2",
	[L3] = "L'",
	[F]  = "F",
	[F2] = "F2",
	[F3] = "F'",
	[B]  = "B",
	[B2] = "B2",
	[B3] = "B'",
};

_static char *transstr[] = {
	[UFr] = "rotation UF",
	[UFm] = "mirrored UF",
	[ULr] = "rotation UL",
	[ULm] = "mirrored UL",
	[UBr] = "rotation UB",
	[UBm] = "mirrored UB",
	[URr] = "rotation UR",
	[URm] = "mirrored UR",
	[DFr] = "rotation DF",
	[DFm] = "mirrored DF",
	[DLr] = "rotation DL",
	[DLm] = "mirrored DL",
	[DBr] = "rotation DB",
	[DBm] = "mirrored DB",
	[DRr] = "rotation DR",
	[DRm] = "mirrored DR",
	[RUr] = "rotation RU",
	[RUm] = "mirrored RU",
	[RFr] = "rotation RF",
	[RFm] = "mirrored RF",
	[RDr] = "rotation RD",
	[RDm] = "mirrored RD",
	[RBr] = "rotation RB",
	[RBm] = "mirrored RB",
	[LUr] = "rotation LU",
	[LUm] = "mirrored LU",
	[LFr] = "rotation LF",
	[LFm] = "mirrored LF",
	[LDr] = "rotation LD",
	[LDm] = "mirrored LD",
	[LBr] = "rotation LB",
	[LBm] = "mirrored LB",
	[FUr] = "rotation FU",
	[FUm] = "mirrored FU",
	[FRr] = "rotation FR",
	[FRm] = "mirrored FR",
	[FDr] = "rotation FD",
	[FDm] = "mirrored FD",
	[FLr] = "rotation FL",
	[FLm] = "mirrored FL",
	[BUr] = "rotation BU",
	[BUm] = "mirrored BU",
	[BRr] = "rotation BR",
	[BRm] = "mirrored BR",
	[BDr] = "rotation BD",
	[BDm] = "mirrored BD",
	[BLr] = "rotation BL",
	[BLm] = "mirrored BL",
};

static uint8_t inverse_trans_table[48] = {
	[UFr] = UFr,
	[UFm] = UFm,
	[ULr] = URr,
	[ULm] = ULm,
	[UBr] = UBr,
	[UBm] = UBm,
	[URr] = ULr,
	[URm] = URm,
	[DFr] = DFr,
	[DFm] = DFm,
	[DLr] = DLr,
	[DLm] = DRm,
	[DBr] = DBr,
	[DBm] = DBm,
	[DRr] = DRr,
	[DRm] = DLm,
	[RUr] = FRr,
	[RUm] = FLm,
	[RFr] = LFr,
	[RFm] = RFm,
	[RDr] = BLr,
	[RDm] = BRm,
	[RBr] = RBr,
	[RBm] = LBm,
	[LUr] = FLr,
	[LUm] = FRm,
	[LFr] = RFr,
	[LFm] = LFm,
	[LDr] = BRr,
	[LDm] = BLm,
	[LBr] = LBr,
	[LBm] = RBm,
	[FUr] = FUr,
	[FUm] = FUm,
	[FRr] = RUr,
	[FRm] = LUm,
	[FDr] = BUr,
	[FDm] = BUm,
	[FLr] = LUr,
	[FLm] = RUm,
	[BUr] = FDr,
	[BUm] = FDm,
	[BRr] = LDr,
	[BRm] = RDm,
	[BDr] = BDr,
	[BDm] = BDm,
	[BLr] = RDr,
	[BLm] = LDm,
};

/******************************************************************************
Section: portable fast methods

This section contains performance-critical methods that do not use
advanced CPU instructions. They are used as an alternative to the ones
in the previous section(s) for unsupported architectures.
******************************************************************************/

typedef cube_t cube_fast_t;

_static_inline cube_fast_t fastcube(
    uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
    uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
    uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
    uint8_t, uint8_t, uint8_t, uint8_t, uint8_t
);
_static cube_fast_t cubetofast(cube_t);
_static cube_t fasttocube(cube_fast_t);
_static_inline bool equal_fast(cube_fast_t, cube_fast_t);
_static_inline bool issolved_fast(cube_fast_t);
_static_inline cube_fast_t invertco_fast(cube_fast_t);
_static_inline cube_fast_t compose_fast(cube_fast_t, cube_fast_t);

_static_inline int64_t coord_fast_co(cube_fast_t);
_static_inline int64_t coord_fast_eo(cube_fast_t);

_static_inline cube_fast_t
fastcube(
	uint8_t c_ufr,
	uint8_t c_ubl,
	uint8_t c_dfl,
	uint8_t c_dbr,
	uint8_t c_ufl,
	uint8_t c_ubr,
	uint8_t c_dfr,
	uint8_t c_dbl,

	uint8_t e_uf,
	uint8_t e_ub,
	uint8_t e_db,
	uint8_t e_df,
	uint8_t e_ur,
	uint8_t e_ul,
	uint8_t e_dl,
	uint8_t e_dr,
	uint8_t e_fr,
	uint8_t e_fl,
	uint8_t e_bl,
	uint8_t e_br
)
{
	cube_fast_t cube = {
		.corner = {
			c_ufr, c_ubl, c_dfl, c_dbr, c_ufl, c_ubr, c_dfr, c_dbl
		},
		.edge = {
			e_uf, e_ub, e_db, e_df, e_ur, e_ul,
			e_dl, e_dr, e_fr, e_fl, e_bl, e_br
		}
	};

	return cube;
}

_static cube_fast_t
cubetofast(cube_t cube)
{
	cube_fast_t fast;
	memcpy(&fast, &cube, sizeof(cube_fast_t));
	return fast;
}

_static cube_t
fasttocube(cube_fast_t fast)
{
	cube_t cube;
	memcpy(&cube, &fast, sizeof(cube_fast_t));
	return cube;
}

_static_inline bool
equal_fast(cube_fast_t c1, cube_fast_t c2)
{
	uint8_t i;
	bool ret;

	ret = true;
	for (i = 0; i < 8; i++)
		ret = ret && c1.corner[i] == c2.corner[i];
	for (i = 0; i < 12; i++)
		ret = ret && c1.edge[i] == c2.edge[i];

	return ret;
}

_static_inline bool
issolved_fast(cube_fast_t cube)
{
	return equal_fast(cube, solved_fast);
}

_static_inline cube_fast_t
invertco_fast(cube_fast_t c)
{
	uint8_t i, piece, orien;
	cube_fast_t ret;

	ret = c;
	for (i = 0; i < 8; i++) {
		piece = c.corner[i];
		orien = ((piece << 1) | (piece >> 1)) & _cobits2;
		ret.corner[i] = (piece & _pbits) | orien;
	}

	return ret;
}

_static_inline cube_fast_t
compose_fast(cube_fast_t c1, cube_fast_t c2)
{
	cube_fast_t ret;
	uint8_t i, piece1, piece2, p, orien, aux, auy;

	ret = zero_fast;

	for (i = 0; i < 12; i++) {
		piece2 = c2.edge[i];
		p = piece2 & _pbits;
		piece1 = c1.edge[p];
		orien = (piece2 ^ piece1) & _eobit;
		ret.edge[i] = (piece1 & _pbits) | orien;
	}

	for (i = 0; i < 8; i++) {
		piece2 = c2.corner[i];
		p = piece2 & _pbits;
		piece1 = c1.corner[p];
		aux = (piece2 & _cobits) + (piece1 & _cobits);
		auy = (aux + _ctwist_cw) >> 2U;
		orien = (aux + auy) & _cobits2;
		ret.corner[i] = (piece1 & _pbits) | orien;
	}

	return ret;
}

_static_inline int64_t
coord_fast_co(cube_fast_t c)
{
	int i, p;
	int64_t ret;

	for (ret = 0, i = 0, p = 1; i < 7; i++, p *= 3)
		ret += p * (c.corner[i] >> _coshift);

	return ret;
}

_static_inline int64_t
coord_fast_eo(cube_fast_t c)
{
	int i, p;
	int64_t ret;

	for (ret = 0, i = 1, p = 1; i < 12; i++, p *= 2)
		ret += p * (c.edge[i] >> _eoshift);

	return ret;
}

/******************************************************************************
Section: generic methods

This section contains generic functionality, including the public functions.
Some of these routines depend on the efficient functions implemented in the
previous sections, while some other operate directly on the cube.
******************************************************************************/

#define _move(M, c) compose_fast(c, _move_cube_ ## M)
#define _premove(M, c) compose_fast(_move_cube_ ## M, c)
#define _trans_rotation(T, c) \
	compose_fast(compose_fast(_trans_cube_ ## T, c), \
	_trans_cube_ ## T ## _inverse)
#define _trans_mirrored(T, c) \
	invertco_fast(compose_fast(compose_fast(_trans_cube_ ## T, c), \
	_trans_cube_ ## T ## _inverse))

#define _foreach_move(_m, _c, _d, instruction) \
	for (_m = 0; _m < 18; _m++) { _d = move(_c, _m); instruction }
#define _foreach_trans(_t, _c, _d, instruction) \
	for (_t = 0; _t < 48; _t++) { _d = transform(_c, _t); instruction }

cube_t solvedcube(void);
bool isconsistent(cube_t);
bool issolvable(cube_t);
bool issolved(cube_t cube);
bool equal(cube_t, cube_t);
bool iserror(cube_t);
cube_t compose(cube_t, cube_t);
cube_t inverse(cube_t);
cube_t applymoves(cube_t, char *);
cube_t applytrans(cube_t, char *);
cube_t readcube(char *, char *);
void writecube(char *, cube_t, char *);

_static int permsign(uint8_t *, int);
_static uint8_t readco(char *);
_static uint8_t readcp(char *);
_static uint8_t readeo(char *);
_static uint8_t readep(char *);
_static cube_t readcube_H48(char *);
_static uint8_t readpiece_LST(char **);
_static cube_t readcube_LST(char *);
_static int writepiece_LST(uint8_t, char *);
_static void writecube_H48(cube_t, char *);
_static void writecube_LST(cube_t, char *);
_static uint8_t readmove(char);
_static uint8_t readmodifier(char);
_static uint8_t readtrans(char *);
_static int writemoves(uint8_t *, int, char *);
_static void writetrans(uint8_t, char *);
_static cube_fast_t move(cube_fast_t, uint8_t);
_static cube_fast_t transform(cube_fast_t, uint8_t);

cube_t
solvedcube(void)
{
	return solved;
}

bool
isconsistent(cube_t cube)
{
	uint8_t i, p, e, piece;
	bool found[12];

	for (i = 0; i < 12; i++)
		found[i] = false;
	for (i = 0; i < 12; i++) {
		piece = cube.edge[i];
		p = piece & _pbits;
		e = piece & _eobit;
		if (p >= 12)
			goto inconsistent_ep;
		if (e != 0 && e != _eobit)
			goto inconsistent_eo;
		found[p] = true;
	}
	for (i = 0; i < 12; i++)
		if (!found[i])
			goto inconsistent_ep;

	for (i = 0; i < 8; i++)
		found[i] = false;
	for (i = 0; i < 8; i++) {
		piece = cube.corner[i];
		p = piece & _pbits;
		e = piece & _cobits;
		if (p >= 8)
			goto inconsistent_cp;
		if (e != 0 && e != _ctwist_cw && e != _ctwist_ccw)
			goto inconsistent_co;
		found[p] = true;
	}
	for (i = 0; i < 8; i++)
		if (!found[i])
			goto inconsistent_co;

	return true;

inconsistent_ep:
	DBG_LOG("Inconsistent EP\n");
	return false;
inconsistent_cp:
	DBG_LOG("Inconsistent CP\n");
	return false;
inconsistent_eo:
	DBG_LOG("Inconsistent EO\n");
	return false;
inconsistent_co:
	DBG_LOG("Inconsistent CO\n");
	return false;
}

bool
issolvable(cube_t cube)
{
	uint8_t i, eo, co, piece, edges[12], corners[8];

	DBG_ASSERT(isconsistent(cube), false,
	    "issolvable: cube is inconsistent\n");

	for (i = 0; i < 12; i++)
		edges[i] = cube.edge[i] & _pbits;
	for (i = 0; i < 8; i++)
		corners[i] = cube.corner[i] & _pbits;

	if (permsign(edges, 12) != permsign(corners, 8))
		goto issolvable_parity;

	eo = 0;
	for (i = 0; i < 12; i++) {
		piece = cube.edge[i];
		eo += (piece & _eobit) >> _eoshift;
	}
	if (eo % 2 != 0)
		goto issolvable_eo;

	co = 0;
	for (i = 0; i < 8; i++) {
		piece = cube.corner[i];
		co += (piece & _cobits) >> _coshift;
	}
	if (co % 3 != 0)
		goto issolvable_co;

	return true;

issolvable_parity:
	DBG_LOG("EP and CP parities are different\n");
	return false;
issolvable_eo:
	DBG_LOG("Odd number of flipped edges\n");
	return false;
issolvable_co:
	DBG_LOG("Sum of corner orientation is not multiple of 3\n");
	return false;
}

bool
issolved(cube_t cube)
{
	return equal(cube, solved);
}

bool
equal(cube_t c1, cube_t c2)
{
	int i;
	bool ret;

	ret = true;
	for (i = 0; i < 8; i++)
		ret = ret && c1.corner[i] == c2.corner[i];
	for (i = 0; i < 12; i++)
		ret = ret && c1.edge[i] == c2.edge[i];

	return ret;
}

bool
iserror(cube_t cube)
{
	return equal(cube, zero);
}

cube_t
compose(cube_t c1, cube_t c2)
{
	DBG_ASSERT(isconsistent(c1) && isconsistent(c2),
	    zero, "compose error: inconsistent cube\n")

	return fasttocube(compose_fast(cubetofast(c1), cubetofast(c2)));
}

cube_t
inverse(cube_t cube)
{
	cube_t ret;
	uint8_t i, piece, orien;

	DBG_ASSERT(isconsistent(cube), zero,
	    "inverse error: inconsistent cube\n");

	ret = zero;

	for (i = 0; i < 12; i++) {
		piece = cube.edge[i];
		orien = piece & _eobit;
		ret.edge[piece & _pbits] = i | orien;
	}

	for (i = 0; i < 8; i++) {
		piece = cube.corner[i];
		orien = ((piece << 1) | (piece >> 1)) & _cobits2;
		ret.corner[piece & _pbits] = i | orien;
	}

	return ret;
}

cube_t
applymoves(cube_t cube, char *buf)
{
	cube_fast_t fast;
	uint8_t r, m;
	char *b;

	DBG_ASSERT(isconsistent(cube), zero,
	    "move error: inconsistent cube\n");

	fast = cubetofast(cube);

	for (b = buf; *b != '\0'; b++) {
		while (*b == ' ' || *b == '\t' || *b == '\n')
			b++;
		if (*b == '\0')
			goto applymoves_finish;
		if ((r = readmove(*b)) == _error)
			goto applymoves_error;
		if ((m = readmodifier(*(b+1))) != 0)
			b++;
		fast = move(fast, r + m);
	}

applymoves_finish:
	return fasttocube(fast);

applymoves_error:
	DBG_LOG("applymoves error\n");
	return zero;
}

cube_t
applytrans(cube_t cube, char *buf)
{
	cube_fast_t fast;
	uint8_t t;

	DBG_ASSERT(isconsistent(cube), zero,
	    "transformation error: inconsistent cube\n");

	t = readtrans(buf);
	fast = cubetofast(cube);
	fast = transform(fast, t);

	return fasttocube(fast);
}

cube_t
readcube(char *format, char *buf)
{
	cube_t cube;

	if (!strcmp(format, "H48")) {
		cube = readcube_H48(buf);
	} else if (!strcmp(format, "LST")) {
		cube = readcube_LST(buf);
	} else {
		DBG_LOG("Cannot read cube in the given format\n");
		cube = zero;
	}

	return cube;
}

void
writecube(char *format, cube_t cube, char *buf)
{
	char *errormsg;
	size_t len;

	if (!isconsistent(cube)) {
		errormsg = "ERROR: cannot write inconsistent cube";
		goto writecube_error;
	}

	if (!strcmp(format, "H48")) {
		writecube_H48(cube, buf);
	} else if (!strcmp(format, "LST")) {
		writecube_LST(cube, buf);
	} else {
		errormsg = "ERROR: cannot write cube in the given format";
		goto writecube_error;
	}

	return;

writecube_error:
	DBG_LOG("writecube error, see stdout for details\n");
	len = strlen(errormsg);
	memcpy(buf, errormsg, len);
	buf[len] = '\n';
	buf[len+1] = '\0';
}

_static int
permsign(uint8_t *a, int n)
{
	int i, j;
	uint8_t ret = 0;

	for (i = 0; i < n; i++)
		for (j = i+1; j < n; j++)
			ret += a[i] > a[j] ? 1 : 0;

	return ret % 2;
}

_static uint8_t
readco(char *str)
{
	if (*str == '0')
		return 0;
	if (*str == '1')
		return _ctwist_cw;
	if (*str == '2')
		return _ctwist_ccw;

	DBG_LOG("Error reading CO\n");
	return _error;
}

_static uint8_t
readcp(char *str)
{
	uint8_t c;

	for (c = 0; c < 8; c++)
		if (!strncmp(str, cornerstr[c], 3) ||
		    !strncmp(str, cornerstralt[c], 3))
			return c;

	DBG_LOG("Error reading CP\n");
	return _error;
}

_static uint8_t
readeo(char *str)
{
	if (*str == '0')
		return 0;
	if (*str == '1')
		return _eflip;

	DBG_LOG("Error reading EO\n");
	return _error;
}

_static uint8_t
readep(char *str)
{
	uint8_t e;

	for (e = 0; e < 12; e++)
		if (!strncmp(str, edgestr[e], 2))
			return e;

	DBG_LOG("Error reading EP\n");
	return _error;
}

_static cube_t
readcube_H48(char *buf)
{
	int i;
	uint8_t piece, orient;
	cube_t ret = {0};
	char *b;
	
	b = buf;

	for (i = 0; i < 12; i++) {
		while (*b == ' ' || *b == '\t' || *b == '\n')
			b++;
		if ((piece = readep(b)) == _error)
			return zero;
		b += 2;
		if ((orient = readeo(b)) == _error)
			return zero;
		b++;
		ret.edge[i] = piece | orient;
	}
	for (i = 0; i < 8; i++) {
		while (*b == ' ' || *b == '\t' || *b == '\n')
			b++;
		if ((piece = readcp(b)) == _error)
			return zero;
		b += 3;
		if ((orient = readco(b)) == _error)
			return zero;
		b++;
		ret.corner[i] = piece | orient;
	}

	return ret;
}

_static uint8_t
readpiece_LST(char **b)
{
	uint8_t ret;
	bool read;

	while (**b == ',' || **b == ' ' || **b == '\t' || **b == '\n')
		(*b)++;

	for (ret = 0, read = false; **b >= '0' && **b <= '9'; (*b)++) {
		read = true;
		ret = ret * 10 + (**b) - '0';
	}

	return read ? ret : _error;
}

_static cube_t
readcube_LST(char *buf)
{
	int i;
	cube_t ret = {0};

	for (i = 0; i < 8; i++)
		ret.corner[i] = readpiece_LST(&buf);

	for (i = 0; i < 12; i++)
		ret.edge[i] = readpiece_LST(&buf);

	return ret;
}

_static int
writepiece_LST(uint8_t piece, char *buf)
{
	char digits[3];
	int i, len = 0;

	while (piece != 0) {
		digits[len++] = (piece % 10) + '0';
		piece /= 10;
	}

	if (len == 0)
		digits[len++] = '0';

	for (i = 0; i < len; i++)
		buf[i] = digits[len-i-1];

	buf[len] = ',';
	buf[len+1] = ' ';

	return len+2;
}

_static void
writecube_H48(cube_t cube, char *buf)
{
	uint8_t piece, perm, orient;
	int i;

	for (i = 0; i < 12; i++) {
		piece = cube.edge[i];
		perm = piece & _pbits;
		orient = (piece & _eobit) >> _eoshift;
		buf[4*i    ] = edgestr[perm][0];
		buf[4*i + 1] = edgestr[perm][1];
		buf[4*i + 2] = orient + '0';
		buf[4*i + 3] = ' ';
	}
	for (i = 0; i < 8; i++) {
		piece = cube.corner[i];
		perm = piece & _pbits;
		orient = (piece & _cobits) >> _coshift;
		buf[48 + 5*i    ] = cornerstr[perm][0];
		buf[48 + 5*i + 1] = cornerstr[perm][1];
		buf[48 + 5*i + 2] = cornerstr[perm][2];
		buf[48 + 5*i + 3] = orient + '0';
		buf[48 + 5*i + 4] = ' ';
	}

	buf[48+39] = '\0';
}

_static void
writecube_LST(cube_t cube, char *buf)
{
	int i, ptr;
	uint8_t piece;

	ptr = 0;

	for (i = 0; i < 8; i++) {
		piece = cube.corner[i];
		ptr += writepiece_LST(piece, buf + ptr);
	}

	for (i = 0; i < 12; i++) {
		piece = cube.edge[i];
		ptr += writepiece_LST(piece, buf + ptr);
	}

	*(buf+ptr-2) = 0;
}

_static uint8_t
readmove(char c)
{
	switch (c) {
	case 'U':
		return U;
	case 'D':
		return D;
	case 'R':
		return R;
	case 'L':
		return L;
	case 'F':
		return F;
	case 'B':
		return B;
	default:
		return _error;
	}
}

_static uint8_t
readmodifier(char c)
{
	switch (c) {
	case '1': /* Fallthrough */
	case '2': /* Fallthrough */
	case '3':
		return c - '0' - 1;
	case '\'':
		return 2;
	default:
		return 0;
	}
}

_static uint8_t
readtrans(char *buf)
{
	uint8_t t;

	for (t = 0; t < 48; t++)
		if (!strncmp(buf, transstr[t], 11))
			return t;

	DBG_LOG("readtrans error\n");
	return _error;
}

_static int
writemoves(uint8_t *m, int n, char *buf)
{
	int i;
	size_t len;
	char *b, *s;

	for (i = 0, b = buf; i < n; i++, b++) {
		s = movestr[m[i]];
		len = strlen(s);
		memcpy(b, s, len);
		b += len;	
		*b = ' ';
	}

	if (b != buf)
		b--; /* Remove last space */
	*b = '\0';

	return b - buf;
}

_static void
writetrans(uint8_t t, char *buf)
{
	if (t >= 48)
		memcpy(buf, "error trans", 11);
	else
		memcpy(buf, transstr[t], 11);
	buf[11] = '\0';
}

_static cube_fast_t
move(cube_fast_t c, uint8_t m)
{
	switch (m) {
	case U:
		return _move(U, c);
	case U2:
		return _move(U2, c);
	case U3:
		return _move(U3, c);
	case D:
		return _move(D, c);
	case D2:
		return _move(D2, c);
	case D3:
		return _move(D3, c);
	case R:
		return _move(R, c);
	case R2:
		return _move(R2, c);
	case R3:
		return _move(R3, c);
	case L:
		return _move(L, c);
	case L2:
		return _move(L2, c);
	case L3:
		return _move(L3, c);
	case F:
		return _move(F, c);
	case F2:
		return _move(F2, c);
	case F3:
		return _move(F3, c);
	case B:
		return _move(B, c);
	case B2:
		return _move(B2, c);
	case B3:
		return _move(B3, c);
	default:
		DBG_LOG("move error, unknown move\n");
		return zero_fast;
	}
}

_static cube_fast_t
transform(cube_fast_t c, uint8_t t)
{
	switch (t) {
	case UFr:
		return _trans_rotation(UFr, c);
	case ULr:
		return _trans_rotation(ULr, c);
	case UBr:
		return _trans_rotation(UBr, c);
	case URr:
		return _trans_rotation(URr, c);
	case DFr:
		return _trans_rotation(DFr, c);
	case DLr:
		return _trans_rotation(DLr, c);
	case DBr:
		return _trans_rotation(DBr, c);
	case DRr:
		return _trans_rotation(DRr, c);
	case RUr:
		return _trans_rotation(RUr, c);
	case RFr:
		return _trans_rotation(RFr, c);
	case RDr:
		return _trans_rotation(RDr, c);
	case RBr:
		return _trans_rotation(RBr, c);
	case LUr:
		return _trans_rotation(LUr, c);
	case LFr:
		return _trans_rotation(LFr, c);
	case LDr:
		return _trans_rotation(LDr, c);
	case LBr:
		return _trans_rotation(LBr, c);
	case FUr:
		return _trans_rotation(FUr, c);
	case FRr:
		return _trans_rotation(FRr, c);
	case FDr:
		return _trans_rotation(FDr, c);
	case FLr:
		return _trans_rotation(FLr, c);
	case BUr:
		return _trans_rotation(BUr, c);
	case BRr:
		return _trans_rotation(BRr, c);
	case BDr:
		return _trans_rotation(BDr, c);
	case BLr:
		return _trans_rotation(BLr, c);
	case UFm:
		return _trans_mirrored(UFm, c);
	case ULm:
		return _trans_mirrored(ULm, c);
	case UBm:
		return _trans_mirrored(UBm, c);
	case URm:
		return _trans_mirrored(URm, c);
	case DFm:
		return _trans_mirrored(DFm, c);
	case DLm:
		return _trans_mirrored(DLm, c);
	case DBm:
		return _trans_mirrored(DBm, c);
	case DRm:
		return _trans_mirrored(DRm, c);
	case RUm:
		return _trans_mirrored(RUm, c);
	case RFm:
		return _trans_mirrored(RFm, c);
	case RDm:
		return _trans_mirrored(RDm, c);
	case RBm:
		return _trans_mirrored(RBm, c);
	case LUm:
		return _trans_mirrored(LUm, c);
	case LFm:
		return _trans_mirrored(LFm, c);
	case LDm:
		return _trans_mirrored(LDm, c);
	case LBm:
		return _trans_mirrored(LBm, c);
	case FUm:
		return _trans_mirrored(FUm, c);
	case FRm:
		return _trans_mirrored(FRm, c);
	case FDm:
		return _trans_mirrored(FDm, c);
	case FLm:
		return _trans_mirrored(FLm, c);
	case BUm:
		return _trans_mirrored(BUm, c);
	case BRm:
		return _trans_mirrored(BRm, c);
	case BDm:
		return _trans_mirrored(BDm, c);
	case BLm:
		return _trans_mirrored(BLm, c);
	default:
		DBG_LOG("transform error, unknown transformation\n");
		return zero_fast;
	}
}

/******************************************************************************
Section: moves, move sequences and transformations

This section contains methods to work with moves and arrays of moves. They
do not rely on the cube structure.
******************************************************************************/

_static_inline uint8_t inverse_trans(uint8_t);
_static_inline uint8_t movebase(uint8_t);
_static_inline uint8_t moveaxis(uint8_t);

_static_inline uint8_t
inverse_trans(uint8_t t)
{
	return inverse_trans_table[t];
}

_static_inline uint8_t
movebase(uint8_t move)
{
	return move / 3;
}

_static_inline uint8_t
moveaxis(uint8_t move)
{
	return move / 6;
}