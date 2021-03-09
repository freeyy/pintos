/* Fixed point type. Only compatible with operations in fixed-point.h */
typedef int32_t fix;

/* Number of bits used as decimal part. */
#define SHIFT_BIT 14


/* Input type: fix, fix; Return type: fix; */
#define F_ADD(A, B) ((A) + (B))

/* Input type: fix, int; Return type: fix; */
#define F_ADD_INT(A, B) ((A) + N_INT_F(B))

/* Input type: fix, fix; Return type: fix; */
#define F_SUB(A, B) ((A) - (B))

/* Input type: fix, int; Return type: fix; */
#define F_SUB_INT(A, B) ((A) - N_INT_F((B)))

/* Input type: fix, fix; Return type: fix; */
#define F_MULT(A, B) ((fix)((((int64_t) (A)) * (int64_t) (B)) >> SHIFT_BIT))

/* Input type: fix, int; Return type: fix; */
#define F_MULT_INT(A, B) ((A) * (B))

/* Input type: fix, fix; Return type: fix; */
#define F_DIV(A, B) ((fix)((((int64_t) (A)) << SHIFT_BIT) / (B)))

/* Input type: fix, int; Return type: fix; */
#define F_DIV_INT(A, B) ((A) / (B))

/* Input type: int; Return type: fix; Conver int to fix */
#define N_INT_F(A) ((fix)((A) << SHIFT_BIT))

/* Input type: fix; Return type: int; rounding to nearest; */
#define F_INT_N(A) ((A) >= 0 ? (((A) + (1 << (SHIFT_BIT - 1))) >> SHIFT_BIT) \
                             : (((A) - (1 << (SHIFT_BIT - 1))) >> SHIFT_BIT))
