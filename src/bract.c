#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

#include "brcutl.h"
#define BRCUTL_INTERN_ONLY
#include "brcutl_intern.h"


/** Default malloc function. */
void * BrMalloc(BrRuntime * vm, size_t size) {
  return calloc(size, 1);
}

/** Default free function. */
void *  BrFree(BrRuntime * vm, void * ptr) {
  free(ptr);
  return NULL;
}

/** Contains the hidden header fields of the BrObjects. */
struct BrHeader_ {
  BrRefcount    refs;
  BrRuntime   * run;
  BrActs      * acts;
};

struct BrCacheEntry_ {
  BrActs    * acts;
  BrObject  * selector;
  BrClosure * closure; 
};
 

struct BrRuntime_ { 
  BrActs          * symbols;
  BrActs          * acts_acts;
  BrActs          * object_acts;
  BrActs          * symbol_acts;
  BrActs          * closure_acts;
  BrObject        * sym_lookup;
  BrObject        * sym_add;
  BrObject        * sym_alloc;
  BrObject        * sym_delegate;
  BrClosure       * active_closure;
  BrAllocFunction * alloc;
  BrFreeFunction  * free;
  BrCacheEntry      cache[BR_METHOD_CACHE_SIZE];
};

/** Acts contains the lookup table in whch methods are looked up. 
* It must be allocated with BrObject_alloc so it will have a hidden header.
**/
struct BrActs_ {
  size_t     size;
  size_t     space;
  BrObject **keys;
  BrObject **values;
  BrActs    *parent;
};

struct BrClosure_ {
  BrFunction * function;
  int          arity;
  BrObject   * data;
};


BrHeader * BrObject_header(BrObject * object) {
  return BR_OBJECT_HEADER(object);
}

BrActs * BrObject_acts(BrObject * object) {
  return BR_OBJECT_ACTS(object);
}

BrActs * BrObject_acts_(BrObject * object, BrActs * acts) {
  return BR_OBJECT_ACTS_(object, acts);
}

BrRuntime * BrObject_runtime(BrObject * object) {
  return BR_OBJECT_RUNTIME(object);
}


BrRuntime * BrObject_runtime_(BrObject * object, BrRuntime * run) {
  return BR_OBJECT_RUNTIME_(object, run);
}


BrObject * BrObject_alloc(BrRuntime * run, size_t size, BrActs * acts) {
  char * ptr        = NULL;
  BrObject * result = NULL;
  size             += sizeof(BrHeader);
  ptr               = (run && run->alloc) ? run->alloc(run, size) : BrMalloc(run, size);
  if(!ptr)        return NULL;
  result            = (BrObject *)(ptr + sizeof(BrHeader));
  BrObject_runtime_(result, run);
  BrObject_acts_(result, acts);
  return (BrObject*) ptr;
}

BrObject * BrObject_free(BrRuntime * run, BrObject * object) {
  char * ptr = (char *) object;
  if(!ptr) return NULL;
  ptr       -= sizeof(BrHeader);
  if (run && run->free) run->free(run, ptr);
  else BrFree(run, ptr);
  return NULL;
}

BrObject *BrActs_lookup(BrActs * self, BrClosure * env, BrObject *key);


#define BR_CACHE_INDEX(ACTS, MESSAGE)                         \
        ((((unsigned)ACTS <<2) ^ ((unsigned)MESSAGE >>3)) &   \
        (BR_METHOD_CACHE_SIZE-1))
        
/*
BrObject * BrClosure_send_0(BrObject * receiver, BrClosure * closure) {
  closure->function(receiver, closure);
}

BrObject * BrClosure_send_1(BrObject * receiver, BrClosure * closure, 
                            BrObject * a1) {
  closure->function(receiver, closure, a1);
}

BrObject * BrClosure_send_2(BrObject * receiver, BrClosure * closure, 
                            BrObject * a1, 
                            BrObject * a2) {
  closure->function(receiver, closure, a1, a2);
}

BrObject * BrClosure_send_3(BrObject * receiver, BrClosure * closure, 
                            BrObject * a1, 
                            BrObject * a2,
                            BrObject * a3) {
  closure->function(receiver, closure, a1, a2, a3);
}

BrObject * BrClosure_send_4(BrObject * receiver, BrClosure * closure, 
                            BrObject * a1, 
                            BrObject * a2,
                            BrObject * a3,
                            BrObject * a4) {
  closure->function(receiver, closure, a1, a2, a3);
}


BrObject * BrClosure_send_5(BrObject * receiver, BrClosure * closure, 
                            BrObject * a1, 
                            BrObject * a2,
                            BrObject * a3,
                            BrObject * a4) {
  closure->function(receiver, closure, a1, a2, a3);
}
*/

#define BR_MAX_ARGS 16

BrObject * BrObject_send(BrObject * receiver, BrObject * message, ...);
BrClosure * BrClosure_bind(BrObject * receiver, BrObject * message);

// todo: cache? 
BrObject * BrObject_send(BrObject * receiver, BrObject * message, ...) {
  BrObject * a[BR_MAX_ARGS];
  int index = 0;
  va_list args;
  BrClosure * closure = (BrClosure*) BrClosure_bind(receiver, message);
  BrFunction * method = closure->function;
  va_start(args, message);
  for (index = 0; index < closure->arity && index < BR_MAX_ARGS; index++) {
    a[index] = va_arg(args, BrObject *);
  }
  va_end(args); 
  switch (closure->arity) {
    case  0: return method(receiver, closure);
    case  1: return method(receiver, closure, a[0]);
    case  2: return method(receiver, closure, a[0], a[1]);
    case  3: return method(receiver, closure, a[0], a[1], a[2]);
    case  4: return method(receiver, closure, a[0], a[1], a[2],
                                                        a[3]);
    case  5: return method(receiver, closure, a[0], a[1], a[2],
                                                        a[3], a[4]);
    case  6: return method(receiver, closure, a[0], a[1], a[2],
                                                        a[3], a[4], a[5]);
    case  7: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7]);
    case  8: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7]);
    case  9: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7],  a[8]);

    case 10: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7],  a[8], a[9]
                                               );
                                                                                    case 11: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7],  a[8], a[9],
                                              a[10]);
    case 12: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7],  a[8], a[9],
                                              a[10], a[11]);
                                
    case 13: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7],  a[8], a[9],
                                              a[10], a[11], a[12]);
    case 14: return method(receiver, closure, a[0],  a[1],  a[2],  a[3], a[4],
                                              a[5],  a[6],  a[7],  a[8], a[9],
                                              a[10], a[11], a[12], a[13]);
                                                  
    case 15: return method(receiver, closure, a[0] , a[1] , a[2] , a[3] , a[4] ,
                                              a[5] , a[6] , a[7] , a[8] , a[9] ,
                                              a[10], a[11], a[12], a[13], a[14]
                                              );
                                                        
    case 16: return method(receiver, closure, a[0] , a[1] , a[2] , a[3] , a[4] ,
                                              a[5] , a[6] , a[7] , a[8] , a[9] ,
                                              a[10], a[11], a[12], a[13], a[14],
                                              a[15]);
    default:return method(receiver, closure);
  } 
}

BrClosure * BrClosure_bind(BrObject * receiver, BrObject * message) {
  BrClosure     * closure = NULL;
  BrRuntime     * run     = BR_OBJECT_RUNTIME(receiver);
  BrActs        * acts    = BR_OBJECT_ACTS(receiver);
  BrCacheEntry  * entry   = run->cache + BR_CACHE_INDEX(acts, message);
  if (entry && entry->acts == acts && entry->selector == message) {
    return entry->closure;
  }
  if ((message == run->sym_lookup) && 
      (((BrActs *)receiver) == run->acts_acts)) {
    closure = (BrClosure *) BrActs_lookup(acts, NULL, message); 
  } else {
    closure = (BrClosure *) Br_send(acts, run->sym_lookup, message);
  }
  entry->acts     = acts;
  entry->selector = message;
  entry->closure  = closure;
  return closure;
} 


BrRuntime br_runtime_default = { NULL, NULL, NULL, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL};




struct BrSymbol_ {
  char * string;
};

struct BrObject {
};

BrObject * BrSymbol_new(BrRuntime * run, char * string) {
  BrObject * self = BrObject_alloc(run, sizeof(BrSymbol),  run->symbol_acts);
  ((BrSymbol*)self)->string = string;
  return self;
}


BrObject * BrClosure_new(BrRuntime * run, BrFunction * func, int arity, BrObject * data) {
  BrObject * self = BrObject_alloc(run, sizeof(BrClosure), run->closure_acts);
  ((BrClosure*)self)->function = func;
  ((BrClosure*)self)->arity    = arity;
  ((BrClosure*)self)->data     = data;
  return self;
}




#define MIX_PARENT(PARENT_TYPE, SELF, MEMBER) \
  ((PARENT_TYPE *)(((char *)SELF) - offsetof(PARENT_TYPE, MEMBER)))

int main(void) {
  return 0;
}

#ifdef _THIS_IS_COMMENT_

/* Copyright (c) 2007 by Ian Piumarta.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * The Software is provided "as is".  Use entirely at your own risk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ICACHE 1  /* nonzero to enable point-of-send inline cache */
#define MCACHE 1  /* nonzero to enable global method cache        */

struct vtable;
struct object;
struct closure;
struct symbol;

typedef struct object *(*imp_t)(struct closure *closure, struct object *receiver, ...);

struct vtable
{
  struct vtable  *_vt[0];
  int             size;
  int             tally;
  struct object **keys;
  struct object **values;
  struct vtable  *parent;
};

struct object {
  struct vtable *_vt[0];
};

struct closure
{
  struct vtable *_vt[0];
  imp_t    method;
  struct object *data;
};

struct symbol
{
  struct vtable *_vt[0];
  char          *string;
};

struct vtable *SymbolList= 0;

struct vtable *vtable_vt;
struct vtable *object_vt;
struct vtable *symbol_vt;
struct vtable *closure_vt;

struct object *s_addMethod = 0;
struct object *s_allocate  = 0;
struct object *s_delegated = 0;
struct object *s_lookup    = 0;

extern inline void *alloc(size_t size)
{
  struct vtable **ppvt= (struct vtable **)calloc(1, sizeof(struct vtable *) + size);
  return (void *)(ppvt + 1);
}

struct object *symbol_new(char *string) {
  struct symbol *symbol = (struct symbol *)alloc(sizeof(struct symbol));
  symbol->_vt[-1] = symbol_vt;
  symbol->string = strdup(string);
  return (struct object *)symbol;
}

struct object *closure_new(imp_t method, struct object *data)
{
  struct closure *closure = (struct closure *)alloc(sizeof(struct closure));
  closure->_vt[-1] = closure_vt;
  closure->method  = method;
  closure->data    = data;
  return (struct object *) closure;
}

struct object *vtable_lookup(struct closure *closure, struct vtable *self, struct object *key);

#if ICACHE
# define send(RCV, MSG, ARGS...) ({       \
      struct        object   *r = (struct object *)(RCV); \
      static struct vtable   *prevVT  = 0;      \
      static struct closure  *closure = 0;      \
      register struct vtable *thisVT  = r->_vt[-1];   \
      thisVT == prevVT            \
  ?  closure            \
  : (prevVT  = thisVT,          \
     closure = bind(r, (MSG)));       \
      closure->method(closure, r, ##ARGS);      \
    })
#else
# define send(RCV, MSG, ARGS...) ({       \
      struct object  *r = (struct object *)(RCV);   \
      struct closure *c = bind(r, (MSG));     \
      c->method(c, r, ##ARGS);          \
    })
#endif

#if MCACHE
struct entry {
  struct vtable  *vtable;
  struct object  *selector;
  struct closure *closure;
} MethodCache[8192];
#endif

struct closure *bind(struct object *rcv, struct object *msg)
{
  struct closure *c;
  struct vtable  *vt = rcv->_vt[-1];
#if MCACHE
  struct entry   *cl = MethodCache + ((((unsigned)vt << 2) ^ ((unsigned)msg >> 3)) & ((sizeof(MethodCache) / sizeof(struct entry)) - 1));
  if (cl->vtable == vt && cl->selector == msg)
    return cl->closure;
#endif
  c = ((msg == s_lookup) && (rcv == (struct object *)vtable_vt))
    ? (struct closure *)vtable_lookup(0, vt, msg)
    : (struct closure *)send(vt, s_lookup, msg);
#if MCACHE
  cl->vtable   = vt;
  cl->selector = msg;
  cl->closure  = c;
#endif
  return c;
}

struct vtable *vtable_delegated(struct closure *closure, struct vtable *self)
{
  struct vtable *child= (struct vtable *)alloc(sizeof(struct vtable));
  child->_vt[-1] = self ? self->_vt[-1] : 0;
  child->size    = 2;
  child->tally   = 0;
  child->keys    = (struct object **)calloc(child->size, sizeof(struct object *));
  child->values  = (struct object **)calloc(child->size, sizeof(struct object *));
  child->parent  = self;
  return child;
}

struct object *vtable_allocate(struct closure *closure, struct vtable *self, int payloadSize)
{
  struct object *object = (struct object *)alloc(payloadSize);
  object->_vt[-1] = self;
  return object;
}

imp_t vtable_addMethod(struct closure *closure, struct vtable *self, struct object *key, imp_t method)
{
  int i;
  for (i = 0;  i < self->tally;  ++i)
    if (key == self->keys[i])
      return ((struct closure *)self->values[i])->method = method;
  if (self->tally == self->size)
    {
      self->size  *= 2;
      self->keys   = (struct object **)realloc(self->keys,   sizeof(struct object *) * self->size);
      self->values = (struct object **)realloc(self->values, sizeof(struct object *) * self->size);
    }
  self->keys  [self->tally  ] = key;
  self->values[self->tally++] = closure_new(method, 0);
  return method;
}

struct object *vtable_lookup(struct closure *closure, struct vtable *self, struct object *key)
{
  int i;
  for (i = 0;  i < self->tally;  ++i)
    if (key == self->keys[i])
      return self->values[i];
  if (self->parent)
    return send(self->parent, s_lookup, key);
  fprintf(stderr, "lookup failed %p %s\n", self, ((struct symbol *)key)->string);
  return 0;
}

struct object *symbol_intern(struct closure *closure, struct object *self, char *string)
{
  struct object *symbol;
  int i;
  for (i = 0;  i < SymbolList->tally;  ++i)
    {
      symbol = SymbolList->keys[i];
      if (!strcmp(string, ((struct symbol *)symbol)->string))
  return symbol;
    }
  symbol = symbol_new(string);
  vtable_addMethod(0, SymbolList, symbol, 0);
  return symbol;
}

void init(void)
{
  vtable_vt = vtable_delegated(0, 0);
  vtable_vt->_vt[-1] = vtable_vt;

  object_vt = vtable_delegated(0, 0);
  object_vt->_vt[-1] = vtable_vt;
  vtable_vt->parent = object_vt;

  symbol_vt  = vtable_delegated(0, object_vt);
  closure_vt = vtable_delegated(0, object_vt);

  SymbolList = vtable_delegated(0, 0);

  s_lookup    = symbol_intern(0, 0, "lookup");
  s_addMethod = symbol_intern(0, 0, "addMethod");
  s_allocate  = symbol_intern(0, 0, "allocate");
  s_delegated = symbol_intern(0, 0, "delegated");

  vtable_addMethod(0, vtable_vt, s_lookup,    (imp_t)vtable_lookup);
  vtable_addMethod(0, vtable_vt, s_addMethod, (imp_t)vtable_addMethod);

  send(vtable_vt, s_addMethod, s_allocate,    vtable_allocate);
  send(vtable_vt, s_addMethod, s_delegated,   vtable_delegated);
}

#endif

