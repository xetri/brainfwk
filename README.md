# Brainfwk Compiler

## It can compile brainfuck code to C and be compiled to executable with gcc. Runs in its own Virtual Machine.

---

## Build
```console
gcc ./bfwk.c -o bfwk.exe
#or
tcc ./bfwk.c -o bfwk.exe
```

## Interpret
```console
./bfwk run ./b.b
```

## Compile
```console
./bfwk com ./b.b ./out
./out
```

## Transpile to C
```console
./bfwk c ./b.b
```

## Specify stack size
```console
./bfwk run ./b.b 255
```

