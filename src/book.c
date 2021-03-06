#include "book.h"

#include "bitboard.h"
#include "board.h"
#include "move.h"

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BOOK_FILE_NAME "book.bin"

typedef uint_fast64_t bk_key_t;

struct __attribute__((packed)) bk_entry {
    bk_key_t key;

    union {
        struct __attribute__((packed)) {
            unsigned int to_file   : 3;
            unsigned int to_rank   : 3;
            unsigned int from_file : 3;
            unsigned int from_rank : 3;
            unsigned int promotion : 3;
        };

        uint16_t move;
    };
};

struct book {
    struct bk_entry *base;

    size_t count;
};

static const bk_key_t BK_KEY_EMPTY = (bk_key_t)0;

static const bk_key_t bk_rand_piece[] = {
    (bk_key_t)0x9d39247e33776d41, (bk_key_t)0x2af7398005aaa5c7,
    (bk_key_t)0x44db015024623547, (bk_key_t)0x9c15f73e62a76ae2,
    (bk_key_t)0x75834465489c0c89, (bk_key_t)0x3290ac3a203001bf,
    (bk_key_t)0x0fbbad1f61042279, (bk_key_t)0xe83a908ff2fb60ca,
    (bk_key_t)0x0d7e765d58755c10, (bk_key_t)0x1a083822ceafe02d,
    (bk_key_t)0x9605d5f0e25ec3b0, (bk_key_t)0xd021ff5cd13a2ed5,
    (bk_key_t)0x40bdf15d4a672e32, (bk_key_t)0x011355146fd56395,
    (bk_key_t)0x5db4832046f3d9e5, (bk_key_t)0x239f8b2d7ff719cc,
    (bk_key_t)0x05d1a1ae85b49aa1, (bk_key_t)0x679f848f6e8fc971,
    (bk_key_t)0x7449bbff801fed0b, (bk_key_t)0x7d11cdb1c3b7adf0,
    (bk_key_t)0x82c7709e781eb7cc, (bk_key_t)0xf3218f1c9510786c,
    (bk_key_t)0x331478f3af51bbe6, (bk_key_t)0x4bb38de5e7219443,
    (bk_key_t)0xaa649c6ebcfd50fc, (bk_key_t)0x8dbd98a352afd40b,
    (bk_key_t)0x87d2074b81d79217, (bk_key_t)0x19f3c751d3e92ae1,
    (bk_key_t)0xb4ab30f062b19abf, (bk_key_t)0x7b0500ac42047ac4,
    (bk_key_t)0xc9452ca81a09d85d, (bk_key_t)0x24aa6c514da27500,
    (bk_key_t)0x4c9f34427501b447, (bk_key_t)0x14a68fd73c910841,
    (bk_key_t)0xa71b9b83461cbd93, (bk_key_t)0x03488b95b0f1850f,
    (bk_key_t)0x637b2b34ff93c040, (bk_key_t)0x09d1bc9a3dd90a94,
    (bk_key_t)0x3575668334a1dd3b, (bk_key_t)0x735e2b97a4c45a23,
    (bk_key_t)0x18727070f1bd400b, (bk_key_t)0x1fcbacd259bf02e7,
    (bk_key_t)0xd310a7c2ce9b6555, (bk_key_t)0xbf983fe0fe5d8244,
    (bk_key_t)0x9f74d14f7454a824, (bk_key_t)0x51ebdc4ab9ba3035,
    (bk_key_t)0x5c82c505db9ab0fa, (bk_key_t)0xfcf7fe8a3430b241,
    (bk_key_t)0x3253a729b9ba3dde, (bk_key_t)0x8c74c368081b3075,
    (bk_key_t)0xb9bc6c87167c33e7, (bk_key_t)0x7ef48f2b83024e20,
    (bk_key_t)0x11d505d4c351bd7f, (bk_key_t)0x6568fca92c76a243,
    (bk_key_t)0x4de0b0f40f32a7b8, (bk_key_t)0x96d693460cc37e5d,
    (bk_key_t)0x42e240cb63689f2f, (bk_key_t)0x6d2bdcdae2919661,
    (bk_key_t)0x42880b0236e4d951, (bk_key_t)0x5f0f4a5898171bb6,
    (bk_key_t)0x39f890f579f92f88, (bk_key_t)0x93c5b5f47356388b,
    (bk_key_t)0x63dc359d8d231b78, (bk_key_t)0xec16ca8aea98ad76,
    (bk_key_t)0x5355f900c2a82dc7, (bk_key_t)0x07fb9f855a997142,
    (bk_key_t)0x5093417aa8a7ed5e, (bk_key_t)0x7bcbc38da25a7f3c,
    (bk_key_t)0x19fc8a768cf4b6d4, (bk_key_t)0x637a7780decfc0d9,
    (bk_key_t)0x8249a47aee0e41f7, (bk_key_t)0x79ad695501e7d1e8,
    (bk_key_t)0x14acbaf4777d5776, (bk_key_t)0xf145b6beccdea195,
    (bk_key_t)0xdabf2ac8201752fc, (bk_key_t)0x24c3c94df9c8d3f6,
    (bk_key_t)0xbb6e2924f03912ea, (bk_key_t)0x0ce26c0b95c980d9,
    (bk_key_t)0xa49cd132bfbf7cc4, (bk_key_t)0xe99d662af4243939,
    (bk_key_t)0x27e6ad7891165c3f, (bk_key_t)0x8535f040b9744ff1,
    (bk_key_t)0x54b3f4fa5f40d873, (bk_key_t)0x72b12c32127fed2b,
    (bk_key_t)0xee954d3c7b411f47, (bk_key_t)0x9a85ac909a24eaa1,
    (bk_key_t)0x70ac4cd9f04f21f5, (bk_key_t)0xf9b89d3e99a075c2,
    (bk_key_t)0x87b3e2b2b5c907b1, (bk_key_t)0xa366e5b8c54f48b8,
    (bk_key_t)0xae4a9346cc3f7cf2, (bk_key_t)0x1920c04d47267bbd,
    (bk_key_t)0x87bf02c6b49e2ae9, (bk_key_t)0x092237ac237f3859,
    (bk_key_t)0xff07f64ef8ed14d0, (bk_key_t)0x8de8dca9f03cc54e,
    (bk_key_t)0x9c1633264db49c89, (bk_key_t)0xb3f22c3d0b0b38ed,
    (bk_key_t)0x390e5fb44d01144b, (bk_key_t)0x5bfea5b4712768e9,
    (bk_key_t)0x1e1032911fa78984, (bk_key_t)0x9a74acb964e78cb3,
    (bk_key_t)0x4f80f7a035dafb04, (bk_key_t)0x6304d09a0b3738c4,
    (bk_key_t)0x2171e64683023a08, (bk_key_t)0x5b9b63eb9ceff80c,
    (bk_key_t)0x506aacf489889342, (bk_key_t)0x1881afc9a3a701d6,
    (bk_key_t)0x6503080440750644, (bk_key_t)0xdfd395339cdbf4a7,
    (bk_key_t)0xef927dbcf00c20f2, (bk_key_t)0x7b32f7d1e03680ec,
    (bk_key_t)0xb9fd7620e7316243, (bk_key_t)0x05a7e8a57db91b77,
    (bk_key_t)0xb5889c6e15630a75, (bk_key_t)0x4a750a09ce9573f7,
    (bk_key_t)0xcf464cec899a2f8a, (bk_key_t)0xf538639ce705b824,
    (bk_key_t)0x3c79a0ff5580ef7f, (bk_key_t)0xede6c87f8477609d,
    (bk_key_t)0x799e81f05bc93f31, (bk_key_t)0x86536b8cf3428a8c,
    (bk_key_t)0x97d7374c60087b73, (bk_key_t)0xa246637cff328532,
    (bk_key_t)0x043fcae60cc0eba0, (bk_key_t)0x920e449535dd359e,
    (bk_key_t)0x70eb093b15b290cc, (bk_key_t)0x73a1921916591cbd,
    (bk_key_t)0x56436c9fe1a1aa8d, (bk_key_t)0xefac4b70633b8f81,
    (bk_key_t)0xbb215798d45df7af, (bk_key_t)0x45f20042f24f1768,
    (bk_key_t)0x930f80f4e8eb7462, (bk_key_t)0xff6712ffcfd75ea1,
    (bk_key_t)0xae623fd67468aa70, (bk_key_t)0xdd2c5bc84bc8d8fc,
    (bk_key_t)0x7eed120d54cf2dd9, (bk_key_t)0x22fe545401165f1c,
    (bk_key_t)0xc91800e98fb99929, (bk_key_t)0x808bd68e6ac10365,
    (bk_key_t)0xdec468145b7605f6, (bk_key_t)0x1bede3a3aef53302,
    (bk_key_t)0x43539603d6c55602, (bk_key_t)0xaa969b5c691ccb7a,
    (bk_key_t)0xa87832d392efee56, (bk_key_t)0x65942c7b3c7e11ae,
    (bk_key_t)0xded2d633cad004f6, (bk_key_t)0x21f08570f420e565,
    (bk_key_t)0xb415938d7da94e3c, (bk_key_t)0x91b859e59ecb6350,
    (bk_key_t)0x10cff333e0ed804a, (bk_key_t)0x28aed140be0bb7dd,
    (bk_key_t)0xc5cc1d89724fa456, (bk_key_t)0x5648f680f11a2741,
    (bk_key_t)0x2d255069f0b7dab3, (bk_key_t)0x9bc5a38ef729abd4,
    (bk_key_t)0xef2f054308f6a2bc, (bk_key_t)0xaf2042f5cc5c2858,
    (bk_key_t)0x480412bab7f5be2a, (bk_key_t)0xaef3af4a563dfe43,
    (bk_key_t)0x19afe59ae451497f, (bk_key_t)0x52593803dff1e840,
    (bk_key_t)0xf4f076e65f2ce6f0, (bk_key_t)0x11379625747d5af3,
    (bk_key_t)0xbce5d2248682c115, (bk_key_t)0x9da4243de836994f,
    (bk_key_t)0x066f70b33fe09017, (bk_key_t)0x4dc4de189b671a1c,
    (bk_key_t)0x51039ab7712457c3, (bk_key_t)0xc07a3f80c31fb4b4,
    (bk_key_t)0xb46ee9c5e64a6e7c, (bk_key_t)0xb3819a42abe61c87,
    (bk_key_t)0x21a007933a522a20, (bk_key_t)0x2df16f761598aa4f,
    (bk_key_t)0x763c4a1371b368fd, (bk_key_t)0xf793c46702e086a0,
    (bk_key_t)0xd7288e012aeb8d31, (bk_key_t)0xde336a2a4bc1c44b,
    (bk_key_t)0x0bf692b38d079f23, (bk_key_t)0x2c604a7a177326b3,
    (bk_key_t)0x4850e73e03eb6064, (bk_key_t)0xcfc447f1e53c8e1b,
    (bk_key_t)0xb05ca3f564268d99, (bk_key_t)0x9ae182c8bc9474e8,
    (bk_key_t)0xa4fc4bd4fc5558ca, (bk_key_t)0xe755178d58fc4e76,
    (bk_key_t)0x69b97db1a4c03dfe, (bk_key_t)0xf9b5b7c4acc67c96,
    (bk_key_t)0xfc6a82d64b8655fb, (bk_key_t)0x9c684cb6c4d24417,
    (bk_key_t)0x8ec97d2917456ed0, (bk_key_t)0x6703df9d2924e97e,
    (bk_key_t)0xc547f57e42a7444e, (bk_key_t)0x78e37644e7cad29e,
    (bk_key_t)0xfe9a44e9362f05fa, (bk_key_t)0x08bd35cc38336615,
    (bk_key_t)0x9315e5eb3a129ace, (bk_key_t)0x94061b871e04df75,
    (bk_key_t)0xdf1d9f9d784ba010, (bk_key_t)0x3bba57b68871b59d,
    (bk_key_t)0xd2b7adeeded1f73f, (bk_key_t)0xf7a255d83bc373f8,
    (bk_key_t)0xd7f4f2448c0ceb81, (bk_key_t)0xd95be88cd210ffa7,
    (bk_key_t)0x336f52f8ff4728e7, (bk_key_t)0xa74049dac312ac71,
    (bk_key_t)0xa2f61bb6e437fdb5, (bk_key_t)0x4f2a5cb07f6a35b3,
    (bk_key_t)0x87d380bda5bf7859, (bk_key_t)0x16b9f7e06c453a21,
    (bk_key_t)0x7ba2484c8a0fd54e, (bk_key_t)0xf3a678cad9a2e38c,
    (bk_key_t)0x39b0bf7dde437ba2, (bk_key_t)0xfcaf55c1bf8a4424,
    (bk_key_t)0x18fcf680573fa594, (bk_key_t)0x4c0563b89f495ac3,
    (bk_key_t)0x40e087931a00930d, (bk_key_t)0x8cffa9412eb642c1,
    (bk_key_t)0x68ca39053261169f, (bk_key_t)0x7a1ee967d27579e2,
    (bk_key_t)0x9d1d60e5076f5b6f, (bk_key_t)0x3810e399b6f65ba2,
    (bk_key_t)0x32095b6d4ab5f9b1, (bk_key_t)0x35cab62109dd038a,
    (bk_key_t)0xa90b24499fcfafb1, (bk_key_t)0x77a225a07cc2c6bd,
    (bk_key_t)0x513e5e634c70e331, (bk_key_t)0x4361c0ca3f692f12,
    (bk_key_t)0xd941aca44b20a45b, (bk_key_t)0x528f7c8602c5807b,
    (bk_key_t)0x52ab92beb9613989, (bk_key_t)0x9d1dfa2efc557f73,
    (bk_key_t)0x722ff175f572c348, (bk_key_t)0x1d1260a51107fe97,
    (bk_key_t)0x7a249a57ec0c9ba2, (bk_key_t)0x04208fe9e8f7f2d6,
    (bk_key_t)0x5a110c6058b920a0, (bk_key_t)0x0cd9a497658a5698,
    (bk_key_t)0x56fd23c8f9715a4c, (bk_key_t)0x284c847b9d887aae,
    (bk_key_t)0x04feabfbbdb619cb, (bk_key_t)0x742e1e651c60ba83,
    (bk_key_t)0x9a9632e65904ad3c, (bk_key_t)0x881b82a13b51b9e2,
    (bk_key_t)0x506e6744cd974924, (bk_key_t)0xb0183db56ffc6a79,
    (bk_key_t)0x0ed9b915c66ed37e, (bk_key_t)0x5e11e86d5873d484,
    (bk_key_t)0xf678647e3519ac6e, (bk_key_t)0x1b85d488d0f20cc5,
    (bk_key_t)0xdab9fe6525d89021, (bk_key_t)0x0d151d86adb73615,
    (bk_key_t)0xa865a54edcc0f019, (bk_key_t)0x93c42566aef98ffb,
    (bk_key_t)0x99e7afeabe000731, (bk_key_t)0x48cbff086ddf285a,
    (bk_key_t)0x7f9b6af1ebf78baf, (bk_key_t)0x58627e1a149bba21,
    (bk_key_t)0x2cd16e2abd791e33, (bk_key_t)0xd363eff5f0977996,
    (bk_key_t)0x0ce2a38c344a6eed, (bk_key_t)0x1a804aadb9cfa741,
    (bk_key_t)0x907f30421d78c5de, (bk_key_t)0x501f65edb3034d07,
    (bk_key_t)0x37624ae5a48fa6e9, (bk_key_t)0x957baf61700cff4e,
    (bk_key_t)0x3a6c27934e31188a, (bk_key_t)0xd49503536abca345,
    (bk_key_t)0x088e049589c432e0, (bk_key_t)0xf943aee7febf21b8,
    (bk_key_t)0x6c3b8e3e336139d3, (bk_key_t)0x364f6ffa464ee52e,
    (bk_key_t)0xd60f6dcedc314222, (bk_key_t)0x56963b0dca418fc0,
    (bk_key_t)0x16f50edf91e513af, (bk_key_t)0xef1955914b609f93,
    (bk_key_t)0x565601c0364e3228, (bk_key_t)0xecb53939887e8175,
    (bk_key_t)0xbac7a9a18531294b, (bk_key_t)0xb344c470397bba52,
    (bk_key_t)0x65d34954daf3cebd, (bk_key_t)0xb4b81b3fa97511e2,
    (bk_key_t)0xb422061193d6f6a7, (bk_key_t)0x071582401c38434d,
    (bk_key_t)0x7a13f18bbedc4ff5, (bk_key_t)0xbc4097b116c524d2,
    (bk_key_t)0x59b97885e2f2ea28, (bk_key_t)0x99170a5dc3115544,
    (bk_key_t)0x6f423357e7c6a9f9, (bk_key_t)0x325928ee6e6f8794,
    (bk_key_t)0xd0e4366228b03343, (bk_key_t)0x565c31f7de89ea27,
    (bk_key_t)0x30f5611484119414, (bk_key_t)0xd873db391292ed4f,
    (bk_key_t)0x7bd94e1d8e17debc, (bk_key_t)0xc7d9f16864a76e94,
    (bk_key_t)0x947ae053ee56e63c, (bk_key_t)0xc8c93882f9475f5f,
    (bk_key_t)0x3a9bf55ba91f81ca, (bk_key_t)0xd9a11fbb3d9808e4,
    (bk_key_t)0x0fd22063edc29fca, (bk_key_t)0xb3f256d8aca0b0b9,
    (bk_key_t)0xb03031a8b4516e84, (bk_key_t)0x35dd37d5871448af,
    (bk_key_t)0xe9f6082b05542e4e, (bk_key_t)0xebfafa33d7254b59,
    (bk_key_t)0x9255abb50d532280, (bk_key_t)0xb9ab4ce57f2d34f3,
    (bk_key_t)0x693501d628297551, (bk_key_t)0xc62c58f97dd949bf,
    (bk_key_t)0xcd454f8f19c5126a, (bk_key_t)0xbbe83f4ecc2bdecb,
    (bk_key_t)0xdc842b7e2819e230, (bk_key_t)0xba89142e007503b8,
    (bk_key_t)0xa3bc941d0a5061cb, (bk_key_t)0xe9f6760e32cd8021,
    (bk_key_t)0x09c7e552bc76492f, (bk_key_t)0x852f54934da55cc9,
    (bk_key_t)0x8107fccf064fcf56, (bk_key_t)0x098954d51fff6580,
    (bk_key_t)0x23b70edb1955c4bf, (bk_key_t)0xc330de426430f69d,
    (bk_key_t)0x4715ed43e8a45c0a, (bk_key_t)0xa8d7e4dab780a08d,
    (bk_key_t)0x0572b974f03ce0bb, (bk_key_t)0xb57d2e985e1419c7,
    (bk_key_t)0xe8d9ecbe2cf3d73f, (bk_key_t)0x2fe4b17170e59750,
    (bk_key_t)0x11317ba87905e790, (bk_key_t)0x7fbf21ec8a1f45ec,
    (bk_key_t)0x1725cabfcb045b00, (bk_key_t)0x964e915cd5e2b207,
    (bk_key_t)0x3e2b8bcbf016d66d, (bk_key_t)0xbe7444e39328a0ac,
    (bk_key_t)0xf85b2b4fbcde44b7, (bk_key_t)0x49353fea39ba63b1,
    (bk_key_t)0x1dd01aafcd53486a, (bk_key_t)0x1fca8a92fd719f85,
    (bk_key_t)0xfc7c95d827357afa, (bk_key_t)0x18a6a990c8b35ebd,
    (bk_key_t)0xcccb7005c6b9c28d, (bk_key_t)0x3bdbb92c43b17f26,
    (bk_key_t)0xaa70b5b4f89695a2, (bk_key_t)0xe94c39a54a98307f,
    (bk_key_t)0xb7a0b174cff6f36e, (bk_key_t)0xd4dba84729af48ad,
    (bk_key_t)0x2e18bc1ad9704a68, (bk_key_t)0x2de0966daf2f8b1c,
    (bk_key_t)0xb9c11d5b1e43a07e, (bk_key_t)0x64972d68dee33360,
    (bk_key_t)0x94628d38d0c20584, (bk_key_t)0xdbc0d2b6ab90a559,
    (bk_key_t)0xd2733c4335c6a72f, (bk_key_t)0x7e75d99d94a70f4d,
    (bk_key_t)0x6ced1983376fa72b, (bk_key_t)0x97fcaacbf030bc24,
    (bk_key_t)0x7b77497b32503b12, (bk_key_t)0x8547eddfb81ccb94,
    (bk_key_t)0x79999cdff70902cb, (bk_key_t)0xcffe1939438e9b24,
    (bk_key_t)0x829626e3892d95d7, (bk_key_t)0x92fae24291f2b3f1,
    (bk_key_t)0x63e22c147b9c3403, (bk_key_t)0xc678b6d860284a1c,
    (bk_key_t)0x5873888850659ae7, (bk_key_t)0x0981dcd296a8736d,
    (bk_key_t)0x9f65789a6509a440, (bk_key_t)0x9ff38fed72e9052f,
    (bk_key_t)0xe479ee5b9930578c, (bk_key_t)0xe7f28ecd2d49eecd,
    (bk_key_t)0x56c074a581ea17fe, (bk_key_t)0x5544f7d774b14aef,
    (bk_key_t)0x7b3f0195fc6f290f, (bk_key_t)0x12153635b2c0cf57,
    (bk_key_t)0x7f5126dbba5e0ca7, (bk_key_t)0x7a76956c3eafb413,
    (bk_key_t)0x3d5774a11d31ab39, (bk_key_t)0x8a1b083821f40cb4,
    (bk_key_t)0x7b4a38e32537df62, (bk_key_t)0x950113646d1d6e03,
    (bk_key_t)0x4da8979a0041e8a9, (bk_key_t)0x3bc36e078f7515d7,
    (bk_key_t)0x5d0a12f27ad310d1, (bk_key_t)0x7f9d1a2e1ebe1327,
    (bk_key_t)0xda3a361b1c5157b1, (bk_key_t)0xdcdd7d20903d0c25,
    (bk_key_t)0x36833336d068f707, (bk_key_t)0xce68341f79893389,
    (bk_key_t)0xab9090168dd05f34, (bk_key_t)0x43954b3252dc25e5,
    (bk_key_t)0xb438c2b67f98e5e9, (bk_key_t)0x10dcd78e3851a492,
    (bk_key_t)0xdbc27ab5447822bf, (bk_key_t)0x9b3cdb65f82ca382,
    (bk_key_t)0xb67b7896167b4c84, (bk_key_t)0xbfced1b0048eac50,
    (bk_key_t)0xa9119b60369ffebd, (bk_key_t)0x1fff7ac80904bf45,
    (bk_key_t)0xac12fb171817eee7, (bk_key_t)0xaf08da9177dda93d,
    (bk_key_t)0x1b0cab936e65c744, (bk_key_t)0xb559eb1d04e5e932,
    (bk_key_t)0xc37b45b3f8d6f2ba, (bk_key_t)0xc3a9dc228caac9e9,
    (bk_key_t)0xf3b8b6675a6507ff, (bk_key_t)0x9fc477de4ed681da,
    (bk_key_t)0x67378d8eccef96cb, (bk_key_t)0x6dd856d94d259236,
    (bk_key_t)0xa319ce15b0b4db31, (bk_key_t)0x073973751f12dd5e,
    (bk_key_t)0x8a8e849eb32781a5, (bk_key_t)0xe1925c71285279f5,
    (bk_key_t)0x74c04bf1790c0efe, (bk_key_t)0x4dda48153c94938a,
    (bk_key_t)0x9d266d6a1cc0542c, (bk_key_t)0x7440fb816508c4fe,
    (bk_key_t)0x13328503df48229f, (bk_key_t)0xd6bf7baee43cac40,
    (bk_key_t)0x4838d65f6ef6748f, (bk_key_t)0x1e152328f3318dea,
    (bk_key_t)0x8f8419a348f296bf, (bk_key_t)0x72c8834a5957b511,
    (bk_key_t)0xd7a023a73260b45c, (bk_key_t)0x94ebc8abcfb56dae,
    (bk_key_t)0x9fc10d0f989993e0, (bk_key_t)0xde68a2355b93cae6,
    (bk_key_t)0xa44cfe79ae538bbe, (bk_key_t)0x9d1d84fcce371425,
    (bk_key_t)0x51d2b1ab2ddfb636, (bk_key_t)0x2fd7e4b9e72cd38c,
    (bk_key_t)0x65ca5b96b7552210, (bk_key_t)0xdd69a0d8ab3b546d,
    (bk_key_t)0x604d51b25fbf70e2, (bk_key_t)0x73aa8a564fb7ac9e,
    (bk_key_t)0x1a8c1e992b941148, (bk_key_t)0xaac40a2703d9bea0,
    (bk_key_t)0x764dbeae7fa4f3a6, (bk_key_t)0x1e99b96e70a9be8b,
    (bk_key_t)0x2c5e9deb57ef4743, (bk_key_t)0x3a938fee32d29981,
    (bk_key_t)0x26e6db8ffdf5adfe, (bk_key_t)0x469356c504ec9f9d,
    (bk_key_t)0xc8763c5b08d1908c, (bk_key_t)0x3f6c6af859d80055,
    (bk_key_t)0x7f7cc39420a3a545, (bk_key_t)0x9bfb227ebdf4c5ce,
    (bk_key_t)0x89039d79d6fc5c5c, (bk_key_t)0x8fe88b57305e2ab6,
    (bk_key_t)0xa09e8c8c35ab96de, (bk_key_t)0xfa7e393983325753,
    (bk_key_t)0xd6b6d0ecc617c699, (bk_key_t)0xdfea21ea9e7557e3,
    (bk_key_t)0xb67c1fa481680af8, (bk_key_t)0xca1e3785a9e724e5,
    (bk_key_t)0x1cfc8bed0d681639, (bk_key_t)0xd18d8549d140caea,
    (bk_key_t)0x4ed0fe7e9dc91335, (bk_key_t)0xe4dbf0634473f5d2,
    (bk_key_t)0x1761f93a44d5aefe, (bk_key_t)0x53898e4c3910da55,
    (bk_key_t)0x734de8181f6ec39a, (bk_key_t)0x2680b122baa28d97,
    (bk_key_t)0x298af231c85bafab, (bk_key_t)0x7983eed3740847d5,
    (bk_key_t)0x66c1a2a1a60cd889, (bk_key_t)0x9e17e49642a3e4c1,
    (bk_key_t)0xedb454e7badc0805, (bk_key_t)0x50b704cab602c329,
    (bk_key_t)0x4cc317fb9cddd023, (bk_key_t)0x66b4835d9eafea22,
    (bk_key_t)0x219b97e26ffc81bd, (bk_key_t)0x261e4e4c0a333a9d,
    (bk_key_t)0x1fe2cca76517db90, (bk_key_t)0xd7504dfa8816edbb,
    (bk_key_t)0xb9571fa04dc089c8, (bk_key_t)0x1ddc0325259b27de,
    (bk_key_t)0xcf3f4688801eb9aa, (bk_key_t)0xf4f5d05c10cab243,
    (bk_key_t)0x38b6525c21a42b0e, (bk_key_t)0x36f60e2ba4fa6800,
    (bk_key_t)0xeb3593803173e0ce, (bk_key_t)0x9c4cd6257c5a3603,
    (bk_key_t)0xaf0c317d32adaa8a, (bk_key_t)0x258e5a80c7204c4b,
    (bk_key_t)0x8b889d624d44885d, (bk_key_t)0xf4d14597e660f855,
    (bk_key_t)0xd4347f66ec8941c3, (bk_key_t)0xe699ed85b0dfb40d,
    (bk_key_t)0x2472f6207c2d0484, (bk_key_t)0xc2a1e7b5b459aeb5,
    (bk_key_t)0xab4f6451cc1d45ec, (bk_key_t)0x63767572ae3d6174,
    (bk_key_t)0xa59e0bd101731a28, (bk_key_t)0x116d0016cb948f09,
    (bk_key_t)0x2cf9c8ca052f6e9f, (bk_key_t)0x0b090a7560a968e3,
    (bk_key_t)0xabeeddb2dde06ff1, (bk_key_t)0x58efc10b06a2068d,
    (bk_key_t)0xc6e57a78fbd986e0, (bk_key_t)0x2eab8ca63ce802d7,
    (bk_key_t)0x14a195640116f336, (bk_key_t)0x7c0828dd624ec390,
    (bk_key_t)0xd74bbe77e6116ac7, (bk_key_t)0x804456af10f5fb53,
    (bk_key_t)0xebe9ea2adf4321c7, (bk_key_t)0x03219a39ee587a30,
    (bk_key_t)0x49787fef17af9924, (bk_key_t)0xa1e9300cd8520548,
    (bk_key_t)0x5b45e522e4b1b4ef, (bk_key_t)0xb49c3b3995091a36,
    (bk_key_t)0xd4490ad526f14431, (bk_key_t)0x12a8f216af9418c2,
    (bk_key_t)0x001f837cc7350524, (bk_key_t)0x1877b51e57a764d5,
    (bk_key_t)0xa2853b80f17f58ee, (bk_key_t)0x993e1de72d36d310,
    (bk_key_t)0xb3598080ce64a656, (bk_key_t)0x252f59cf0d9f04bb,
    (bk_key_t)0xd23c8e176d113600, (bk_key_t)0x1bda0492e7e4586e,
    (bk_key_t)0x21e0bd5026c619bf, (bk_key_t)0x3b097adaf088f94e,
    (bk_key_t)0x8d14dedb30be846e, (bk_key_t)0xf95cffa23af5f6f4,
    (bk_key_t)0x3871700761b3f743, (bk_key_t)0xca672b91e9e4fa16,
    (bk_key_t)0x64c8e531bff53b55, (bk_key_t)0x241260ed4ad1e87d,
    (bk_key_t)0x106c09b972d2e822, (bk_key_t)0x7fba195410e5ca30,
    (bk_key_t)0x7884d9bc6cb569d8, (bk_key_t)0x0647dfedcd894a29,
    (bk_key_t)0x63573ff03e224774, (bk_key_t)0x4fc8e9560f91b123,
    (bk_key_t)0x1db956e450275779, (bk_key_t)0xb8d91274b9e9d4fb,
    (bk_key_t)0xa2ebee47e2fbfce1, (bk_key_t)0xd9f1f30ccd97fb09,
    (bk_key_t)0xefed53d75fd64e6b, (bk_key_t)0x2e6d02c36017f67f,
    (bk_key_t)0xa9aa4d20db084e9b, (bk_key_t)0xb64be8d8b25396c1,
    (bk_key_t)0x70cb6af7c2d5bcf0, (bk_key_t)0x98f076a4f7a2322e,
    (bk_key_t)0xbf84470805e69b5f, (bk_key_t)0x94c3251f06f90cf3,
    (bk_key_t)0x3e003e616a6591e9, (bk_key_t)0xb925a6cd0421aff3,
    (bk_key_t)0x61bdd1307c66e300, (bk_key_t)0xbf8d5108e27e0d48,
    (bk_key_t)0x240ab57a8b888b20, (bk_key_t)0xfc87614baf287e07,
    (bk_key_t)0xef02cdd06ffdb432, (bk_key_t)0xa1082c0466df6c0a,
    (bk_key_t)0x8215e577001332c8, (bk_key_t)0xd39bb9c3a48db6cf,
    (bk_key_t)0x2738259634305c14, (bk_key_t)0x61cf4f94c97df93d,
    (bk_key_t)0x1b6baca2ae4e125b, (bk_key_t)0x758f450c88572e0b,
    (bk_key_t)0x959f587d507a8359, (bk_key_t)0xb063e962e045f54d,
    (bk_key_t)0x60e8ed72c0dff5d1, (bk_key_t)0x7b64978555326f9f,
    (bk_key_t)0xfd080d236da814ba, (bk_key_t)0x8c90fd9b083f4558,
    (bk_key_t)0x106f72fe81e2c590, (bk_key_t)0x7976033a39f7d952,
    (bk_key_t)0xa4ec0132764ca04b, (bk_key_t)0x733ea705fae4fa77,
    (bk_key_t)0xb4d8f77bc3e56167, (bk_key_t)0x9e21f4f903b33fd9,
    (bk_key_t)0x9d765e419fb69f6d, (bk_key_t)0xd30c088ba61ea5ef,
    (bk_key_t)0x5d94337fbfaf7f5b, (bk_key_t)0x1a4e4822eb4d7a59,
    (bk_key_t)0x6ffe73e81b637fb3, (bk_key_t)0xddf957bc36d8b9ca,
    (bk_key_t)0x64d0e29eea8838b3, (bk_key_t)0x08dd9bdfd96b9f63,
    (bk_key_t)0x087e79e5a57d1d13, (bk_key_t)0xe328e230e3e2b3fb,
    (bk_key_t)0x1c2559e30f0946be, (bk_key_t)0x720bf5f26f4d2eaa,
    (bk_key_t)0xb0774d261cc609db, (bk_key_t)0x443f64ec5a371195,
    (bk_key_t)0x4112cf68649a260e, (bk_key_t)0xd813f2fab7f5c5ca,
    (bk_key_t)0x660d3257380841ee, (bk_key_t)0x59ac2c7873f910a3,
    (bk_key_t)0xe846963877671a17, (bk_key_t)0x93b633abfa3469f8,
    (bk_key_t)0xc0c0f5a60ef4cdcf, (bk_key_t)0xcaf21ecd4377b28c,
    (bk_key_t)0x57277707199b8175, (bk_key_t)0x506c11b9d90e8b1d,
    (bk_key_t)0xd83cc2687a19255f, (bk_key_t)0x4a29c6465a314cd1,
    (bk_key_t)0xed2df21216235097, (bk_key_t)0xb5635c95ff7296e2,
    (bk_key_t)0x22af003ab672e811, (bk_key_t)0x52e762596bf68235,
    (bk_key_t)0x9aeba33ac6ecc6b0, (bk_key_t)0x944f6de09134dfb6,
    (bk_key_t)0x6c47bec883a7de39, (bk_key_t)0x6ad047c430a12104,
    (bk_key_t)0xa5b1cfdba0ab4067, (bk_key_t)0x7c45d833aff07862,
    (bk_key_t)0x5092ef950a16da0b, (bk_key_t)0x9338e69c052b8e7b,
    (bk_key_t)0x455a4b4cfe30e3f5, (bk_key_t)0x6b02e63195ad0cf8,
    (bk_key_t)0x6b17b224bad6bf27, (bk_key_t)0xd1e0ccd25bb9c169,
    (bk_key_t)0xde0c89a556b9ae70, (bk_key_t)0x50065e535a213cf6,
    (bk_key_t)0x9c1169fa2777b874, (bk_key_t)0x78edefd694af1eed,
    (bk_key_t)0x6dc93d9526a50e68, (bk_key_t)0xee97f453f06791ed,
    (bk_key_t)0x32ab0edb696703d3, (bk_key_t)0x3a6853c7e70757a7,
    (bk_key_t)0x31865ced6120f37d, (bk_key_t)0x67fef95d92607890,
    (bk_key_t)0x1f2b1d1f15f6dc9c, (bk_key_t)0xb69e38a8965c6b65,
    (bk_key_t)0xaa9119ff184cccf4, (bk_key_t)0xf43c732873f24c13,
    (bk_key_t)0xfb4a3d794a9a80d2, (bk_key_t)0x3550c2321fd6109c,
    (bk_key_t)0x371f77e76bb8417e, (bk_key_t)0x6bfa9aae5ec05779,
    (bk_key_t)0xcd04f3ff001a4778, (bk_key_t)0xe3273522064480ca,
    (bk_key_t)0x9f91508bffcfc14a, (bk_key_t)0x049a7f41061a9e60,
    (bk_key_t)0xfcb6be43a9f2fe9b, (bk_key_t)0x08de8a1c7797da9b,
    (bk_key_t)0x8f9887e6078735a1, (bk_key_t)0xb5b4071dbfc73a66,
    (bk_key_t)0x230e343dfba08d33, (bk_key_t)0x43ed7f5a0fae657d,
    (bk_key_t)0x3a88a0fbbcb05c63, (bk_key_t)0x21874b8b4d2dbc4f,
    (bk_key_t)0x1bdea12e35f6a8c9, (bk_key_t)0x53c065c6c8e63528,
    (bk_key_t)0xe34a1d250e7a8d6b, (bk_key_t)0xd6b04d3b7651dd7e,
    (bk_key_t)0x5e90277e7cb39e2d, (bk_key_t)0x2c046f22062dc67d,
    (bk_key_t)0xb10bb459132d0a26, (bk_key_t)0x3fa9ddfb67e2f199,
    (bk_key_t)0x0e09b88e1914f7af, (bk_key_t)0x10e8b35af3eeab37,
    (bk_key_t)0x9eedeca8e272b933, (bk_key_t)0xd4c718bc4ae8ae5f,
    (bk_key_t)0x81536d601170fc20, (bk_key_t)0x91b534f885818a06,
    (bk_key_t)0xec8177f83f900978, (bk_key_t)0x190e714fada5156e,
    (bk_key_t)0xb592bf39b0364963, (bk_key_t)0x89c350c893ae7dc1,
    (bk_key_t)0xac042e70f8b383f2, (bk_key_t)0xb49b52e587a1ee60,
    (bk_key_t)0xfb152fe3ff26da89, (bk_key_t)0x3e666e6f69ae2c15,
    (bk_key_t)0x3b544ebe544c19f9, (bk_key_t)0xe805a1e290cf2456,
    (bk_key_t)0x24b33c9d7ed25117, (bk_key_t)0xe74733427b72f0c1,
    (bk_key_t)0x0a804d18b7097475, (bk_key_t)0x57e3306d881edb4f,
    (bk_key_t)0x4ae7d6a36eb5dbcb, (bk_key_t)0x2d8d5432157064c8,
    (bk_key_t)0xd1e649de1e7f268b, (bk_key_t)0x8a328a1cedfe552c,
    (bk_key_t)0x07a3aec79624c7da, (bk_key_t)0x84547ddc3e203c94,
    (bk_key_t)0x990a98fd5071d263, (bk_key_t)0x1a4ff12616eefc89,
    (bk_key_t)0xf6f7fd1431714200, (bk_key_t)0x30c05b1ba332f41c,
    (bk_key_t)0x8d2636b81555a786, (bk_key_t)0x46c9feb55d120902,
    (bk_key_t)0xccec0a73b49c9921, (bk_key_t)0x4e9d2827355fc492,
    (bk_key_t)0x19ebb029435dcb0f, (bk_key_t)0x4659d2b743848a2c,
    (bk_key_t)0x963ef2c96b33be31, (bk_key_t)0x74f85198b05a2e7d,
    (bk_key_t)0x5a0f544dd2b1fb18, (bk_key_t)0x03727073c2e134b1,
    (bk_key_t)0xc7f6aa2de59aea61, (bk_key_t)0x352787baa0d7c22f,
    (bk_key_t)0x9853eab63b5e0b35, (bk_key_t)0xabbdcdd7ed5c0860,
    (bk_key_t)0xcf05daf5ac8d77b0, (bk_key_t)0x49cad48cebf4a71e,
    (bk_key_t)0x7a4c10ec2158c4a6, (bk_key_t)0xd9e92aa246bf719e,
    (bk_key_t)0x13ae978d09fe5557, (bk_key_t)0x730499af921549ff,
    (bk_key_t)0x4e4b705b92903ba4, (bk_key_t)0xff577222c14f0a3a,
    (bk_key_t)0x55b6344cf97aafae, (bk_key_t)0xb862225b055b6960,
    (bk_key_t)0xcac09afbddd2cdb4, (bk_key_t)0xdaf8e9829fe96b5f,
    (bk_key_t)0xb5fdfc5d3132c498, (bk_key_t)0x310cb380db6f7503,
    (bk_key_t)0xe87fbb46217a360e, (bk_key_t)0x2102ae466ebb1148,
    (bk_key_t)0xf8549e1a3aa5e00d, (bk_key_t)0x07a69afdcc42261a,
    (bk_key_t)0xc4c118bfe78feaae, (bk_key_t)0xf9f4892ed96bd438,
    (bk_key_t)0x1af3dbe25d8f45da, (bk_key_t)0xf5b4b0b0d2deeeb4,
    (bk_key_t)0x962aceefa82e1c84, (bk_key_t)0x046e3ecaaf453ce9,
    (bk_key_t)0xf05d129681949a4c, (bk_key_t)0x964781ce734b3c84,
    (bk_key_t)0x9c2ed44081ce5fbd, (bk_key_t)0x522e23f3925e319e,
    (bk_key_t)0x177e00f9fc32f791, (bk_key_t)0x2bc60a63a6f3b3f2,
    (bk_key_t)0x222bbfae61725606, (bk_key_t)0x486289ddcc3d6780,
    (bk_key_t)0x7dc7785b8efdfc80, (bk_key_t)0x8af38731c02ba980,
    (bk_key_t)0x1fab64ea29a2ddf7, (bk_key_t)0xe4d9429322cd065a,
    (bk_key_t)0x9da058c67844f20c, (bk_key_t)0x24c0e332b70019b0,
    (bk_key_t)0x233003b5a6cfe6ad, (bk_key_t)0xd586bd01c5c217f6,
    (bk_key_t)0x5e5637885f29bc2b, (bk_key_t)0x7eba726d8c94094b,
    (bk_key_t)0x0a56a5f0bfe39272, (bk_key_t)0xd79476a84ee20d06,
    (bk_key_t)0x9e4c1269baa4bf37, (bk_key_t)0x17efee45b0dee640,
    (bk_key_t)0x1d95b0a5fcf90bc6, (bk_key_t)0x93cbe0b699c2585d,
    (bk_key_t)0x65fa4f227a2b6d79, (bk_key_t)0xd5f9e858292504d5,
    (bk_key_t)0xc2b5a03f71471a6f, (bk_key_t)0x59300222b4561e00,
    (bk_key_t)0xce2f8642ca0712dc, (bk_key_t)0x7ca9723fbb2e8988,
    (bk_key_t)0x2785338347f2ba08, (bk_key_t)0xc61bb3a141e50e8c,
    (bk_key_t)0x150f361dab9dec26, (bk_key_t)0x9f6a419d382595f4,
    (bk_key_t)0x64a53dc924fe7ac9, (bk_key_t)0x142de49fff7a7c3d,
    (bk_key_t)0x0c335248857fa9e7, (bk_key_t)0x0a9c32d5eae45305,
    (bk_key_t)0xe6c42178c4bbb92e, (bk_key_t)0x71f1ce2490d20b07,
    (bk_key_t)0xf1bcc3d275afe51a, (bk_key_t)0xe728e8c83c334074,
    (bk_key_t)0x96fbf83a12884624, (bk_key_t)0x81a1549fd6573da5,
    (bk_key_t)0x5fa7867caf35e149, (bk_key_t)0x56986e2ef3ed091b,
    (bk_key_t)0x917f1dd5f8886c61, (bk_key_t)0xd20d8c88c8ffe65f,
};

static const bk_key_t bk_rand_castle[] = {
    (bk_key_t)0x31d71dce64b2c310, (bk_key_t)0xf165b587df898190,
    (bk_key_t)0xa57e6339dd2cf3a0, (bk_key_t)0x1ef6e6dbb1961ec9,
};

static const bk_key_t bk_rand_en_passant[] = {
    (bk_key_t)0x70cc73d90bc26e24, (bk_key_t)0xe21a6b35df0c3ad7,
    (bk_key_t)0x003a93d8b2806962, (bk_key_t)0x1c99ded33cb890a1,
    (bk_key_t)0xcf3145de0add4289, (bk_key_t)0xd0e4427a5514fb72,
    (bk_key_t)0x77c621cc9fb3a483, (bk_key_t)0x67a34dac4356550b,
};

static const bk_key_t bk_rand_turn = (bk_key_t)0xf8d626aaaf278509;

static const char *bk_piece_chars = "pPnNbBrRqQkK";

static struct book book;

static bk_key_t bk_get_key(struct board *board) {
    assert(board != NULL);

    bk_key_t key = BK_KEY_EMPTY;

    for (enum color c = 0; c < COLOR_CNT; ++c) {
        for (enum piece p = 0; p < PIECE_CNT; ++p) {
            char piece_char = piece_to_char(c, p);
            size_t piece_id = strchr(bk_piece_chars, piece_char)-bk_piece_chars;

            bb_t bb_pieces = board->bb_pieces[c][p];

            while (bb_pieces) {
                enum square s = bb_pop_lsb(&bb_pieces);

                key ^= bk_rand_piece[SQ_CNT*piece_id+s];
            }
        }
    }

    if (board->castle_rights & CASTLE_RIGHT_WHITE_KING) {
        key ^= bk_rand_castle[0];
    }

    if (board->castle_rights & CASTLE_RIGHT_WHITE_QUEEN) {
        key ^= bk_rand_castle[1];
    }

    if (board->castle_rights & CASTLE_RIGHT_BLACK_KING) {
        key ^= bk_rand_castle[2];
    }

    if (board->castle_rights & CASTLE_RIGHT_BLACK_QUEEN) {
        key ^= bk_rand_castle[3];
    }

    if (board->en_passant != SQ_NONE) {
        bb_t bb_pawns = BB_EMPTY;

        if (board->color == WHITE) {
            bb_pawns = ((bb_squares[board->en_passant] >> (FL_CNT-1)) & ~FL_H)
                     | ((bb_squares[board->en_passant] >> (FL_CNT+1)) & ~FL_A);
        } else if (board->color == BLACK) {
            bb_pawns = ((bb_squares[board->en_passant] << (FL_CNT-1)) & ~FL_A)
                     | ((bb_squares[board->en_passant] << (FL_CNT+1)) & ~FL_H);
        }

        if (bb_pawns & board->bb_pieces[board->color][BB_PAWNS]) {
            key ^= bk_rand_en_passant[square_to_file(board->en_passant)];
        }
    }

    if (board->color == WHITE) {
        key ^= bk_rand_turn;
    }

    return key;
}

static int bk_comp_entry(const void *fst, const void *snd) {
    const struct bk_entry *fst_e = (const struct bk_entry *)fst;
    const struct bk_entry *snd_e = (const struct bk_entry *)snd;

    if (fst_e->key == snd_e->key) {
        return 0;
    }

    return fst_e->key < snd_e->key ? -1 : 1;
}

static struct bk_entry * bk_get_entry(bk_key_t key) {
    struct bk_entry t = { .key = key };

    return (struct bk_entry *)bsearch(&t, book.base,
                                      book.count, sizeof(*book.base),
                                      bk_comp_entry);
}

void bk_init(void) {
    FILE *f = fopen(BOOK_FILE_NAME, "rb");

    if (f == NULL) {
        error(EXIT_FAILURE, errno, "could not open book file '%s'", BOOK_FILE_NAME);
    }

    fseek(f, 0, SEEK_END);
    book.count = ftell(f)/sizeof(*book.base);

    book.base = malloc(book.count*sizeof(*book.base));

    if (book.base == NULL) {
        error(EXIT_FAILURE, errno, "could not allocate space for book");
    }

    rewind(f);

    struct bk_entry buf[128];

    for (size_t i = 0; !feof(f); ) {
        size_t c = fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), f);

        for (size_t j = 0; j < c; ++j) {
            buf[j].key = be64toh(buf[j].key);
            buf[j].move = be16toh(buf[j].move);
        }

        memcpy(&book.base[i], buf, c*sizeof(*buf));

        i += c;
    }

    fclose(f);
}

void bk_term(void) {
    free(book.base);
}

struct move bk_search(struct board *board) {
    enum color color = board->color;
    enum color color_other = color_flip(color);

    struct move move = { .flags = MOVE_FLAG_INVALID };

    bk_key_t key = bk_get_key(board);
    struct bk_entry *e = bk_get_entry(key);

    if (e != NULL) {
        move.flags = MOVE_FLAG_NONE;

        move.from = rank_file_to_square(e->from_rank, e->from_file);
        move.to = rank_file_to_square(e->to_rank, e->to_file);

        for (enum piece p = 0; p < PIECE_CNT; ++p) {
            if (bb_squares[move.from] & board->bb_pieces[color][p]) {
                move.piece = p;
                break;
            }
        }

        for (enum piece p = 0; p < PIECE_CNT; ++p) {
            if (board->bb_pieces[color_other][p] & bb_squares[move.to]) {
                move.flags |= MOVE_FLAG_CAPTURE;
                move.capture = p;
                break;
            }
        }

        if (move.piece == KING &&
            move.from == (color == WHITE ? SQ_E1 : SQ_E8))
        {
            if (move.to == (color == WHITE ? SQ_H1 : SQ_H8)) {
                move.flags |= MOVE_FLAG_KING_CASTLE;
                move.to = color == WHITE ? SQ_G1 : SQ_G8;
            } else if (move.to == (color == WHITE ? SQ_A1 : SQ_A8)) {
                move.flags |= MOVE_FLAG_QUEEN_CASTLE;
                move.to = color == WHITE ? SQ_C1 : SQ_C8;
            }
        } else if (move.piece == PAWN) {
            if (bb_squares[move.from] & bb_ranks[color == WHITE ? RK_2 : RK_7] &&
                bb_squares[move.to] & bb_ranks[color == WHITE ? RK_4 : RK_5])
            {
                move.flags |= MOVE_FLAG_PAWN_DOUBLE_PUSH;
            } else if (!(move.flags & MOVE_FLAG_CAPTURE) && move.to == board->en_passant) {
                move.flags |= MOVE_FLAG_EN_PASSANT;
            }
        }

        if (e->promotion != 0) {
            move.promotion = (enum piece[]){PIECE_NONE, KNIGHT, BISHOP, ROOK, QUEEN}[e->promotion];
            move.flags |= MOVE_FLAG_PROMOTION;
        }
    }

    return move;
}
