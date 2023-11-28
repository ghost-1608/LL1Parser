/*
  An LL1 parser written in C. I re-wrote the parser I wrote in Python in C.
  Makes use of a hash table in place of the Python Dict used in the original code.

  The only drawback is that the hash table isn't a true hash table. It functions
  by storing the keys and values in two separate arrays, using the same index.

  (ghost-1608 26/11/23)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaring data structures necessary
union HTABLE_UNION;

typedef enum { HTBL_ERR = -1, HTBL_NULL, HTABLE_t, STR_t } HTBL_ENM;

typedef struct
{
  size_t size;
  char* keys;
  HTBL_ENM type;
  union HTABLE_UNION* data;
} HTABLE;

union HTABLE_UNION
{
  char* str;
  HTABLE* htbl;
};

typedef union HTABLE_UNION HTBL_U;

typedef struct
{
  size_t size;
  size_t top;
  char* data;
} CHRSTK;
// end declarations

// Stack functions
void cstkinit(CHRSTK*, size_t);
int cstkpush(CHRSTK*, char);
size_t cstklen(CHRSTK*);
int cstkextend(CHRSTK*, char*, size_t);
int cstkcmp(CHRSTK*, char);
char cstkpop(CHRSTK*);
void cstkfree(CHRSTK*);
// end

// (pseudo) Hash Table functions
HTABLE* htblcreate(char*, HTBL_ENM, HTABLE**, char**, size_t);
int htblin(HTABLE*, char);
HTBL_ENM htblat(HTABLE*, char, char**, HTABLE**);
void htblfree(HTABLE*);
void htblprint(HTABLE*);
// end

// Definitions to make sure grammar rules are flexible
#define EPSILON '#'
#define EPSILON_str "#"
#define isnonterminal(x, table) htblin(table, x) // Wrapper to check if key exists in table
// end

// The good stuff :)

/*
 * LL1 implementation
 * Needs input string, starting grammar symbol, and parse table
 * Returns book value on acceptance or not
 */
int ll1(char* inp_arg, char start_symbol, HTABLE* table)
{
  CHRSTK stack; // Declaring the stack on the stack :)
  int ret = 0;
  char inp[64] = {0}; // Creating string buffer to store input string
 
  cstkinit(&stack, 64);
  
  cstkpush(&stack, '$');
  cstkpush(&stack, start_symbol);
  strncpy(inp, inp_arg, 64);
  strcat(inp, "$");

  // Main loop
  int i = 0;
  while (cstklen(&stack) > 1)
  {
    char chr = inp[i];
    char symbol = cstkpop(&stack);

    if (isnonterminal(symbol, table))
    {
      // Replace stack symbol suing parse tables
      char* new_symbols;
      HTABLE* htmp;
      htblat(table, symbol, NULL, &htmp);
      htblat(htmp, chr, &new_symbols, NULL);
      cstkextend(&stack, new_symbols, strlen(new_symbols));
    }
    else if (chr == EPSILON || chr == symbol)
      // Move to next input string
      i++;
  }

  // Final acceptance condition
  if (cstkcmp(&stack, '$') && inp[i - 1] == '$')
  {
    cstkpop(&stack);
    ret = 1;
  }

  cstkfree(&stack);
  return ret;
}

/*
 * Initialises stack
 * Needs stack, size
 */
void cstkinit(CHRSTK* cstk, size_t size)
{
  cstk->data = (char*) malloc(sizeof(char) * size);
  cstk->top = 0;
  cstk->size = size;
}

/*
 * Pushes to stack
 * Needs stack, char
 * Returns status
 */
inline int cstkpush(CHRSTK* cstk, char x)
{
  if (cstk->top == cstk->size) return -1;
  cstk->data[cstk->top++] = x;
}

/*
 * Returns len of stack
 * Needs stack
 * Returns len
 */
inline size_t cstklen(CHRSTK* cstk)
{
  return cstk->top;
}

/*
 * Push series of chars to stack
 * Needs stack, char array
 * Returns status
 */
inline int cstkextend(CHRSTK* cstk, char* str, size_t len)
{
  if (cstk->top + len > cstk->size) return -1;
  for (size_t i = len - 1; i != (size_t)-1; i--)
    cstkpush(cstk, str[i]);
}

/*
 * Pops from stack
 * Needs stack
 * Returns char
 */
inline char cstkpop(CHRSTK* cstk)
{
  if (cstk->top == 0) return 0;
  return cstk->data[--cstk->top];
}

/*
 * Compares stack with char
 * Needs stack, char
 * Returns true if it matches, false else
 * NOTE: Not usual stack operation. Does
 *       two functions: checks if length is
 *       one and if top char matches
 */
inline int cstkcmp(CHRSTK* cstk, char x)
{
  if (cstklen(cstk) != 1)
    return 0;
  return cstk->data[cstk->top] == x;
}

/*
 * Frees dynamically allocated parts of the stack
 * Needs stack
 */
void cstkfree(CHRSTK* cstk)
{
  free(cstk->data);
  cstk->top = -1;
  cstk->size = 0;
}

/*
 * Creates new hash table
 * Needs array of keys, value type, array of hash table values, or
 * string values as applicable, and size
 * Returns hash table address
 */
HTABLE* htblcreate(char* keys, HTBL_ENM type, HTABLE** hvalues, char** svalues, size_t size)
{
  HTABLE* htbl = (HTABLE*) malloc(sizeof(HTABLE));
  
  htbl->size = size;
  htbl->type = type;
  htbl->keys = (char*) malloc(sizeof(char) * size);
  htbl->data = (HTBL_U*) malloc(sizeof(HTBL_U) * size);
  
  for (size_t i = 0; i < size; i++)
  {
    htbl->keys[i] = keys[i];
    switch (type)
    {
      case HTABLE_t:
	htbl->data[i].htbl = hvalues[i];
	break;
      case STR_t:
	htbl->data[i].str = svalues[i];
    }	
  }

  return htbl;
}

/*
 * Checks if key is in hash table
 * Needs hash table, char
 * Returns bool value
 */
int htblin(HTABLE* htbl, char x)
{
  for (size_t i = 0; i < htbl->size; i++)
    if (htbl->keys[i] == x)
      return 1;
  return 0;
}

/*
 * Fetches char at location of hash table
 * Needs hash table, key, string or hash table buffer as needed
 * Returns type
 */
HTBL_ENM htblat(HTABLE* htbl, char key, char** sbuffer, HTABLE** hbuffer)
{
  if (!htbl->size)
    return HTBL_ERR;

  size_t index = -1;
  
  for (size_t i = 0; i < htbl->size; i++)
    if (htbl->keys[i] == key)
      index = i;

  if (index == -1)
    return HTBL_ERR;
  
  switch (htbl->type)
  {
    case HTABLE_t:
      if (!hbuffer) return HTBL_NULL;
      *hbuffer = htbl->data[index].htbl;
      break;
    case STR_t:
      if (!sbuffer) return HTBL_NULL;
      *sbuffer = htbl->data[index].str;
  }

  return htbl->type;
}

/*
 * Frees hash table
 * Needs hash table
 */
void htblfree(HTABLE* htbl)
{
  free(htbl->keys);
  free(htbl->data);
  htbl->size = -1;
  free(htbl);
}

/*
 * Prints hash table python Dict style
 * Needs hash table
 */
void htblprint(HTABLE* htbl)
{
  printf("{");
  for (size_t i = 0; i < htbl->size; i++)
  {
    printf("'%c': ", htbl->keys[i]);
    switch (htbl->type)
    {
      case HTABLE_t:
	htblprint(htbl->data[i].htbl);
	break;
      case STR_t:
	printf("\"%s\"", htbl->data[i].str);
    }
    if (i < htbl->size - 1) printf(", ");
  }
  printf("}");
}

// Ryan Gosling code
int main()
{
  // Sample parse table (ONLY FOR PRINTING)
  char parse_table[] =
    "+-------------------------------------+\n"
    "|   ||  (  | i  | $ |  +  |  *  |  )  |\n"
    "|=====================================|\n"
    "| S ||  E$ | E$ |   |     |     |     |\n"
    "| E ||  TF | TF |   |     |     |     |\n"
    "| F ||     |    | # | +TF |     |  #  |\n"
    "| T ||  GU | GU |   |     |     |     |\n"
    "| U ||     |    | # |  #  | *GU |  #  |\n"
    "| G || (E) | i  |   |     |     |     |\n"
    "+-------------------------------------+\n"
    ;

  // Parse table shenanigans
  char* valin0[] = {"E$", "E$"};
  char* valin1[] = {"TF", "TF"};
  char* valin2[] = {EPSILON_str, "+TF", EPSILON_str};
  char* valin3[] = {"GU", "GU"};
  char* valin4[] = {EPSILON_str, EPSILON_str, "*GU", EPSILON_str};
  char* valin5[] = {"(E)", "i"};
  char keyin00[] = {'(', 'i'};
  char keyin01[] = {'(', 'i'};
  char keyin02[] = {'$', '+', ')'};
  char keyin03[] = {'(', 'i'};
  char keyin04[] = {'$', '+', '*', ')'};
  char keyin05[] = {'(', 'i'};
  HTABLE* hin[] = {
    htblcreate(keyin00, STR_t, NULL, valin0, 2),
    htblcreate(keyin01, STR_t, NULL, valin1, 2),
    htblcreate(keyin02, STR_t, NULL, valin2, 3),
    htblcreate(keyin03, STR_t, NULL, valin3, 2),
    htblcreate(keyin04, STR_t, NULL, valin4, 4),
    htblcreate(keyin05, STR_t, NULL, valin5, 2)
  };
  char keyin1[] = {'S', 'E', 'F', 'T', 'U', 'G'};
  HTABLE* table = htblcreate(keyin1, HTABLE_t, hin, NULL, 6);
  // parse table hell ends

  // Some user-friendlyness
  printf("Parse Table\n");
  printf("===========\n");

  printf("%s\n", parse_table);

  printf("Input String: \"i+i\"\n\n");

  // Result
  printf("Result: ");
  if (ll1("i+i", 'S', table))
    printf("Accepted!\n");
  else
    printf("Rejected!\n");
  
  return 0;
}
