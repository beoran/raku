#ifndef _BRCUTL_H_INTERN_
#define _BRCUTL_H_INTERN_

#ifndef BRCUTL_INTERN_ONLY
#error "This header is for internal use only! Do not include it!"
#endif

#define BR_METHOD_CACHE_SIZE 8192

#define BR_OBJECT_HEADER(OBJECT) (OBJECT ?               \
        ((BrHeader*)(((char *)OBJECT)-sizeof(BrHeader))) \
        : NULL )

#define BR_OBJECT_ACTS(OBJECT) (OBJECT ?                       \
        ((BrHeader*)(((char *)OBJECT)-sizeof(BrHeader)))->acts \
        : NULL )


#define BR_OBJECT_ACTS_(OBJECT, ACTS) (OBJECT ?                       \
        ((BrHeader*)(((char *)OBJECT)-sizeof(BrHeader)))->acts = ACTS \
        : NULL )

#define BR_OBJECT_RUNTIME(OBJECT) (OBJECT ?                       \
        ((BrHeader*)(((char *)OBJECT)-sizeof(BrHeader)))->run     \
        : NULL )

#define BR_OBJECT_RUNTIME_(OBJECT, RUN) (OBJECT ?                   \
        ((BrHeader*)(((char *)OBJECT)-sizeof(BrHeader)))->run = RUN \
        : NULL )

#define BR_CACHE_INDEX(ACTS, MESSAGE)                         \
        ((((unsigned)ACTS <<2) ^ ((unsigned)MESSAGE >>3)) &   \
        (BR_METHOD_CACHE_SIZE-1))
 

#endif
