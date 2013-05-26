#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raku.h"
#include "bstrlib.h"
#include "bstraux.h"
#include "slre.h"

/*
 Raku gramar is LL(1), verified by: http://smlweb.cpsc.ucalgary.ca/start.html 
  
 PROGRAM -> STATEMENTS .
STATEMENTS -> STATEMENT STATEMENTS | .
STATEMENT -> EXPRESSION | BLOCK | EMPTY_LINE | comment .
EXPRESSION -> VALUE PARAMETERS NL.
PARAMETERS_NONEMPTY -> PARAMETER PARAMETERS.
PARAMETERS-> PARAMETERS_NONEMPTY | .
PARAMETER -> BLOCK | VALUE .
EMPTY_LINE -> NL .
BLOCK -> ob STATEMENTS cb | op STATEMENTS cp | oa STATEMENTS ca.
NL -> nl | semicolon .
VALUE -> string | float | integer | symbol .
*/


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

struct RaValue2_ {
  char kind;
  union {
    int           integer;
    double        number;
    RaFunction  * func;
    struct RaRef_ ref;
  } value;
};


struct RaArray_ {
  struct RaValue2_ * data;
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
  RA_OP_NOP       = 'N',
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


struct TestArray_ {
  int           num;
  const char *  str;
};

typedef struct TestArray_ TestArray;


void * testarray_print_walker(void * array, int index, void * value, void * extra) {
  TestArray * elem = value;
  int stop;
  if (extra) { 
    stop = *((int*)extra);
  }
  if (index >= stop) return array;
  printf("Array index %i: %i %s\n", index, elem->num, elem->str);
  return NULL;
}

void testarray_init_elt(void * value) {
  TestArray * elem      = value;
  elem->num             = -1;
  elem->str             = NULL;
}

void testarray_done_elt(void * value) {
  TestArray * elem      = value;
  elem->num             = -1;
  //free(elem->str);
  elem->str             = NULL;
}


void test_arrays() {     
  const char * even = "even";
  const char * odd  = "odd";
  void * arr, *arr2;
  int index, arrsize, arr2size, elsize;
  elsize = sizeof(TestArray);
  arrsize = 10;
  arr2size = 20;
  
  TestArray  ta1, * ta2;
  arr = ra_array_alloc(arrsize, elsize);
  for(index = 0; index < arrsize; index++) {
    ta1.num = index;
    ta1.str = ((index % 2) ? odd  : even);
    ra_array_put(arr, arrsize, elsize, index, &ta1);
  }
  arr2 = ra_array_growcopy(arr, arrsize, elsize, arr2size);
  for (index = 0; index < arrsize; index ++) {
    ta2 = ra_array_get(arr2, arr2size, elsize, index);
    printf("Array index %i: %i %s\n", index, ta2->num, ta2->str);
  }
  ra_array_through(arr2, arr2size, elsize, arrsize, testarray_init_elt);
  ra_array_walk(arr2, arr2size, elsize, testarray_print_walker, &arr2size);
  ra_array_free(arr);
  ra_array_free(arr2);
}

void test_alloc(Raku * raku) {
  RaCell        * cell, * cell2, *cell3;
  cell = racell_alloc(raku);
  racell_free(raku, cell);
  cell = racell_alloc(raku);
  racell_free(raku, cell);
  cell = racell_alloc(raku);
  racell_free(raku, cell);
  cell  = racell_alloc(raku);
  cell2 = racell_alloc(raku);
  cell3 = racell_alloc(raku);
  racell_free(raku, cell);
  racell_free(raku, cell2);
  racell_free(raku, cell3);  
}

char * test_slre_callback(int n, char * capt, int size, void * extra) {
  char * str = calloc(1, size + 1); 
  strncpy(str, capt, size);
  fprintf(stdout, "Capture: %d %d %p >%s<\n", n, size, extra, str);
  free(str);
  return NULL;
}

void test_slre() {
  char re[128]; 
  char rebuf[128]; 
  char inbuf[1024]; 
  int error;
  int relen, inlen;
  // crude initialization;
  re[0] = '\0';
  
  while (TRUE) { 
    fprintf(stdout, "re (%s)>", re);
    fflush(stdout);
    if(!fgets(rebuf, 128, stdin)) return;
    relen = strlen(rebuf);
    if (relen < 2) { 
      fprintf(stdout, "Reusing RE:>%s<\n", re);
      // reuse old re
    } else {
      strncpy(re, rebuf, relen);      
      if (re[relen - 1] == '\n') re[relen - 1] = '\0';
      fprintf(stdout, "Using RE:>%s<\n", re);
    }
    
    fprintf(stdout, "text?>");
    fflush(stdout);  
    if(!fgets(inbuf, 1024, stdin)) return;  
    
    inlen = strlen(inbuf);
    if (inlen < 1) return;
    // crudely get rid of training \n
    if (inbuf[inlen - 1] == '\n') inbuf[inlen - 1] = '\0';
    
    error = slre_match(0, re, inbuf, inlen - 1, SLRE_CALLBACK, test_slre_callback, NULL);
    // error = slre_match(SLRE_NO_CAPTURE, re, inbuf, inlen - 1);
    if (error) {
      fprintf(stderr, "ERROR: %s\n", slre_error(error));  
    } else {
      fprintf(stdout, "Match OK.\n");
    }
  }
}




/* RE's to match Raku's lexemes. Note that they all must start with ^ and
 be enveloped with ( ) so the captures can be caught correctly. 
 All backlashes are doubled becayse of C string escaping, of course. */
#define BLOCK_COMMENT_RE        "^(#{[^}]+})"
#define LINE_COMMENT_RE         "^(#[^\\n]+)"
/* To keep things simpler on the parser, html-like escapes are used in stead of 
 c-like escapes. Like that a string ayways begins with a quote and always ends
 with a quote. Nevertheless these RE's need a lot of escaping myself so they will 
 work correctly from a constant C string! */
#define DQSTRING_RE             "^(\"[^\"]*?\")"
#define SQSTRING_RE             "^('[^']*?')"
#define BQSTRING_RE             "^(`[^`]*?`)"
#define NUMBER_RE               "^(-?\\d+\\.\\d*(e-?\\d+)?)"
#define DECINT_RE               "^(\\d+)"
#define HEXINT_RE               "^(0x\\x+)"
#define OCTINT_RE               "^(0o[01234567]+)"
#define BININT_RE               "^(0b[01]+)"
#define OPENBRACE_RE            "^(\\s*{\\s*)"
#define CLOSEBRACE_RE           "^(\\s*})"
#define OPENPAREN_RE            "^(\\s*\\(\\s*)"
#define CLOSEPAREN_RE           "^(\\s*\\))"
#define OPENBRACKET_RE          "^(\\s*\\[\\s*)"
#define CLOSEBRACKET_RE         "^(\\s*\\])"
#define ESCAPED_STAT_END_RE     "^(\\\\\b*(\\n|\\r\\n|\\r))"
#define STATEMENT_END_RE        "^(;|\\n|\\r\\n|\\r)"
#define COMMA_RE                "^(,\\b*(\\n|\\r\\n|\\r))"
#define WHITESPACE_RE           "^(\\b+)"
#define SYMBOL_RE               "^(:[^ \\t\\n\\r,;\\\\\"'`\\{\\}\\(\\)\\[\\]]+)"
#define WORD_RE                 "^([^ \\t\\n\\r,;\\\\\"'`\\{\\}\\(\\)\\[\\]]+)"
#define ERROR_RE                ".+"

enum RaTokenKinds_ { 
  BLOCK_COMMENT = 1,
  LINE_COMMENT,
  DQSTRING,
  SQSTRING,
  BQSTRING,
  NUMBER,
  DECINT,
  HEXINT,
  OCTINT,
  BININT,
  OPENBRACE,
  CLOSEBRACE,
  OPENPAREN,
  CLOSEPAREN,
  OPENBRACKET,
  CLOSEBRACKET,
  
  ESCAPED_STAT_END,
  STATEMENT_END,
  COMMA,
  WHITESPACE,
  SYMBOL,
  WORD,
  ERROR
};



typedef struct RaTokenRule_ RaTokenRule;

struct RaTokenRule_ {
  const char * regexp;
  int kind;
};


typedef struct RaToken_ RaToken;

struct RaToken_ {
  bstring text;
  int kind;
};

RaTokenRule ra_token_rules[] = {
  { BLOCK_COMMENT_RE    , BLOCK_COMMENT         }, 
  { LINE_COMMENT_RE     , LINE_COMMENT          }, 
  { DQSTRING_RE         , DQSTRING              }, 
  { SQSTRING_RE         , SQSTRING              }, 
  { BQSTRING_RE         , BQSTRING              }, 
  { NUMBER_RE           , DQSTRING              }, 
  { DECINT_RE           , DECINT                }, 
  { HEXINT_RE           , HEXINT                }, 
  { OCTINT_RE           , OCTINT                }, 
  { BININT_RE           , BININT                }, 
  { OPENBRACE_RE        , OPENBRACE             }, 
  { OPENPAREN_RE        , OPENPAREN             }, 
  { OPENBRACKET_RE      , OPENBRACKET           }, 
  { CLOSEBRACE_RE       , CLOSEBRACE            }, 
  { CLOSEPAREN_RE       , CLOSEPAREN            }, 
  { CLOSEBRACKET_RE     , CLOSEBRACKET          }, 
  { ESCAPED_STAT_END_RE , ESCAPED_STAT_END      },
  { STATEMENT_END_RE    , STATEMENT_END         },
  { COMMA_RE            , COMMA                 },
  { WHITESPACE_RE       , WHITESPACE            }, 
  { SYMBOL_RE           , SYMBOL                }, 
  { WORD_RE             , WORD                  }, 
  { ERROR_RE            , ERROR                 },
  { NULL                , 0                     }
};

/* fread wrapper for bstring */
size_t ra_bfread(void * buff, size_t esz, size_t eqty, void * parm) {
  return fread (buff, esz, eqty, (FILE *) parm);
}

/* fgetc wrapper for bstring */
int ra_bfgetc(void * parm) {
  return fgetc ((FILE *) parm);
}


RaCell * raku_tokenize_bstring(bstring str) {
  struct slre_captured cap;
  RaToken       token;
  int           kind;
  int           rdex;
  int           index        = 0 ;
  char        * cstr         =  bdata(str);
  int           stop         =  blength(str);
  bstring       text;
  int           ok;
  while (index < stop) { 
    rdex               = 0;
    ok                 = FALSE;
    RaTokenRule * rule = ra_token_rules + rdex;
    
    while (rule && rule->regexp && rule->kind ) {
      int no_match = slre_match(0, rule->regexp, cstr + index, stop - index, SLRE_CAPTURED, &cap, SLRE_IGNORE);
      if(!no_match) {
        text = blk2bstr(cap.ptr, cap.len);
        fprintf(stderr, "Rule %s, Token: %d >%s<\n", rule->regexp, rule->kind, bdata(text));  
        bdestroy(text);
        index += cap.len;
        ok                 = TRUE;
        break;
      } 
      rdex ++;
      rule = ra_token_rules + rdex;
    }
    if(!ok) { 
      fprintf(stderr, "No matches found! %d, %s\n", index, cstr + index);  
      return NULL;
    }
  }  
  return NULL;
}




RaCell * raku_tokenize_filename(const char * filename) {
  RaCell * res;
  bstring str;  
  FILE * file;
  file = fopen(filename, "rt");  
  if(!file) return NULL;
  str = bread(ra_bfread, file);
  res = raku_tokenize_bstring(str);
  bdestroy(str);
  fclose(file);
  return res;
}


int main(int argc, char * argv[]) {  
  Raku          * raku; 
  raku = raku_new(); 
  test_alloc(raku);
  raku_free(raku);
  // test_slre(); 
  raku_tokenize_filename("test/example1.rak");
  
  return 0;
}



