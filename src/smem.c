/**
 * @file smem.c
 * @brief Implementation of sized memory
 *
 * @author Alex O'Neill <me@aoneill.me>
 * @bugs No known bugs.
 */

#include "smem.h"

/**
 * @brief Initialize a size memory
 *
 * @param mem The sized memory to initialize
 * @param len The desired length
 *
 * @return A non-zero value on success
 */
int smem_init(smem_t *mem, size_t len) {
  assert(mem);

  mem->len = len;
  mem->mem = calloc(mem->len, sizeof(char));
  return !!(mem->mem);
}

/**
 * @brief Check the validity of sized memory
 *
 * @param mem The sized memory to check
 *
 * @return A non-zero value if the memory is valid
 */
int smem_valid(smem_t *mem) {
  assert(mem);
  return !!(mem->mem);
}

/**
 * @brief De-allocate a sized memory
 *
 * @param mem The sized memory to deallocate
 */
void smem_free(smem_t *mem) {
  assert(mem);
  free(mem->mem);

  mem->len = 0;
  mem->mem = NULL;
}

