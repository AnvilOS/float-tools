
#include "pow5.h"

const uint32_t small_pow_5[] =
{
    // 1   2   3    4     5     6      7       8
    1, 5, 25, 125, 625, 3125, 15625, 78125, 390625,
    // 9       10        11        12          13
    1953125, 9765625, 48828125, 244140625, 1220703125,
};

const uint64_t med_pow_5[] =
{
    0x16BCC41E9ULL, // 14
    0x71AFD498DULL,
    0x2386F26FC1ULL,
    0xB1A2BC2EC5ULL,
    0x3782DACE9D9ULL,
    0x1158E460913DULL,
    0x56BC75E2D631ULL,
    0x1B1AE4D6E2EF5ULL,
    0x878678326EAC9ULL,
    0x2A5A058FC295EDULL,
    0xD3C21BCECCEDA1ULL,
    0x422CA8B0A00A425ULL,
    0x14ADF4B7320334B9ULL,
    0x6765C793FA10079DULL,  // 27
};

const uint32_t five_e32[] =
{
    0x85acef81, 0x2d6d415b, 0x000004ee
};

const uint32_t five_e64[] =
{
    0xbf6a1f01, 0x6e38ed64, 0xdaa797ed, 0xe93ff9f4, 0x00184f03
};

const uint32_t five_e128[] =
{
    0x2e953e01, 0x03df9909, 0x0f1538fd, 0x2374e42f, 0xd3cff5ec, 0xc404dc08, 0xbccdb0da, 0xa6337f19,
    0xe91f2603, 0x0000024e
};

const uint32_t five_e256[] =
{
    0x982e7c01, 0xbed3875b, 0xd8d99f72, 0x12152f87, 0x6bde50c6, 0xcf4a6e70, 0xd595d80f, 0x26b2716e,
    0xadc666b0, 0x1d153624, 0x3c42d35a, 0x63ff540e, 0xcc5573c0, 0x65f9ef17, 0x55bc28f2, 0x80dcc7f7,
    0xf46eeddc, 0x5fdcefce, 0x000553f7
};

const uint32_t five_e512[] =
{
    0xfc6cf801, 0x77f27267, 0x8f9546dc, 0x5d96976f, 0xb83a8a97, 0xc31e1ad9, 0x46c40513, 0x94e65747,
    0xc88976c1, 0x4475b579, 0x28f8733b, 0xaa1da1bf, 0x703ed321, 0x1e25cfea, 0xb21a2f22, 0xbc51fb2e,
    0x96e14f5d, 0xbfa3edac, 0x329c57ae, 0xe7fc7153, 0xc3fc0695, 0x85a91924, 0xf95f635e, 0xb2908ee0,
    0x93abade4, 0x1366732a, 0x9449775c, 0x69be5b0e, 0x7343afac, 0xb099bc81, 0x45a71d46, 0xa2699748,
    0x8cb07303, 0x8a0b1f13, 0x8cab8a97, 0xc1d238d9, 0x633415d4, 0x0000001c
};

const uint32_t five_e1024[] =
{
    0x2919f001, 0xf55b2b72, 0x6e7c215b, 0x1ec29f86, 0x991c4e87, 0x15c51a88, 0x140ac535, 0x4c7d1e1a,
    0xcc2cd819, 0x0ed1440e, 0x896634ee, 0x7de16cfb, 0x1e43f61f, 0x9fce837d, 0x231d2b9c, 0x233e55c7,
    0x65dc60d7, 0xf451218b, 0x1c5cd134, 0xc9635986, 0x922bbb9f, 0xa7e89431, 0x9f9f2a07, 0x62be695a,
    0x8e1042c4, 0x045b7a74, 0x1abe1de3, 0x8ad822a5, 0xba34c411, 0xd814b505, 0xbf3fdeb3, 0x8fc51a16,
    0xb1b896bc, 0xf56deeec, 0x31fb6bfd, 0xb6f4654b, 0x101a3616, 0x6b7595fb, 0xdc1a47fe, 0x80d98089,
    0x80bda5a5, 0x9a202882, 0x31eb0f66, 0xfc8f1f90, 0x976a3310, 0xe26a7b7e, 0xdf68368a, 0x3ce3a0b8,
    0x8e4262ce, 0x75a351a2, 0x6cb0b6c9, 0x44597583, 0x31b5653f, 0xc356e38a, 0x35faaba6, 0x0190fba0,
    0x9fc4ed52, 0x88bc491b, 0x1640114a, 0x005b8041, 0xf4f3235e, 0x1e8d4649, 0x36a8de06, 0x73c55349,
    0xa7e6bd2a, 0xc1a6970c, 0x47187094, 0xd2db49ef, 0x926c3f5b, 0xae6209d4, 0x2d433949, 0x34f4a3c6,
    0xd4305d94, 0xd9d61a05, 0x00000325
};

const xint_t big_pow_5[] =
{
    // 32
    { 3, 3, (uint32_t *)five_e32 },
    // 64
    { 5, 5, (uint32_t *)five_e64 },
    // 128
    { 10, 10, (uint32_t *)five_e128 },
    // 256
    { 19, 19, (uint32_t *)five_e256 },
    // 512
    { 38, 38, (uint32_t *)five_e512 },
    // 1024
    { 75, 75, (uint32_t *)five_e1024 },
};

uint32_t xint_mul_5exp(xint_t x, int e)
{
    if (x->size == 0)
    {
        return 0;
    }

    int lll = e & 0x1f;
    if (lll < 28)
    {
        if (lll < 14)
        {
            xint_mul_1(x, x, small_pow_5[lll]);
            e &= ~lll;
        }
        else
        {
            xint_mul_2(x, med_pow_5[lll - 14]);
            e &= ~lll;
        }
    }
    else if (lll & 0x7)
    {
        lll &= 7;
        xint_mul_1(x, x, small_pow_5[lll]);
        e &= ~lll;
        lll = e & 0x1f;
        if (lll < 28)
        {
            xint_mul_2(x, med_pow_5[lll - 14]);
            e &= ~lll;
        }
    }
    
    e >>= 5;
    unsigned ndx = 0;

    while (e && (ndx < (sizeof(big_pow_5) / sizeof(big_pow_5[0]) - 1)))
    {
        if (e & 1)
        {
            xint_mul(x, big_pow_5[ndx], x);
        }
        ++ndx;
        e >>= 1;
    }
    while (e)
    {
        xint_mul(x, big_pow_5[ndx], x);
        --e;
    }
    return 0;
}
