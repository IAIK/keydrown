#include "random.h"
#include <asm/div64.h>
#include <linux/random.h>

// ----------------------------------------------------------------------------------------------------------
uint32_t get_random(uint32_t low, uint32_t high) {
  uint32_t rawRand;
  uint32_t rnd;
  uint32_t N = high - low + 1;
  uint32_t randTruncation = 0;
  uint64_t tmp = (uint64_t)1 << (uint64_t)32;

  do_div(tmp, N);
  randTruncation = tmp * N;

  do {
    get_random_bytes_arch(&rawRand, sizeof(rawRand));
  } while (rawRand >= randTruncation);

  rnd = (rawRand % N) + low;
  return rnd;
}
