#define bitSet(reg, ind) (reg |= 1 << ind)
#define bitClear(reg, ind) (reg &= ~(1 << ind) )
#define bitCheck(reg, ind) ( (reg >> ind) & 1 )
#define bitToggle(reg, ind) (reg ^= (1 << ind))