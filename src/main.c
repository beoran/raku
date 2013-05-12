
#include "bstrlib.h"
#include "bstraux.h"



struct RaRuntime_; 
struct RaValue;

typedef struct RaRuntime * RaFunction(struct RaRuntime_ *);
typedef void RaDestructor(void *);

struct RaRefcount_ {
  RaDestructor * destroy;
  int            count;
};

typedef struct tagbstring RaString;

union RaPointer_ {
  RaString    * str;
  void        * data;
};

struct RaRef_ {
  struct  RaRefcount_   ref;
  union   RaPointer_    ptr;
};

struct RaValue_ {
  char kind;
  union {
    int           integer;
    double        number;
    RaFunction  * func;
    struct RaRef_ ref;
  } value;
};


struct RaArray_ {
  struct RaValue_ * data;
  int               size;
  int               space;
};


enum RaRuntimeModes{
  RA_MODE_INTDATA = 'I',
  RA_MODE_DBLDATA = 'D',
  RA_MODE_STRDATA = 'S',
  RA_MODE_EVAL    = 'E',
};

enum RaOpcodes{
  /* Do nothing */
  RA_OP_NOP       = 'N'
  /* Integer data follows. */
  RA_OP_INTDATA   = 'I',
  /* Double data follows. */
  RA_OP_DBLDATA   = 'D',
  /* String data follows.*/
  RA_OP_STRDATA   = 'S',
  /* */
  RA_OP_ALLOC     = 'A',
  
};

/*
opcode ideas: 
1) Low level: 
STORE
value address -> address+1 ([address] = value)
FETCH
address -> value
CALL
( args ) address -> ( result )
DROP
value ->
DUP
value -> value value



*/


struct RaRuntime_ {
  RaString * program;
  struct RaArray_  * stack;
  struct RaArray_  * dict;  
  int                pc;
  int                mode;
};





int main(int argc, char * argv[]) {
  
  
  return 0;
}