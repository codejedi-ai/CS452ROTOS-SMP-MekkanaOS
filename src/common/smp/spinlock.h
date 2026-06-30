#ifndef SMP_SPINLOCK_H
#define SMP_SPINLOCK_H

#include <stdint.h>

typedef volatile uint32_t spinlock_t;

#define SPINLOCK_INIT 0u

void spinlock_init(spinlock_t *lock);
void spinlock_lock(spinlock_t *lock);
int spinlock_trylock(spinlock_t *lock);
void spinlock_unlock(spinlock_t *lock);

#endif /* SMP_SPINLOCK_H */
