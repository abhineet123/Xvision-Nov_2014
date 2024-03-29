/* config.h.  Generated automatically by configure.  */
#define ENABLE_DITHER 1
#define STDC_HEADERS 1
#define HAVE_SYS_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_NETINET_IN_H 1
#define TIME_WITH_SYS_TIME 1
#define HAVE_VPRINTF 1
#define HAVE_STRTOD 1
#define HAVE_STRTOL 1
#define HAVE_RANDOM 1
#define HAVE_RAND 1
#define HAVE_LRAND48 1
#define HAVE_GETRUSAGE 1
/* #undef CLK_TCK */
/* #undef _HPUX_SOURCE */
/* #undef WORDS_BIGENDIAN */

/* Not sure if these are really needed -- probably cruft from days of yore. */
#define HAVE_RANDOM_DECL 1
#define HAVE_LRAND48_DECL 1

#ifdef DMALLOC
# define DMALLOC_FUNC_CHECK
# define strdup(str) xstrdup(str)
#endif
