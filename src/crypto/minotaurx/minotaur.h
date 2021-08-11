// Copyright (c) 2019-2021 The Litecoin Cash Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LCC_CRYPTO_MINOTAURX_MINOTAUR_H
#define LCC_CRYPTO_MINOTAURX_MINOTAUR_H

#include <uint256.h>

#include "sph_blake.h"
#include "sph_bmw.h"
#include "sph_cubehash.h"
#include "sph_echo.h"
#include "sph_fugue.h"
#include "sph_groestl.h"
#include "sph_hamsi.h"
#include "sph_jh.h"
#include "sph_keccak.h"
#include "sph_luffa.h"
#include "sph_shabal.h"
#include "sph_shavite.h"
#include "sph_simd.h"
#include "sph_skein.h"
#include "sph_whirlpool.h"
#include "sph_sha2.h"
#include "yespower/yespower.h"

// Config
#define MINOTAUR_ALGO_COUNT 16
//#define MINOTAUR_DEBUG

static const yespower_params_t yespower_params = {YESPOWER_1_0, 2048, 8, (const uint8_t*)"et in arcadia ego", 17};

// Graph of hash algos plus SPH contexts
struct TortureNode {
    unsigned int algo;
    TortureNode *childLeft;
    TortureNode *childRight;
};
struct TortureGarden {
    sph_blake512_context context_blake;
    sph_bmw512_context context_bmw;
    sph_cubehash512_context context_cubehash;
    sph_echo512_context context_echo;
    sph_fugue512_context context_fugue;
    sph_groestl512_context context_groestl;
    sph_hamsi512_context context_hamsi;
    sph_jh512_context context_jh;
    sph_keccak512_context context_keccak;
    sph_luffa512_context context_luffa;
    sph_shabal512_context context_shabal;
    sph_shavite512_context context_shavite;
    sph_simd512_context context_simd;
    sph_skein512_context context_skein;
    sph_whirlpool_context context_whirlpool;
    sph_sha512_context context_sha2;

    TortureNode nodes[22];
};

// Get a 64-byte hash for given 64-byte input, using given TortureGarden contexts and given algo index
uint512 GetHash(uint512 inputHash, TortureGarden *garden, unsigned int algo, yespower_local_t *local) {
    uint512 outputHash;
    switch (algo) {
        case 0:
            sph_blake512_init(&garden->context_blake);
            sph_blake512(&garden->context_blake, static_cast<const void*>(&inputHash), 64);
            sph_blake512_close(&garden->context_blake, static_cast<void*>(&outputHash));
            break;
        case 1:
            sph_bmw512_init(&garden->context_bmw);
            sph_bmw512(&garden->context_bmw, static_cast<const void*>(&inputHash), 64);
            sph_bmw512_close(&garden->context_bmw, static_cast<void*>(&outputHash));        
            break;
        case 2:
            sph_cubehash512_init(&garden->context_cubehash);
            sph_cubehash512(&garden->context_cubehash, static_cast<const void*>(&inputHash), 64);
            sph_cubehash512_close(&garden->context_cubehash, static_cast<void*>(&outputHash));
            break;
        case 3:
            sph_echo512_init(&garden->context_echo);
            sph_echo512(&garden->context_echo, static_cast<const void*>(&inputHash), 64);
            sph_echo512_close(&garden->context_echo, static_cast<void*>(&outputHash));
            break;
        case 4:
            sph_fugue512_init(&garden->context_fugue);
            sph_fugue512(&garden->context_fugue, static_cast<const void*>(&inputHash), 64);
            sph_fugue512_close(&garden->context_fugue, static_cast<void*>(&outputHash));
            break;
        case 5:
            sph_groestl512_init(&garden->context_groestl);
            sph_groestl512(&garden->context_groestl, static_cast<const void*>(&inputHash), 64);
            sph_groestl512_close(&garden->context_groestl, static_cast<void*>(&outputHash));
            break;
        case 6:
            sph_hamsi512_init(&garden->context_hamsi);
            sph_hamsi512(&garden->context_hamsi, static_cast<const void*>(&inputHash), 64);
            sph_hamsi512_close(&garden->context_hamsi, static_cast<void*>(&outputHash));
            break;
        case 7:
            sph_sha512_init(&garden->context_sha2);
            sph_sha512(&garden->context_sha2, static_cast<const void*>(&inputHash), 64);
            sph_sha512_close(&garden->context_sha2, static_cast<void*>(&outputHash));
            break;
        case 8:
            sph_jh512_init(&garden->context_jh);
            sph_jh512(&garden->context_jh, static_cast<const void*>(&inputHash), 64);
            sph_jh512_close(&garden->context_jh, static_cast<void*>(&outputHash));
            break;
        case 9:
            sph_keccak512_init(&garden->context_keccak);
            sph_keccak512(&garden->context_keccak, static_cast<const void*>(&inputHash), 64);
            sph_keccak512_close(&garden->context_keccak, static_cast<void*>(&outputHash));
            break;
        case 10:
            sph_luffa512_init(&garden->context_luffa);
            sph_luffa512(&garden->context_luffa, static_cast<const void*>(&inputHash), 64);
            sph_luffa512_close(&garden->context_luffa, static_cast<void*>(&outputHash));
            break;
        case 11:
            sph_shabal512_init(&garden->context_shabal);
            sph_shabal512(&garden->context_shabal, static_cast<const void*>(&inputHash), 64);
            sph_shabal512_close(&garden->context_shabal, static_cast<void*>(&outputHash));
            break;
        case 12:
            sph_shavite512_init(&garden->context_shavite);
            sph_shavite512(&garden->context_shavite, static_cast<const void*>(&inputHash), 64);
            sph_shavite512_close(&garden->context_shavite, static_cast<void*>(&outputHash));
            break;
        case 13:
            sph_simd512_init(&garden->context_simd);
            sph_simd512(&garden->context_simd, static_cast<const void*>(&inputHash), 64);
            sph_simd512_close(&garden->context_simd, static_cast<void*>(&outputHash));
            break;
        case 14:
            sph_skein512_init(&garden->context_skein);
            sph_skein512(&garden->context_skein, static_cast<const void*>(&inputHash), 64);
            sph_skein512_close(&garden->context_skein, static_cast<void*>(&outputHash));
            break;
        case 15:
            sph_whirlpool_init(&garden->context_whirlpool);
            sph_whirlpool(&garden->context_whirlpool, static_cast<const void*>(&inputHash), 64);
            sph_whirlpool_close(&garden->context_whirlpool, static_cast<void*>(&outputHash));
            break;
        // NB: The CPU-hard gate must be case MINOTAUR_ALGO_COUNT.
        case 16:
            if (local == NULL)  // Self-manage storage on current thread
                yespower_tls(inputHash.begin(), 64, &yespower_params, (yespower_binary_t*)outputHash.begin());
            else                // Use provided thread-local storage
                yespower(local, inputHash.begin(), 64, &yespower_params, (yespower_binary_t*)outputHash.begin());

            break;
        default:
            assert(false);
            break;
    }

    return outputHash;
}

// Recursively traverse a given torture garden starting with a given hash and given node within the garden. The hash is overwritten with the final hash.
uint512 TraverseGarden(TortureGarden *garden, uint512 hash, TortureNode *node, yespower_local_t *local) {
    uint512 partialHash = GetHash(hash, garden, node->algo, local);

#ifdef MINOTAUR_DEBUG
    printf("* Ran algo %d. Partial hash:\t%s\n", node->algo, partialHash.ToString().c_str());
    fflush(0);
#endif

    if (partialHash.ByteAt(63) % 2 == 0) {      // Last byte of output hash is even
        if (node->childLeft != NULL)
            return TraverseGarden(garden, partialHash, node->childLeft, local);
    } else {                                    // Last byte of output hash is odd
        if (node->childRight != NULL)
            return TraverseGarden(garden, partialHash, node->childRight, local);
    }

    return partialHash;
}

// Associate child nodes with a parent node
void LinkNodes(TortureNode *parent, TortureNode *childLeft, TortureNode *childRight) {
    parent->childLeft = childLeft;
    parent->childRight = childRight;
}

// Produce a Minotaur 32-byte hash from variable length data
// Optionally, use the MinotaurX hardened hash.
// Optionally, use provided thread-local memory for yespower.
template<typename T> uint256 Minotaur(const T begin, const T end, bool minotaurX, yespower_local_t *local = NULL) {
    // Create torture garden nodes. Note that both sides of 19 and 20 lead to 21, and 21 has no children (to make traversal complete).
    // Every path through the garden stops at 7 nodes.
    TortureGarden garden;
    LinkNodes(&garden.nodes[0], &garden.nodes[1], &garden.nodes[2]);
    LinkNodes(&garden.nodes[1], &garden.nodes[3], &garden.nodes[4]);
    LinkNodes(&garden.nodes[2], &garden.nodes[5], &garden.nodes[6]);
    LinkNodes(&garden.nodes[3], &garden.nodes[7], &garden.nodes[8]);
    LinkNodes(&garden.nodes[4], &garden.nodes[9], &garden.nodes[10]);
    LinkNodes(&garden.nodes[5], &garden.nodes[11], &garden.nodes[12]);
    LinkNodes(&garden.nodes[6], &garden.nodes[13], &garden.nodes[14]);
    LinkNodes(&garden.nodes[7], &garden.nodes[15], &garden.nodes[16]);
    LinkNodes(&garden.nodes[8], &garden.nodes[15], &garden.nodes[16]);
    LinkNodes(&garden.nodes[9], &garden.nodes[15], &garden.nodes[16]);
    LinkNodes(&garden.nodes[10], &garden.nodes[15], &garden.nodes[16]);
    LinkNodes(&garden.nodes[11], &garden.nodes[17], &garden.nodes[18]);
    LinkNodes(&garden.nodes[12], &garden.nodes[17], &garden.nodes[18]);
    LinkNodes(&garden.nodes[13], &garden.nodes[17], &garden.nodes[18]);
    LinkNodes(&garden.nodes[14], &garden.nodes[17], &garden.nodes[18]);
    LinkNodes(&garden.nodes[15], &garden.nodes[19], &garden.nodes[20]);
    LinkNodes(&garden.nodes[16], &garden.nodes[19], &garden.nodes[20]);
    LinkNodes(&garden.nodes[17], &garden.nodes[19], &garden.nodes[20]);
    LinkNodes(&garden.nodes[18], &garden.nodes[19], &garden.nodes[20]);
    LinkNodes(&garden.nodes[19], &garden.nodes[21], &garden.nodes[21]);
    LinkNodes(&garden.nodes[20], &garden.nodes[21], &garden.nodes[21]);
    garden.nodes[21].childLeft = NULL;
    garden.nodes[21].childRight = NULL;
        
    // Find initial sha512 hash of the variable length data
    uint512 hash;
    static unsigned char empty[1];
    sph_sha512_init(&garden.context_sha2);
    sph_sha512(&garden.context_sha2, (begin == end ? empty : static_cast<const void*>(&begin[0])), (end - begin) * sizeof(begin[0]));
    sph_sha512_close(&garden.context_sha2, static_cast<void*>(&hash));

#ifdef MINOTAUR_DEBUG
    printf("** Initial hash:\t\t%s\n", hash.ToString().c_str());
    fflush(0);
#endif    

    // Assign algos to torture net nodes based on initial hash
    for (int i = 0; i < 22; i++)
        garden.nodes[i].algo = hash.ByteAt(i) % MINOTAUR_ALGO_COUNT;

    // Hardened garden gates on MinotaurX
    if (minotaurX)
        garden.nodes[21].algo = MINOTAUR_ALGO_COUNT;

    // Send the initial hash through the torture garden
    hash = TraverseGarden(&garden, hash, &garden.nodes[0], local);

#ifdef MINOTAUR_DEBUG
    printf("** Final hash:\t\t\t%s\n", uint256(hash).ToString().c_str());
    fflush(0);
#endif

    // Return truncated result
    return uint256(hash);
}

#endif // LCC_CRYPTO_MINOTAURX_MINOTAUR_H
