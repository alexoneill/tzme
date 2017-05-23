/**
 * @file smem.h
 * @brief Definitions for sized memory
 *
 * @author Alex O'Neill <me@aoneill.me>
 */

#ifndef __SMEM_H__
#define __SMEM_H__

#include <assert.h>
#include <stdlib.h>

/** @brief A container for bytes with associated length, sized memory */
typedef struct {
  /** @brief The length of memory */
  size_t len;

  /** @brief The contents of memory */
  char *mem;
} smem_t;

// Methods
int smem_init(smem_t *mem, size_t len);
int smem_valid(smem_t *mem);
void smem_free(smem_t *mem);

#endif /* __SMEM_H__ */
