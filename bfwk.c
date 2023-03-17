#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CELL 32768

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
  if (lexer->cur == '\t') lexer->col += 3;
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

void checker(char* file, Token_t* tokens, unsigned long long int len) {
    unsigned long long int c = 0;
    for (; c < len; c++) {
        if (tokens[c].kind == OPEN) {
            Token_t open = tokens[c];
            while(tokens[c].kind != CLOSE) {
                if (c >= len) {
                    printf("%s:%d:%d: Unclosed `[`\n", file, open.pos.line, open.pos.col);
                    exit(-2);
                }
                c++;
            }
        }
    }
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
        case INC : ++vm->stack[vm->sp]; break;
        case DEC : --vm->stack[vm->sp]; break;
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
                while(depth > 0 || tk[vm->ip].kind == CLOSE) {
                    vm->ip++;
                    if (tk[vm->ip].kind == OPEN) depth++;
                    else if (tk[vm->ip].kind == CLOSE) depth--;
                }
            } break;
        }
        case CLOSE : {
            if (vm->stack[vm->sp] != 0) {
                unsigned int depth = 1;
                while(depth > 0 || tk[vm->ip].kind == OPEN) {
                    vm->ip--;
                    if (tk[vm->ip].kind == OPEN) depth--;
                    else if (tk[vm->ip].kind == CLOSE) depth++;
                }
            } break;
        }
        case DOT: { printf("%c", vm->stack[vm->sp]); break; }
        case COMMA : { scanf("%c", (char*)&vm->stack[vm->sp]); break; }
        default: {};
    }
        vm->ip++;
}

char *filereader(char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fclose(file);
        printf("%s%s\n", "Unable to read from: ", path);
        exit(1);
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
        fclose(file);
        exit(1);
    }
    fprintf(file, "%s", content);
    fclose(file);
}

const char* cc1 =
"#include <stdio.h>\n"
"#define CELL ";

const char* cc2 = 
"\ntypedef enum { INC, DEC, SR, SL, OPEN, CLOSE, DOT, COMMA } INST;\n"
"typedef struct {\n"
"    unsigned long long int ss;\n"
"    unsigned long long int ip;\n"
"    unsigned long long int sp;\n"
"    int stack[CELL];\n"
"} VM;\n"
"VM new_vm(unsigned long long int ss) {\n"
"    VM vm = {0};\n"
"    vm.ss = ss;\n"
"    vm.ip = 0;\n"
"    vm.sp = 0;\n"
"    return vm;\n"
"}\n"
"void vm_execute(VM* vm, INST insts[], unsigned long long int len) {\n"
"    while (vm->ip < len) {\n"
"    INST inst = insts[vm->ip];\n"
"    switch (inst) {\n"
"        case INC : ++vm->stack[vm->sp]; break;\n"
"        case DEC : --vm->stack[vm->sp]; break;\n"
"        case SR : {\n"
"            if (vm->sp == vm->ss - 1) vm->sp = 0;\n"
"            else vm->sp++;\n"
"        } break;\n"
"        case SL : {\n"
"            if (vm->sp == 0) vm->sp = vm->ss - 1;\n"
"            else vm->sp--;\n"
"        } break;\n"
"        case OPEN : {\n"
"            if (vm->stack[vm->sp] == 0) {\n"
"                unsigned int depth = 1;\n"
"                while(depth > 0 || insts[vm->ip] == CLOSE) {\n"
"                    vm->ip++;\n"
"                    if (insts[vm->ip] == OPEN) depth++;\n"
"                    else if (insts[vm->ip] == CLOSE) depth--;\n"
"                }\n"
"            } break;\n"
"        }\n"
"        case CLOSE : {\n"
"            if (vm->stack[vm->sp] != 0) {\n"
"                unsigned int depth = 1;\n"
"                while(depth > 0 || insts[vm->ip] == OPEN) {\n"
"                    vm->ip--;\n"
"                    if (insts[vm->ip] == OPEN) depth--;\n"
"                    else if (insts[vm->ip] == CLOSE) depth++;\n"
"                }\n"
"            } break;\n"
"        }\n"
"        case DOT: printf(\"%c\", vm->stack[vm->sp]); break;\n"
"        case COMMA : scanf(\"%c\", (char*)&vm->stack[vm->sp]); break;\n"
"        default: {};\n"
"    }   vm->ip++; }\n"
"}\n"
"INST insts[] = {";

const char* cc3 = 
"};\n"
"int main(int _, char** __) {\n"
"    VM vm = new_vm(CELL);\n"
"    vm_execute(&vm, insts, sizeof(insts)/sizeof(insts[0]));\n"
"    return 0;\n"
"}";


char* tokens_as_arrstr(Token_t* tk, unsigned long long int len) {
    char* str = calloc(1, sizeof(char) * (len * 6));

    unsigned long long int c = 0;
    for (; c < len; c++) {
        strcat(str, inst_as_str(tk[c].kind));
        if (c < len - 1) strcat(str, ",");
    }

    char* clone = strdup(str);
    free(str);
    return clone;
}

char* compiled_code(unsigned long long int cell, char* arr) {
    char* code = calloc(1, sizeof(char) * (strlen(cc1) + strlen(cc2) + strlen(cc3) + strlen(arr) + 32));

    char buf[32];

    strcat(code, cc1);
    sprintf(buf, "%lld", cell);
    strcat(code, buf);
    strcat(code , cc2);
    strcat(code, arr);
    strcat(code, cc3);

    char* clone = strdup(code);
    free(code);
    return clone;
}

int usage() {
    printf("Brainfwk Compiler\n\n");
    printf("    run  <file>         <stacksize>     Interpret\n");
    printf("    com  <file>  <out>  <stacksize>     Compile with gcc\n");
    printf("    c    <file>  <out>  <stacksize>     Transpile to C code\n");
    return -1;
};

void run(char* file, unsigned long long int cell) {
    char* source = filereader(file);
    Lexer_t lexer = {
        .file = file,
        .source = source,
        .pos = 0,
        .line = 1,
        .cur = source[0],
        .col = 1,
        .tokens = malloc(sizeof(Token_t) * ( strlen(source) + 1)),
        .ntok = 0,
    };
    tokenize(&lexer);
    checker(file, lexer.tokens, lexer.ntok);

    VM vm = new_vm(cell);
    while(vm.ip < lexer.ntok) vm_execute(&vm, lexer.tokens);

    if (lexer.tokens) free(lexer.tokens);
    if (vm.stack) free(vm.stack);
}

char* compile(char* file, unsigned long long int cell) {
    char* source = filereader(file);
    Lexer_t lexer = {
        .file = file,
        .source = source,
        .pos = 0,
        .line = 1,
        .cur = source[0],
        .col = 1,
        .tokens = malloc(sizeof(Token_t) * ( strlen(source) + 1)),
        .ntok = 0,
    };
    tokenize(&lexer);
    checker(file, lexer.tokens, lexer.ntok);

    char* code = compiled_code(cell, tokens_as_arrstr(lexer.tokens, lexer.ntok));

    if (lexer.tokens) free(lexer.tokens);
    return code;
}


int main(int _, char** argv) {
    if (_ < 2) return usage();

    char* out = "b";
    unsigned long long int cell = CELL;

    if (strcmp(argv[1], "help") == 0) return (usage() + 1);
    else if (strcmp(argv[1], "run") == 0) {
        if (_ < 3) {
            usage();
            printf("\n    No file specified to run\n");
            return 3;
        }
        if (_ > 3) cell = atoll(argv[3]);
        run(argv[2], cell);
    }
    else if (strcmp(argv[1], "com") == 0) {
        if (_ < 3) {
            usage();
            printf("\n    No file specified to compile\n");
            return 3;
        }
        char* file = argv[2];
        char* ec = "0xffffff-69420.c";
        char* out = "b.exe";
        if (_ > 4) cell = atoll(argv[4]);
        if (_ > 3) out = argv[3];
        char* sc = compile(file, cell);
        filewriter(ec, sc);
        char cmd[500] = "gcc ";
        strcat(cmd, ec);
        strcat(cmd, " -o ");
        strcat(cmd, out);
        system(cmd);
        remove(ec);
    }
    else if (strcmp(argv[1], "c") == 0) {
        if (_ < 3) {
            usage();
            printf("\n    No file specified to transpile\n");
            return 3;
        }
        out = "b.c";
        if (_ > 3) out = argv[3];
        if (_ > 4) cell = (unsigned long long int)atoll(argv[4]);
        char* sc = compile(argv[2], cell);
        filewriter(out, sc);
    }
    else {
       usage();
       printf("\n    Unregistered command `%s`\n", argv[1]);
       return 3;
    };

    return 0;
}
