#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *filereader(char *path) {
  FILE *file = fopen(path, "r");
  if (!file) {
    fclose(file);
    printf("%s%s\n", "Unable to read from: ", path);
    exit(-3);
  }
  fseek(file, 0, SEEK_END);
  long long file_size = ftell(file);
  rewind(file);
  char *buffer = (char *)malloc(file_size);
  long long bytes_read = fread((char *)buffer, 1, file_size, file);

  fclose(file);
  buffer[bytes_read] = '\0';
  char *content = strdup(buffer);
  free(buffer);

  return content;
}

void filewriter(char *path, char *content) {
  FILE *file = fopen(path, "w");
  if (!file) {
      printf("%s%s\n", "Unable to write to: ", path);
      return;
  }
  fprintf(file, "%s", content);
  fclose(file);
}

typedef enum { INC, DEC, SR, SL, OPEN, CLOSE, DOT, COMMA } INST;
typedef struct { unsigned int line; unsigned int col; } Node_position;
typedef struct { INST kind; Node_position pos; } Token_t;
typedef struct {
    char* file;
    char* source;
    char cur;
    unsigned long int pos;
    unsigned long int col;
    unsigned long int line;
    unsigned long long int ntok;
    Token_t *tokens;
} Lexer_t;

char lexer_next(Lexer_t *lexer) {
  if (lexer->cur == '\0') return lexer->cur;
  if (lexer->cur == '\n') { lexer->line++; lexer->col = 0; }
  lexer->pos++;
  lexer->col++;
  lexer->cur = lexer->source[lexer->pos];
  return lexer->cur;
}

void tokenize(Lexer_t *lexer) {
    char lexing = 1;
    while (lexing) {
        switch (lexer->cur) {
            case '\0': { lexing = 0; break; }
            case '+' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = INC 
                };
                break;
            }
            case '-' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = DEC
                };
                break;
            }
            case '>' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = SR
                };
                break;
            }
            case '<' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = SL
                };
                break;
            }
            case '[' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = OPEN
                };
                break;
            }
            case ']' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = CLOSE
                };
                break;
            }
            case '.' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = DOT
                };
                break;
            }
            case ',' : {
                lexer->tokens[lexer->ntok++] = (Token_t){
                    .pos = (Node_position){ .col = lexer->col, .line = lexer->line },
                    .kind = COMMA 
                };
                break;
            }
            default: {};
        }
        lexer_next(lexer);
    }
}

char* inst_as_str(INST inst) {
    switch (inst) {
        case INC: return "INC";
        case DEC: return "DEC";
        case SR: return "SR";
        case SL: return "SL";
        case OPEN: return "OPEN";
        case CLOSE: return "CLOSE";
        case DOT: return "DOT";
        case COMMA: return "COMMA";
    }   return 0;
}

int checker(char* file, Token_t* tokens, unsigned long long int len) {
    unsigned long long int c = 0;
    for (; c < len; c++) {
        if (tokens[c].kind == OPEN) {
            Token_t open = tokens[c];
            while(tokens[c].kind != CLOSE) {
                if (c >= len) {
                    printf("%s:%d:%d: Unclosed `[`\n", file, open.pos.line, open.pos.col);
                    return 0;
                }
                c++;
            }
        }
    }
    return 1;
}

typedef struct {
  unsigned long long int ss;
  unsigned long long int ip;
  unsigned long long int sp;
  int *stack;
} VM;

VM new_vm(unsigned long long int ss) {
    VM vm = {0};
    vm.ss = ss;
    vm.ip = 0;
    vm.sp = 0;
    vm.stack = calloc(1, vm.ss * sizeof(int));
    return vm;
}

void vm_execute(VM* vm, Token_t *tk) {
    INST inst = tk[vm->ip].kind;
    switch (inst) {
        case INC : { ++vm->stack[vm->sp]; break; }
        case DEC : { --vm->stack[vm->sp]; break; }
        case SR : {
            if (vm->sp == vm->ss - 1) vm->sp = 0;
            else vm->sp++;
            break;
        }
        case SL : {
            if (vm->sp == 0) vm->sp = vm->ss - 1;
            else vm->sp--;
            break;
        }
        case OPEN : {
            if (vm->stack[vm->sp] == 0) {
                unsigned int depth = 1;
                while(depth > 0) {
                    INST inst = tk[++vm->ip].kind;
                    if (inst == OPEN) depth++;
                    else if (inst == CLOSE) depth--;
                }
            }
            break;
        }
        case CLOSE : {
            if (vm->stack[vm->sp] != 0) {
                int depth = 1;
                while (depth > 0) {
                    INST inst = tk[--vm->ip].kind;
                    if (inst == CLOSE) depth++;
                    else if (inst == OPEN) depth--;
                }
            }
            break;
        }
        case DOT: { printf("%c", vm->stack[vm->sp]); break; }
        case COMMA : { scanf("%c", (char*)&vm->stack[vm->sp]); break; }
        default: {};
    }   vm->ip++;
}

int main(int _, char** argv) {
    if (_ < 2) return -1;

    char* file = argv[1];
    char* source = filereader(file);
    unsigned long long int len = 39390;

    Lexer_t lexer = { .file = file, .source = source, .pos = 0, .line = 1, .cur = source[0], .col = 1, .tokens = malloc(sizeof(Token_t) * ( len + 1)), .ntok = 0, };
    tokenize(&lexer);
    if (!checker(file, lexer.tokens, lexer.ntok)) goto dis;

    VM vm = new_vm(len);
    while(vm.ip < lexer.ntok) vm_execute(&vm, lexer.tokens);

dis:
    if (lexer.tokens) free(lexer.tokens);
    if (vm.stack) free(vm.stack);
    return 0;
}
