#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#define SYMTBL_SZ 1000

#define KWLIST_SZ 9

#define MAXTOKEN 16

int lblcount; /* indica o rótulo atual */

/* tabela de símbolos */

char *symtbl[SYMTBL_SZ];

char symtype[SYMTBL_SZ];

int nsym; /* número de entradas atuais na tabela de símbolos */

/* códigos e lista de palavras-chave */

char kwcode[KWLIST_SZ+1] = "ileweRWve";

char *kwlist[KWLIST_SZ] = {"IF", "ELSE", "ENDIF", "WHILE", "ENDWHILE",

                                                  "READ", "WRITE", "VAR", "END"};

char look; /* O caracter lido "antecipadamente" (lookahead) */

char token; /* código do token atual */

char value[MAXTOKEN+1]; /* texto do token atual */

/* PROTÓTIPOS */

/* rotinas utilitárias */

void init();

void nextchar();

void error(char *s);

void fatal(char *s);

void expected(char *s);

void undefined(char *name);

void duplicated(char *name);

void checkident();

/* reconhecedores */

int isaddop(char c);

int ismulop(char c);

int isorop(char c);

int isrelop(char c);

/* tratamento de símbolos */

int lookup(char *s, char *list[], int size);

int locate(char *name);

int intable(char *name);

void checktable(char *name);

void addsymbol(char *name, char type);

/* analisador léxico */

void skipwhite();

void getname();

void getnum();

void getop();

void nexttoken();

void scan();

void matchstring(char *s);

/* rótulos */

int newlabel();

/* rotinas de geração de código */

void asm_clear();

void asm_negative();

void asm_loadconst(char *name);

void asm_loadvar(char *name);

void asm_push();

void asm_popadd();

void asm_popsub();

void asm_popmul();

void asm_popdiv();

void asm_store(char *name);

void asm_not();

void asm_popand();

void asm_popor();

void asm_popxor();

void asm_popcompare();

void asm_relop(char op);

void asm_jmp(int label);

void asm_jmpfalse(int label);

void asm_read();

void asm_write();

void header();

void prolog();

void epilog();

void allocvar(char *name, int value);

/* rotinas do analisador sintático */

/* expressões aritméticas */

void factor();

void multiply();

void divide();

void term();

void add();

void subtract();

void expression();

/* expressões booleanas e relações */

void relation();

void notfactor();

void boolterm();

void boolor();

void boolxor();

void boolexpression();

/* bloco, estruturas de controle e comandos */

void assignment();

void doif();

void dowhile();

void readvar();

void doread();

void writevar();

void dowrite();

void block();

/* declarações */

void decl();

void topdecls();

/* PROGRAMA PRINCIPAL */

int main()

{

                init();

                matchstring("PROGRAM");

                header();

                topdecls();

                matchstring("BEGIN");

                prolog();

                block();

                matchstring("END");

                epilog();

                return 0;

}

/* inicialização do compilador */

void init()

{

                nsym = 0;

                nextchar();

                nexttoken();

}

/* lê próximo caracter da entrada em lookahead */

void nextchar()

{

                look = getchar();

}

/* imprime mensagem de erro sem sair */

void error(char *s)

{

                fprintf(stderr, "Error: %s\n", s);

}

/* imprime mensagem de erro e sai */

void fatal(char *s)

{

                error(s);

                exit(1);

}

/* mensagem de erro para string esperada */

void expected(char *s)

{

                fprintf(stderr, "Error: \'%s\' expected\n", s);

                exit(1);

}

/* avisa a respeito de um identificador desconhecido */

void undefined(char *name)

{

                fprintf(stderr, "Error: Undefined identifier \'%s\'\n", name);

                exit(1);

}

/* avisa a respeito de um identificador desconhecido */

void duplicated(char *name)

{

                fprintf(stderr, "Error: Duplicated identifier \'%s\'\n", name);

                exit(1);

}

/* reporta um erro se token NÃO é identificador */

void checkident()

{

        if (token != 'x')

                expected("Identifier");

}

/* testa operadores de adição */

int isaddop(char c)

{

                return (c == '+' || c == '-');

}

/* testa operadores de multiplicação */

int ismulop(char c)

{

                return (c == '*' || c == '/');

}

/* testa operadores OU */

int isorop(char c)

{

                return (c == '|' || c == '~');

}

/* testa operadores relacionais */

int isrelop(char c)

{

                return (strchr("=#<>", c) != NULL);

}

/* pula caracteres em branco */

void skipwhite()

{

                while (isspace(look))

                               nextchar();

}

/* procura por string em tabela,

   usado para procurar palavras-chave e símbolos */

int lookup(char *s, char *list[], int size)

{

                int i;

                for (i = 0; i < size; i++) {

                               if (strcmp(list[i], s) == 0)

                                               return i;

                }

                return -1;

}

/* retorna o endereço do identificador na tabela de símbolos */

int locate(char *name)

{

        return lookup(name, symtbl, nsym);

}

/* verifica se "name" consta na tabela de símbolos */

int intable(char *name)

{

                if (lookup(name, symtbl, nsym) < 0)

                               return 0;

                return 1;

}

/* reporta um erro se identificador NÃO constar na tabela de símbolos */

void checktable(char *name)

{

        if (!intable(name))

                undefined(name);

}

/* reporta um erro se identificador JÁ constar na tabela de símbolos */

void checkdup(char *name)

{

        if (intable(name))

                duplicated(name);

}

/* adiciona novo identificador à tabela de símbolos */

void addsymbol(char *name, char type)

{

                char *newsym;

        checkdup(name);

                if (nsym >= SYMTBL_SZ) {

                               fatal("Symbol table full!");

                }

                newsym = (char *) malloc(sizeof(char) * (strlen(name) + 1));

                if (newsym == NULL)

                               fatal("Out of memory!");

                strcpy(newsym, name);

                symtbl[nsym] = newsym;

        symtype[nsym] = type;

        nsym++;

}

/* analisa e traduz um nome (identificador ou palavra-chave) */

void getname()

{

                int i;

                skipwhite();

                if (!isalpha(look))

                               expected("Identifier");

                for (i = 0; isalnum(look) && i < MAXTOKEN; i++) {

                               value[i] = toupper(look);

                               nextchar();

                }

                value[i] = '\0';

                token = 'x';

}

/* analisa e traduz um número inteiro */

void getnum()

{

                int i;

                skipwhite();

                if (!isdigit(look))

                               expected("Number");

                for (i = 0; isdigit(look) && i < MAXTOKEN; i++) {

                               value[i] = look;

                               nextchar();

                }

                value[i] = '\0';

                token = '#';

}

/* analisa e traduz um operador */

void getop()

{

                skipwhite();

                token = look;

                value[0] = look;

                value[1] = '\0';

                nextchar();

}

/* pega o próximo token de entrada */

void nexttoken()

{

                skipwhite();

                if (isalpha(look))

                               getname();

                else if (isdigit(look))

                               getnum();

                else

                               getop();

}

/* analisador léxico. analisa identificador ou palavra-chave */

void scan()

{

                int kw;

        if (token == 'x') {

                kw = lookup(value, kwlist, KWLIST_SZ);

                if (kw >= 0)

                               token = kwcode[kw];

        }

}

/* compara string com texto do token atual */

void matchstring(char *s)

{

                if (strcmp(value, s) != 0)

                               expected(s);

        nexttoken();

}

/* gera um novo rótulo */

int newlabel()

{

                return lblcount++;

}

/* ROTINAS DE GERAÇÃO DE CÓDIGO */

/* zera o registrador primário */

void asm_clear()

{

                printf("\txor ax, ax\n");

}

/* negativa o reg. primário */

void asm_negative()

{

                printf("\tneg ax\n");

}

/* carrega uma constante numérica no reg. prim. */

void asm_loadconst(char *val)

{

                printf("\tmov ax, %s\n", val);

}

/* carrega uma variável no reg. prim. */

void asm_loadvar(char *name)

{

                if (!intable(name))

                               undefined(name);

                printf("\tmov ax, word ptr %s\n", name);

}

/* coloca reg. prim. na pilha */

void asm_push()

{

                printf("\tpush ax\n");

}

/* adiciona o topo da pilha ao reg. prim. */

void asm_popadd()

{

                printf("\tpop bx\n");

                printf("\tadd ax, bx\n");

}

/* subtrai o reg. prim. do topo da pilha */

void asm_popsub()

{

                printf("\tpop bx\n");

                printf("\tsub ax, bx\n");

                printf("\tneg ax\n");

}

/* multiplica o topo da pilha pelo reg. prim. */

void asm_popmul()

{

                printf("\tpop bx\n");

                printf("\tmul bx\n");

}

/* divide o topo da pilha pelo reg. prim. */

void asm_popdiv()

{

                printf("\tpop bx\n");

                printf("\txchg ax, bx\n");

                printf("\tcwd\n");

                printf("\tdiv bx\n");

}

/* armazena reg. prim. em variável */

void asm_store(char *name)

{

                printf("\tmov word ptr %s, ax\n", name);

}

/* inverte reg. prim. */

void asm_not()

{

                printf("\tnot ax\n");

}

/* "E" do topo da pilha com reg. prim. */

void asm_popand()

{

                printf("\tpop bx\n");

                printf("\tand ax, bx\n");

}

/* "OU" do topo da pilha com reg. prim. */

void asm_popor()

{

                printf("\tpop bx\n");

                printf("\tor ax, bx\n");

}

/* "OU-exclusivo" do topo da pilha com reg. prim. */

void asm_popxor()

{

                printf("\tpop bx\n");

                printf("\txor ax, bx\n");

}

/* compara topo da pilha com reg. prim. */

void asm_popcompare()

{

                printf("\tpop bx\n");

                printf("\tcmp bx, ax\n");

}

/* altera reg. primário (e flags, indiretamente) conforme a comparação */

void asm_relop(char op)

{

                char *jump;

                int l1, l2;

                l1 = newlabel();

                l2 = newlabel();

                switch (op) {

                  case '=': jump = "je"; break;

                  case '#': jump = "jne"; break;

                  case '<': jump = "jl"; break;

                  case '>': jump = "jg"; break;

                  case 'L': jump = "jle"; break;

                  case 'G': jump = "jge"; break;

                }

                printf("\t%s L%d\n", jump, l1);

                printf("\txor ax, ax\n");

                printf("\tjmp L%d\n", l2);

                printf("L%d:\n", l1);

                printf("\tmov ax, -1\n");

                printf("L%d:\n", l2);

}

/* desvio incondicional */

void asm_jmp(int label)

{

                printf("\tjmp L%d\n", label);

}

/* desvio se falso (0) */

void asm_jmpfalse(int label)

{

                printf("\tjz L%d\n", label);

}

/* lê um valor para o registrador primário e armazena em variável */

void asm_read()

{

                printf("\tcall READ\n");

                asm_store(value);

}

/* mostra valor do reg. primário */

void asm_write()

{

                printf("\tcall WRITE\n");

}

/* cabeçalho do código assembly */

void header()

{

                printf(".model small\n");

                printf(".stack\n");

                printf(".code\n");

                printf("extrn READ:near, WRITE:near\n");

                printf("PROG segment byte public\n");

                printf("\tassume cs:PROG,ds:PROG,es:PROG,ss:PROG\n");

}

/* prólogo da rotina principal */

void prolog()

{

                printf("MAIN:\n");

                printf("\tmov ax, PROG\n");

                printf("\tmov ds, ax\n");

                printf("\tmov es, ax\n");

}

/* epílogo da rotina principal */

void epilog()

{

                printf("\tmov ax,4C00h\n");

                printf("\tint 21h\n");

                printf("PROG ends\n");

                printf("\tend MAIN\n");

}

/* aloca memória para uma declaração de variável (+inicializador) */

void allocvar(char *name, int value)

{

                printf("%s:\tdw %d\n", name, value);

}

/* analisa e traduz um fator matemático */

void factor()

{

                if (token == '(') {

                nexttoken();

                               boolexpression();

                               matchstring(")");

                } else {

                if (token == 'x')

                               asm_loadvar(value);

                else if (token == '#')

                               asm_loadconst(value);

                else

                               expected("Math Factor");

                nexttoken();

        }

}

/* reconhece e traduz uma multiplicação */

void multiply()

{

        nexttoken();

                factor();

                asm_popmul();

}

/* reconhece e traduz uma divisão */

void divide()

{

        nexttoken();

                factor();

                asm_popdiv();

}

/* analisa e traduz um termo matemático */

void term()

{

        factor();

                while (ismulop(token))  {

                               asm_push();

                               switch (token) {

                                 case '*':

                                               multiply();

                                               break;

                                 case '/':

                                               divide();

                                               break;

                               }

                }

}

/* reconhece e traduz uma adição */

void add()

{

        nexttoken();

                term();

                asm_popadd();

}

/* reconhece e traduz uma subtração*/

void subtract()

{

        nexttoken();

                term();

                asm_popsub();

}

/* analisa e traduz uma expressão matemática */

void expression()

{

        if (isaddop(token))

                asm_clear();

        else

                term();

                while (isaddop(token))  {

                               asm_push();

                               switch (token) {

                                 case '+':

                                               add();

                                               break;

                                 case '-':

                                               subtract();

                                               break;

                               }

                }

}

/* analisa e traduz uma relação */

void relation()

{

                char op;

                expression();

                if (isrelop(token)) {

                               op = token;

                               nexttoken(); /* só para remover o operador do caminho */

                               if (op == '<') {

                                               if (token == '>') { /* <> */

                                                               nexttoken();

                                                               op = '#';

                                               } else if (token == '=') {

                                                               nexttoken();

                                                               op = 'L';

                                               }

                               } else if (op == '>' && token == '=') {

                                               nexttoken();

                                               op = 'G';

                               }

                               asm_push();

                expression();

                asm_popcompare();

                               asm_relop(op);

                }

}

/* analisa e traduz um fator booleano com NOT inicial */

void notfactor()

{

                if (token == '!') {

                               nexttoken();

                               relation();

                               asm_not();

                } else

                               relation();

}

/* analisa e traduz um termo booleano */

void boolterm()

{

                notfactor();

                while (token == '&') {

                               asm_push();

                               nexttoken();

                               notfactor();

                               asm_popand();

                }

}

/* reconhece e traduz um "OR" */

void boolor()

{

                nexttoken();

                boolterm();

                asm_popor();

}

/* reconhece e traduz um "xor" */

void boolxor()

{

                nexttoken();

                boolterm();

                asm_popxor();

}

/* analisa e traduz uma expressão booleana */

void boolexpression()

{

                boolterm();

                while (isorop(token)) {

                               asm_push();

                               switch (token) {

                                 case '|':

                                               boolor();

                                               break;

                                 case '~':

                                               boolxor();

                                               break;

                               }

                }

}

/* analisa e traduz um comando de atribuição */

void assignment()

{

                char name[MAXTOKEN+1];

                strcpy(name, value);

        checktable(name);

                nexttoken();

                matchstring("=");

                boolexpression();

                asm_store(name);

}

/* analiza e traduz um comando IF-ELSE-ENDIF */

void doif()

{

                int l1, l2;

                nexttoken();

                boolexpression();

                l1 = newlabel();

                l2 = l1;

                asm_jmpfalse(l1);

                block();

                if (token == 'l') {

                               nexttoken();

                               l2 = newlabel();

                               asm_jmp(l2);

                               printf("L%d:\n", l1);

                               block();

                }

                printf("L%d:\n", l2);

                matchstring("ENDIF");

}

/* analiza e traduz um comando WHILE-ENDWHILE */

void dowhile()

{

                int l1, l2;

                nexttoken();

                l1 = newlabel();

                l2 = newlabel();

                printf("L%d:\n", l1);

                boolexpression();

                asm_jmpfalse(l2);

                block();

                matchstring("ENDWHILE");

                asm_jmp(l1);

                printf("L%d:\n", l2);

}

/* lê uma variável única */

void readvar()

{

        checkident();

        checktable(value);

        asm_read(value);

        nexttoken();

}

/* analiza e traduz um comando READ */

void doread()

{

        nexttoken();

                matchstring("(");

                for (;;) {

                readvar();

                               if (token != ',')

                                               break;

                nexttoken();

                }

                matchstring(")");

}

/* analiza e traduz um comando WRITE */

void dowrite()

{

                nexttoken();

                matchstring("(");

                for (;;) {

                               expression();

                               asm_write();

                               if (token != ',')

                                               break;

                               nexttoken();

                }

                matchstring(")");

}

/* analiza e traduz um bloco de comandos */

void block()

{

                int follow = 0;

                do {

                               scan();

                               switch (token) {

                                 case 'i':

                                               doif();

                                               break;

                                 case 'w':

                                               dowhile();

                                               break;

                                 case 'R':

                                               doread();

                                               break;

                                 case 'W':

                                               dowrite();

                                               break;

                                 case 'e':

                                 case 'l':

                                               follow = 1;

                                               break;

                                 default:

                                               assignment();

                                               break;

                               }

                } while (!follow);

}

/* declaração de variáveis */

void decl()

{

        nexttoken();

        if (token != 'x')

                expected("Variable name");

        checkdup(value);

        addsymbol(value, 'v');

        allocvar(value, 0);

        nexttoken();

}

/* analisa e traduz declarações */

void topdecls()

{

                scan();

                while (token == 'v') {

                do {

                        decl();

                } while (token == ',');

                }

}
