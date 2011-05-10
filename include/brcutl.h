#ifndef _BRCUTL_H_INTERN_
#define _BRCUTL_H_

#ifdef __cplusplus
#error "brcutl proudly refuses to be compiled by that monstrosity!"
#endif

#include <stddef.h>

struct BrRuntime_;
typedef struct BrRuntime_ BrRuntime;

struct BrObject_;
typedef struct BrObject_ BrObject;

struct BrClosure_;
typedef struct BrClosure_ BrClosure;

typedef BrObject * (BrFunction)(BrObject * self, BrClosure * env, ...);

typedef void * (BrAllocFunction)(BrRuntime * vm, size_t size);
typedef void * (BrFreeFunction)(BrRuntime * vm, void *  ptr);


/** Default malloc function. */
void * BrMalloc(BrRuntime * vm, size_t size);

/** Default free function. */
void *  BrFree(BrRuntime * vm, void * ptr);

struct BrActs_;
typedef struct BrActs_ BrActs;

struct BrSymbol_;
typedef struct BrSymbol_ BrSymbol;

typedef volatile size_t BrRefcount;

struct BrHeader_;
typedef struct BrHeader_ BrHeader;

struct BrCacheEntry_;
typedef struct BrCacheEntry_ BrCacheEntry;

BrHeader  * BrObject_header(BrObject * object);

BrActs    * BrObject_acts(BrObject * object);
BrActs    * BrObject_acts_(BrObject * object, BrActs * acts);
BrRuntime * BrObject_runtime(BrObject * object);
BrRuntime * BrObject_runtime_(BrObject * object, BrRuntime * run);

BrObject  * BrObject_alloc(BrRuntime * run, size_t size, BrActs * acts); 
BrObject  * BrObject_free(BrRuntime * run, BrObject * object);
BrObject  * BrActs_lookup(BrActs * self, BrClosure * env, BrObject *key);

BrClosure * BrClosure_bind(BrObject * receiver, BrObject * message);



#endif
