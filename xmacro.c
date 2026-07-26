#define LIST_OF_COLOR \
	X(red, 111)       \
	X(blue, 222)      \
	X(yellow, 333)

#define X(a, b) a,
typedef enum { LIST_OF_COLOR } color;
#undef X

/* cc -E -o xmacro.i xmacro.c */