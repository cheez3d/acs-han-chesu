#include "bitboard.h"

#include "board.h"
#include "xboard.h"

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>

/**************
 * REFERENCES *
 **************
 * 
 * Bitboard calculator:
 * http://cinnamonchess.altervista.org/bitboard_calculator/Calc.html
 * 
 * Magic bitboards:
 * http://pradu.us/old/Nov27_2008/Buzz/research/magic/Bitboards.pdf
 * https://rhysre.net/2019/01/15/magic-bitboards.html
 * https://www.chessprogramming.org/Magic_Bitboards
 * 
 */

const bb_t BB_EMPTY = (bb_t)0;
const bb_t BB_ONE   = (bb_t)1;

/**
 * Array index for accessing magic number for a particular
 * sliding piece on a particular square.
 */
enum bb_magics_idx {
    BB_MAGICS_ROOK, BB_MAGICS_BISHOP,

    BB_MAGICS_SZ, // array size
};

/**
 * Array of magic numbers used for hash generation.
 */
static const bb_t BB_MAGICS[BB_MAGICS_SZ][SQ_CNT] = {
    /** These were generated using the code found here:
     * https://www.chessprogramming.org/Looking_for_Magics
     * 
     * Seed used for generation: 1583534401U
     */

    [BB_MAGICS_ROOK] = {
        (bb_t)0x0080102080004001, (bb_t)0x1240001000200441,
        (bb_t)0x4100100820010041, (bb_t)0x2080100008008004,
        (bb_t)0x2200080200102004, (bb_t)0x0200080200812430,
        (bb_t)0x1480210020800200, (bb_t)0x0500020030804100,
        (bb_t)0x8040800040008020, (bb_t)0x0000802000400082,
        (bb_t)0x0010802000801000, (bb_t)0x0981001001002008,
        (bb_t)0x4000808004000800, (bb_t)0x0082000802000410,
        (bb_t)0x0004000102049008, (bb_t)0x0001001088510012,
        (bb_t)0x0020008080004000, (bb_t)0x0050004040002000,
        (bb_t)0x0c90008010802000, (bb_t)0x1010010008210010,
        (bb_t)0x0101030008001004, (bb_t)0x8000808004000200,
        (bb_t)0x000004000801a250, (bb_t)0x0001460000844304,
        (bb_t)0x1100401880002080, (bb_t)0x0221028300400220,
        (bb_t)0x2410040020002800, (bb_t)0x2020400a00220010,
        (bb_t)0x0088020040040041, (bb_t)0x2109000300240008,
        (bb_t)0x5400080400108102, (bb_t)0x0400028200210a44,
        (bb_t)0x4380002000c00045, (bb_t)0x2210002000404003,
        (bb_t)0x0000408202001021, (bb_t)0x8004840800801000,
        (bb_t)0x8808001101000408, (bb_t)0x0022001002000804,
        (bb_t)0x8082102804002d32, (bb_t)0x48492100420000a4,
        (bb_t)0x0080804000618000, (bb_t)0x8840402010004003,
        (bb_t)0x0000200010008080, (bb_t)0x04020040200a0010,
        (bb_t)0x0080100801010004, (bb_t)0x2104008002008004,
        (bb_t)0x0000100802640041, (bb_t)0x00140048a1020014,
        (bb_t)0x000042800101b500, (bb_t)0x2040004090200080,
        (bb_t)0x4130102000410500, (bb_t)0x1000200810010100,
        (bb_t)0x0008800800040080, (bb_t)0x1401000802040100,
        (bb_t)0x00c810490a088400, (bb_t)0x8800240100a04200,
        (bb_t)0x8981004020800011, (bb_t)0x20444002a1708101,
        (bb_t)0x8002001044082082, (bb_t)0x1000100021000409,
        (bb_t)0x0001001004080023, (bb_t)0x4012002410c80102,
        (bb_t)0x0042000088014402, (bb_t)0x00100410a70080c2,
    },

    [BB_MAGICS_BISHOP] = {
        (bb_t)0x0110900909082200, (bb_t)0x9004012424008000,
        (bb_t)0x804ca80881008080, (bb_t)0x0548260040400000,
        (bb_t)0x0844504095008060, (bb_t)0xc005014940000000,
        (bb_t)0x0292011023101000, (bb_t)0x0400405208014000,
        (bb_t)0x800804c4c8021402, (bb_t)0x0018200404509021,
        (bb_t)0x0000880084009450, (bb_t)0x0300090405010231,
        (bb_t)0x0512040504000010, (bb_t)0x0131020104220420,
        (bb_t)0x8841041088080a00, (bb_t)0x0000410c1b040217,
        (bb_t)0x5110484024090424, (bb_t)0x0810821410021160,
        (bb_t)0x08080021060c0010, (bb_t)0x0d08000520404020,
        (bb_t)0x8001000820080904, (bb_t)0x0580201410080810,
        (bb_t)0x0050800402015010, (bb_t)0x42415000240c040c,
        (bb_t)0x2214e10004881013, (bb_t)0x4008020008100124,
        (bb_t)0x432e120020408400, (bb_t)0x00a8101008004090,
        (bb_t)0x0001014004044000, (bb_t)0x0042002002009004,
        (bb_t)0x200c006101080208, (bb_t)0x02a1010104440081,
        (bb_t)0x8082084040041014, (bb_t)0x0182682000041102,
        (bb_t)0x0012003400020800, (bb_t)0x2020020082080080,
        (bb_t)0x5000610040240040, (bb_t)0x40100a0280003000,
        (bb_t)0x2428108100040126, (bb_t)0x0308208020008208,
        (bb_t)0x2841101004041040, (bb_t)0x0000421084005090,
        (bb_t)0x8091044022001000, (bb_t)0x4010042018004101,
        (bb_t)0x4120400491000208, (bb_t)0x4042140800200200,
        (bb_t)0x082048010100c04c, (bb_t)0x110200a202808210,
        (bb_t)0x04a0909008200900, (bb_t)0x40404d0801104805,
        (bb_t)0x4205304044100020, (bb_t)0x0140400884040002,
        (bb_t)0x0006409102120100, (bb_t)0x0000440808284080,
        (bb_t)0x102208024084000a, (bb_t)0x50281800c0820000,
        (bb_t)0x4000a10802300280, (bb_t)0x808080240104110a,
        (bb_t)0x4103002211008800, (bb_t)0x4000701080208800,
        (bb_t)0x0000101050020884, (bb_t)0x8000b04002043100,
        (bb_t)0x0038082001220200, (bb_t)0x8008202104010118,
    },
};

bb_t bb_ranks[RK_CNT];

static void bb_init_ranks(void) {
    for (enum rank r = 0; r < RK_CNT; ++r) {
        bb_ranks[r] = (bb_t)0x00000000000000ff << FL_CNT*r;
    }
}

bb_t bb_files[FL_CNT];

static void bb_init_files(void) {
    for (enum file f = 0; f < FL_CNT; ++f) {
        bb_files[f] = (bb_t)0x101010101010101 << f;
    }
}

bb_t bb_squares[SQ_CNT];

static void bb_init_squares(void) {
    for (enum square s = 0; s < SQ_CNT; ++s) {
        bb_squares[s] = BB_ONE << s;
    }
}

bb_t bb_pieces[COLOR_CNT][PIECE_CNT];

static void bb_init_pieces(void) {
    bb_pieces[WHITE][BB_ROOKS]   = bb_squares[SQ_A1] | bb_squares[SQ_H1];
    bb_pieces[WHITE][BB_KNIGHTS] = bb_squares[SQ_B1] | bb_squares[SQ_G1];
    bb_pieces[WHITE][BB_BISHOPS] = bb_squares[SQ_C1] | bb_squares[SQ_F1];
    bb_pieces[WHITE][BB_QUEENS]  = bb_squares[SQ_D1];
    bb_pieces[WHITE][BB_KING]    = bb_squares[SQ_E1];
    bb_pieces[WHITE][BB_PAWNS]   = bb_ranks[RK_2];

    bb_pieces[BLACK][BB_ROOKS]   = bb_squares[SQ_A8] | bb_squares[SQ_H8];
    bb_pieces[BLACK][BB_KNIGHTS] = bb_squares[SQ_B8] | bb_squares[SQ_G8];
    bb_pieces[BLACK][BB_BISHOPS] = bb_squares[SQ_C8] | bb_squares[SQ_F8];
    bb_pieces[BLACK][BB_QUEENS]  = bb_squares[SQ_D8];
    bb_pieces[BLACK][BB_KING]    = bb_squares[SQ_E8];
    bb_pieces[BLACK][BB_PAWNS]   = bb_ranks[RK_7];
}

/**
 * Represents array index for accessing ray bitboard
 * for a particular direction for a particular square.
 */
enum bb_rays_idx {
    BB_RAYS_N, BB_RAYS_S, BB_RAYS_E, BB_RAYS_W,
    BB_RAYS_NE, BB_RAYS_SE, BB_RAYS_NW, BB_RAYS_SW,

    BB_RAYS_SZ, // array size
};

/**
 * Array for converting from direction and square to bitboard ray.
 */
static bb_t bb_rays[BB_RAYS_SZ][SQ_CNT];

static void bb_init_rays(void) {
    for (enum square s = 0; s < SQ_CNT; ++s) {
        // north ray for current square
        bb_rays[BB_RAYS_N][s] = (bb_t)0x0101010101010100 << s;

        // south ray for current square
        bb_rays[BB_RAYS_S][s] = (bb_t)0x0080808080808080 >> (SQ_CNT-1-s);

        // east ray for current square
        bb_rays[BB_RAYS_E][s] = (bb_squares[s | (FL_CNT-1)]-bb_squares[s]) << 1;

        // west ray for current square
        bb_rays[BB_RAYS_W][s] = bb_squares[s]-bb_squares[s & (SQ_CNT-FL_CNT)];

        // north east ray for current square
        {
            bb_t bb = (bb_t)0x8040201008040200;

            for (enum file f = square_to_file(s); f-- > 0; ) {
                bb = (bb << 1) & ~bb_files[0];
            }

            bb_rays[BB_RAYS_NE][s] = bb << FL_CNT*square_to_rank(s);
        }

        // south east ray for current square
        {
            bb_t bb = (bb_t)0x0002040810204080;

            for (enum file f = square_to_file(s); f-- > 0; ) {
                bb = (bb << 1) & ~bb_files[0];
            }

            bb_rays[BB_RAYS_SE][s] = bb >> FL_CNT*(FL_CNT-1-square_to_rank(s));
        }

        // north west ray for current square
        {
            bb_t bb = (bb_t)0x0102040810204000;

            for (enum file f = FL_CNT-1-square_to_file(s); f-- > 0; ) {
                bb = (bb >> 1) & ~bb_files[FL_CNT-1];
            }

            bb_rays[BB_RAYS_NW][s] = bb << FL_CNT*square_to_rank(s);
        }

        // south west ray for current square
        {
            bb_t bb = (bb_t)0x0040201008040201;

            for (enum file f = FL_CNT-1-square_to_file(s); f-- > 0; ) {
                bb = (bb >> 1) & ~bb_files[FL_CNT-1];
            }

            bb_rays[BB_RAYS_SW][s] = bb >> FL_CNT*(FL_CNT-1-square_to_rank(s));
        }
    }
}

/**
 * Array for converting from piece type and square to an occupancy mask that
 * will be used to isolate the relevant occupancy bits that are needed
 * for calculating the hash value through which we will be able to look up
 * the attack set.
 */
static bb_t bb_occ_masks[BB_MAGICS_SZ][SQ_CNT];

/**
 * Array for storing the amount of set bits for each occupancy mask
 * in the array above.
 */
static int bb_occ_masks_bit_cnt[BB_MAGICS_SZ][SQ_CNT];

static void bb_init_occ_masks(void) {
    // rook masks
    for (enum square s = 0; s < SQ_CNT; ++s) {
        bb_t bb_rook_mask = (bb_rays[BB_RAYS_N][s] & ~bb_ranks[RK_8])
                          | (bb_rays[BB_RAYS_S][s] & ~bb_ranks[RK_1])
                          | (bb_rays[BB_RAYS_E][s] & ~bb_files[FL_H])
                          | (bb_rays[BB_RAYS_W][s] & ~bb_files[FL_A]);

        bb_occ_masks[BB_MAGICS_ROOK][s] = bb_rook_mask;

        bb_occ_masks_bit_cnt[BB_MAGICS_ROOK][s] = bb_bit_cnt(bb_rook_mask);
    }

    bb_t bb_edge = bb_ranks[RK_1] | bb_ranks[RK_8]
                 | bb_files[FL_A] | bb_files[FL_H];

    // bishop masks
    for (enum square s = 0; s < SQ_CNT; ++s) {
        bb_t bb_bishop_mask = bb_rays[BB_RAYS_NE][s]
                            | bb_rays[BB_RAYS_SE][s]
                            | bb_rays[BB_RAYS_NW][s]
                            | bb_rays[BB_RAYS_SW][s];

        bb_bishop_mask &= ~bb_edge;

        bb_occ_masks[BB_MAGICS_BISHOP][s] = bb_bishop_mask;

        bb_occ_masks_bit_cnt[BB_MAGICS_BISHOP][s] = bb_bit_cnt(bb_bishop_mask);
    }
}

/**
 * Array for mapping occupancies to attack sets through their hash.
 */
static bb_t *bb_sliding_attacks[BB_MAGICS_SZ][SQ_CNT];

/**
 * Given the occupancy mask, generate the occupancy possibility `p`.
 * (i.e. for each bit in the occupancy mask we can either set it on or off)
 */
static bb_t bb_gen_occ(bb_t bb_occ_mask, int bb_occ_mask_bit_cnt, int p) {
    assert(p < (1 << bb_occ_mask_bit_cnt));

    bb_t bb_occ = BB_EMPTY;

    for (int bit = 0; bit < bb_occ_mask_bit_cnt; ++bit) {
        enum square s = bb_pop_lsb(&bb_occ_mask);

        if ((1 << bit) & p) {
            bb_occ |= bb_squares[s];
        }
    }

    return bb_occ;
}

/**
 * Given the square and the occupancy, generate the root attack set.
 */
static bb_t bb_gen_rook_attacks(enum square s, bb_t bb_occ) {
    assert(s >= 0 && s < SQ_CNT);

    bb_t bb_attacks = BB_EMPTY;

    // north ray
    {
        bb_attacks |= bb_rays[BB_RAYS_N][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_N][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_N][bb_scan_lsb(bb_occ_ray)];
        }
    }

    // south ray
    {
        bb_attacks |= bb_rays[BB_RAYS_S][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_S][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_S][bb_scan_msb(bb_occ_ray)];
        }
    }

    // east ray
    {
        bb_attacks |= bb_rays[BB_RAYS_E][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_E][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_E][bb_scan_lsb(bb_occ_ray)];
        }
    }

    // west ray
    {
        bb_attacks |= bb_rays[BB_RAYS_W][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_W][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_W][bb_scan_msb(bb_occ_ray)];
        }
    }

    return bb_attacks;
}

/**
 * Given the square and the occupancy, generate the bishop attack set.
 */
static bb_t bb_gen_bishop_attacks(enum square s, bb_t bb_occ) {
    assert(s >= 0 && s < SQ_CNT);

    bb_t bb_attacks = BB_EMPTY;

    // north east ray
    {
        bb_attacks |= bb_rays[BB_RAYS_NE][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_NE][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_NE][bb_scan_lsb(bb_occ_ray)];
        }
    }

    // south east ray
    {
        bb_attacks |= bb_rays[BB_RAYS_SE][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_SE][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_SE][bb_scan_msb(bb_occ_ray)];
        }
    }

    // north west ray
    {
        bb_attacks |= bb_rays[BB_RAYS_NW][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_NW][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_NW][bb_scan_lsb(bb_occ_ray)];
        }
    }

    // south west ray
    {
        bb_attacks |= bb_rays[BB_RAYS_SW][s];
        bb_t bb_occ_ray = bb_occ & bb_rays[BB_RAYS_SW][s];

        if (bb_occ_ray) {
            bb_attacks &= ~bb_rays[BB_RAYS_SW][bb_scan_msb(bb_occ_ray)];
        }
    }

    return bb_attacks;
}

static void bb_init_sliding_attacks(void) {
    bb_t (* const BB_ATTACKS_GEN[BB_MAGICS_SZ])(enum square, bb_t) = {
        [BB_MAGICS_ROOK]   = bb_gen_rook_attacks,
        [BB_MAGICS_BISHOP] = bb_gen_bishop_attacks
    };

    for (enum bb_magics_idx i = 0; i < BB_MAGICS_SZ; ++i) {
        for (enum square s = 0; s < SQ_CNT; ++s) {
            bb_t bb_occ_mask = bb_occ_masks[i][s];
            int bb_occ_mask_bit_cnt = bb_occ_masks_bit_cnt[i][s];

            // number of occupancy posibilities for occupancy mask
            int p_cnt = 1 << bb_occ_mask_bit_cnt;

            bb_sliding_attacks[i][s] = malloc(p_cnt*sizeof(*bb_sliding_attacks[i][s]));

            if (bb_sliding_attacks[i][s] == NULL) {
                error(EXIT_FAILURE, errno, "could not allocate space for sliding attack sets");
            }

            // generate all occupancy possibilities for the occupancy mask
            // and generate the attack set for each occupancy possibility
            for (int p = 0; p < p_cnt; ++p) {
                bb_t bb_occ = bb_gen_occ(bb_occ_mask, bb_occ_mask_bit_cnt, p);

                int hash = bb_occ*BB_MAGICS[i][s] >> (8*sizeof(bb_t)-bb_occ_mask_bit_cnt);

                bb_sliding_attacks[i][s][hash] = BB_ATTACKS_GEN[i](s, bb_occ);
            }
        }
    }
}

static void bb_term_sliding_attacks(void) {
    for (enum bb_magics_idx i = 0; i < BB_MAGICS_SZ; ++i) {
        for (enum square s = 0; s < SQ_CNT; ++s) {
            free(bb_sliding_attacks[i][s]);
        }
    }
}

/**
 * Array for storing knight attack sets for each square.
 */
static bb_t bb_knight_attacks[SQ_CNT];

static void bb_init_knight_attacks(void) {
    for (enum square s = 0; s < SQ_CNT; ++s) {
        bb_t bb = bb_squares[s];

        bb_knight_attacks[s] = ((bb >> (2*FL_CNT-1) | bb << (2*FL_CNT+1)) & ~bb_files[FL_A])
                             | ((bb >> (FL_CNT-2) | bb << (FL_CNT+2)) & ~(bb_files[FL_A] | bb_files[FL_B]))
                             | ((bb >> (2*FL_CNT+1) | bb << (2*FL_CNT-1)) & ~bb_files[FL_H])
                             | ((bb >> (FL_CNT+2) | bb << (FL_CNT-2)) & ~(bb_files[FL_H] | bb_files[FL_G]));
    }
}

/**
 * Array for storing king attack sets for each square.
 */
static bb_t bb_king_attacks[SQ_CNT];

static void bb_init_king_attacks(void) {
    for (enum square s = 0; s < SQ_CNT; ++s) {
        bb_t bb = bb_squares[s];

        bb_king_attacks[s] = bb << FL_CNT
                           | bb >> FL_CNT
                           | ((bb >> (FL_CNT+1) | bb >> 1 | bb << (FL_CNT-1)) & ~bb_files[FL_H])
                           | ((bb >> (FL_CNT-1) | bb << 1 | bb << (FL_CNT+1)) & ~bb_files[FL_A]);
    }
}

/**
 * Array for storing pawn attack sets for each color for each square.
 */
static bb_t bb_pawn_attacks[COLOR_CNT][SQ_CNT];

static void bb_init_pawn_attacks(void) {
    for (enum square s = SQ_A2; s <= SQ_H7; ++s) {
        bb_t bb = bb_squares[s];

        bb_pawn_attacks[WHITE][s] = (bb << (FL_CNT-1) & ~bb_files[FL_H])
                                  | (bb << (FL_CNT+1) & ~bb_files[FL_A]);

        bb_pawn_attacks[BLACK][s] = (bb >> (FL_CNT+1) & ~bb_files[FL_H])
                                  | (bb >> (FL_CNT-1) & ~bb_files[FL_A]);
    }
}

void bb_init(void) {
    bb_init_ranks();
    bb_init_files();
    bb_init_squares();

    bb_init_pieces();

    bb_init_rays();
    bb_init_occ_masks();
    bb_init_sliding_attacks();

    bb_init_knight_attacks();
    bb_init_king_attacks();
    bb_init_pawn_attacks();
}

void bb_term(void) {
    bb_term_sliding_attacks();
}

static bb_t bb_get_sliding_attacks(enum bb_magics_idx idx, enum square s, bb_t bb_occ) {
    assert(idx >= 0 && idx < BB_MAGICS_SZ);

    bb_occ &= bb_occ_masks[idx][s];

    int hash = bb_occ*BB_MAGICS[idx][s] >> (8*sizeof(bb_t)-bb_occ_masks_bit_cnt[idx][s]);

    return bb_sliding_attacks[idx][s][hash];
}

bb_t bb_get_attacks(enum color c, enum piece p, enum square s, bb_t bb_occ) {
    assert(c >= 0 && c < COLOR_CNT);
    assert(p >= 0 && p < PIECE_CNT);
    assert(s >= 0 && s < SQ_CNT);

    switch (p) {
    case ROOK:
        return bb_get_sliding_attacks(BB_MAGICS_ROOK, s, bb_occ);

    case KNIGHT:
        return bb_knight_attacks[s];

    case BISHOP:
        return bb_get_sliding_attacks(BB_MAGICS_BISHOP, s, bb_occ);

    case QUEEN:
        return bb_get_sliding_attacks(BB_MAGICS_ROOK, s, bb_occ)
             | bb_get_sliding_attacks(BB_MAGICS_BISHOP, s, bb_occ);

    case KING:
        return bb_king_attacks[s];

    case PAWN:
        return bb_pawn_attacks[c][s];

    default:
        return BB_EMPTY;
    }
}

void bb_print(bb_t bb) {
    for (enum rank r = RK_CNT; r-- > 0; ) {
        for (enum file f = 0; f < FL_CNT; ++f) {
            enum square s = rank_file_to_square(r, f);

            xb_comment("%c ", bb & bb_squares[s] ? 'x' : '.');
        }

        xb_comment("%c\n", rank_to_char(r));
    }

    for (enum file f = 0; f < FL_CNT; ++f) {
        xb_comment("%c ", file_to_char(f));
    }

    xb_comment("\n");
}

void bb_print_fancy(bb_t bb) {
    for (enum rank r = RK_CNT; r-- > 0; ) {
        xb_commentln("+---+---+---+---+---+---+---+---+   ");

        for (enum file f = 0; f < FL_CNT; ++f) {
            enum square s = rank_file_to_square(r, f);

            xb_comment("| %c ", bb & bb_squares[s] ? 'x' : ' ');
        }

        xb_comment("| %c \n", rank_to_char(r));
    }

    xb_commentln("+---+---+---+---+---+---+---+---+   ");

    for (enum file f = 0; f < FL_CNT; ++f) {
        xb_comment("  %c ", file_to_char(f));
    }

    xb_comment("    \n");
}
