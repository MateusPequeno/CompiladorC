#include <stdio.h>
#include <string.h>
#include "cradle.h"
#define MaxEntry 26
const char TAB = '\t';
const char CR = '\n';
const char LF = '\r';

char tmp[MAX_BUF];  /*buffer temporario */

char Look;
char ST[MaxEntry];   /* tabelas de simbolos */

/* lendo novo caractere da entrada */
void GetChar()
{
    Look = getchar();
}

/* Reporta um Erro */
void Error(char *str)
{

    printf("\n");
    printf("\a Foi detectado um Erro: %s.\n", str);
}

/* reporta um Erro e para */
void Abort(char *str)
{
    Error(str);
    exit(1);
}

/* reporta o que é esperado */
void Expected(char *str)
{
    sprintf(tmp, "Se espera o seguinte tipo de entrada: %s", str);
    Abort(tmp);
}

/* reporta um identificador indefinido */
void Undefined(char name)
{
    sprintf(tmp, "Identificador Indefinido: %c", name);
    Abort(tmp);
}

/* reporta uma duplicata do identificador  */
void Duplicate(char name)
{
    sprintf(tmp, "Duplicata do identificador: %c", name);
    Abort(tmp);
}

/* Recebe tipo do simbolo */
char TypeOf(char symbol)
{
    return ST[symbol - 'A'];
}

/* checa se o simbolo está na tabela */
bool InTable(char symbol)
{
    return ST[symbol - 'A'] != '?';
}

/* adiciona um novo simbolo a tabela */
void AddEntry(char symbol, char type)
{
    CheckDup(symbol);
    ST[symbol-'A'] = type;
}

/* checa uma entrada para garantir que é uma variavel  */
void CheckVar(char name)
{
    char tmp_buf[MAX_BUF];
    if (!InTable(name)) {
        Undefined(name);
    }
    if (TypeOf(name) != 'v') {
        sprintf(tmp_buf, "%c nao eh uma variavel", name);
        Abort(tmp_buf);
    }
}

/* checa por duplicatas nos nomes das variaveis */
void CheckDup(char name)
{
    if (InTable(name)) {
        Duplicate(name);
    }
}

/* torna um caracter em caixa alta */
char upcase(char c)
{
    return (c & 0xDF);
}

bool IsAlpha(char c)
{
    char upper = upcase(c);
    return (upper >= 'A') && (upper <= 'Z');
}

bool IsDigit(char c)
{
    return (c >= '0') && (c <= '9');
}

bool IsAlNum(char c)
{
    return IsAlpha(c) || IsDigit(c);
}

bool IsAddop(char c)
{
    return strchr("+-", c) != NULL;
}

bool IsMulop(char c)
{
    return strchr("*/", c) != NULL;
}

bool IsRelop(char c)
{
    return strchr("=#<>", c) != NULL;
}

bool IsWhite(char c)
{
    return strchr(" \t", c) != NULL;
}

bool IsVarType(char c)
{
    return strchr("BWLbwl", c) != NULL;
}

/* recebe um tipo de variavel da tabela de simbolos */
char VarType(char name)
{
    char type = TypeOf(name);
    if (!IsVarType(type)) {
        sprintf(tmp, "Identificador %c não é uma variavel", name);
    }
    return type;
}

/*  pulando um espaço em branco */
void SkipWhite(void)
{
    while(IsWhite(Look)) {
        GetChar();
    }
}

/* pula o fim da linha */
void Fin(void)
{
    if (Look == CR) {
        GetChar();
        if (Look == LF) {
            GetChar();
        }
    } else if (Look == LF){
        GetChar();
    }
}

/* combina um tipo especifico de caractere de entrada */
void Match(char c)
{
    if (Look == c) {
        GetChar();
    } else {
        char tmp_buf[MAX_BUF];
        sprintf(tmp_buf, "'%c'", c);
        Expected(tmp_buf);
    }
    SkipWhite();
}

/*  recebe um identificador */
char GetName(void)
{
    if (! IsAlpha(Look)) {
        Expected("Name");
    }
    char name = upcase(Look);
    GetChar();
    SkipWhite();
    return name;
}

/* recebe um número */
int GetNum(void)
{
    if (!IsDigit(Look)) {
        Expected("Integer");
    }
    int val = 0;
    while(IsDigit(Look)) {
        val = 10*val + Look - '0';
        GetChar();
    }
    SkipWhite();
    return val;
}

/* carrega uma constranto ao registrador primário*/
char LoadNum(int val)
{
    char type;
    if (abs(val) <= 127) {
        type = 'B';
    } else if (abs(val) <= 32767) {
        type = 'W';
    } else {
        type = 'L';
    }
    LoadConst(val, type);
    return type;
}

/* imprime uma string com TAB */
void Emit(char *str)
{
    printf("\t%s", str);
}

/* imprime uma string com tab e CRLF */
void EmitLn(char *str)
{
    Emit(str);
    printf("\n");
}

/* Poe um "etiqueta" na saída */
void PostLabel(char *label)
{
    printf("%s:\n", label);
}

/* carrega uma variável no registrador primário */
void LoadVar(char name, char type)
{
    char src[MAX_BUF];
    src[0] = name;
    src[1] = '\0';
    char *dst;
    switch(type) {
        case 'B':
            dst = "%al";
            break;
        case 'W':
            dst = "%ax";
            break;
        case 'L':
            dst = "%eax";
            break;
        default:
            dst = "%eax";
            break;
    }
    Move(type, src, dst);
}

void Move(char size, char *src, char *dest)
{
    sprintf(tmp, "MOV%c %s, %s", size, src, dest);
    EmitLn(tmp);
}

/* guarda o registrador primario */
void StoreVar(char name, char type)
{
    char dest[MAX_BUF];
    dest[0] = name;
    dest[1] = '\0';
    char *src;
    switch(type) {
        case 'B':
            src = "%al";
            break;
        case 'W':
            src = "%ax";
            break;
        case 'L':
            src = "%eax";
            break;
        default:
            src = "%eax";
            break;
    }
    Move(type, src, dest);
}

/* carrega a variavel no registrador primario */
char Load(char name)
{
    char type = VarType(name);
    LoadVar(name, type);
    return type;
}

/* carrega a constante no registrador primario*/
void LoadConst(int val, char type)
{
    char src[MAX_BUF];
    sprintf(src, "$%d", val);
    char *dst;
    switch(type) {
        case 'B':
            dst = "%al";
            break;
        case 'W':
            dst = "%ax";
            break;
        case 'L':
            dst = "%eax";
            break;
        default:
            dst = "%eax";
            break;
    }
    Move(type, src, dst);
}


/* guarda uma variavel do registrador primario */
void Store(char name, char src_type)
{
    char dst_type = VarType(name);
    Convert(src_type, dst_type, 'a');
    StoreVar(name, dst_type);
}

/* converte os dados do item de um tipo para outro */
void Convert(char src, char dst, char reg)
{
    /* Essa função funciona quando salvamos uma variavel
     (B,W) -> (W,L)
     e a ação são iguais, zero extende %eax
   */

    char tmp_buf[MAX_BUF];
    if (src != dst) {
        switch(src) {
            case 'B':
                sprintf(tmp_buf, "movzx %%%cl, %%e%cx", reg, reg);
                EmitLn(tmp_buf);
                break;
            case 'W':
                sprintf(tmp_buf, "movzx %%%cx, %%e%cx", reg, reg);
                EmitLn(tmp_buf);
                break;
            default:
                break;
        }
    }
}

/* aumenta o tamanho do valor do registrador */
char Promote(char src_type, char dst_type, char reg)
{
    char type = src_type;
    if (src_type != dst_type) {
        if ((src_type == 'B') || ((src_type == 'W' && dst_type == 'L'))) {
            Convert(src_type, dst_type, reg);
            type = dst_type;
        }
    }
    return type;
}

/* força os dois argumentos para o mesmo tipo */
char SameType(char src_type, char dst_type)
{
    src_type = Promote(src_type, dst_type, 'd');
    return Promote(dst_type, src_type, 'a');
}

/* inicializa a tabela de simbolos */
void InitTable(void)
{
    int i;
    for (i = 0; i < MaxEntry; ++i) {
        ST[i] = '?';
    }
}

/* Esvazia a tabela de simbolos */
void DumpTable()
{
    int i;
    for (i = 0; i < MaxEntry; ++i) {
        if (ST[i] != '?') {
            printf("%c: %c\n", i+'A', ST[i]);
        }
    }
}

/* Inicializador */
void Init()
{
    GetChar();
    SkipWhite();
    InitTable();
}

void Clear()
{
    EmitLn("xor %eax, %eax");
}

/* Puxa o primario na pila */
void Push(char type)
{
    switch(type) {
        case 'B':
        case 'W':
            EmitLn("pushw %ax");
            break;
        case 'L':
            EmitLn("pushl %eax");
            break;
        default:
            break;
    }
}

void Pop(char type)
{
    switch(type) {
        case 'B':
        case 'W':
            EmitLn("popw %dx");
            break;
        case 'L':
            EmitLn("popl %edx");
            break;
        default:
            break;
    }
}

/* Adiciona a pilha do topo para o primario */
char PopAdd(char src_type, char dst_type)
{
    Pop(src_type);
    dst_type = SameType(src_type, dst_type);
    GenAdd(dst_type);
    return dst_type;

    EmitLn("addl (%esp), %eax");
    EmitLn("addl $4, %esp");
}

/* Subtrai o primario do topo da pilha */
char PopSub(char src_type, char dst_type)
{
    Pop(src_type);
    dst_type = SameType(src_type, dst_type);
    GenSub(dst_type);
    return dst_type;

    EmitLn("subl (%esp), %eax");
    EmitLn("neg %eax");
    EmitLn("addl $4, %esp");
}

/* Adiciona o topo da pilha a primaria */
void GenAdd(char type)
{
    switch(type) {
        case 'B':
            EmitLn("addb %dl, %al");
            break;
        case 'W':
            EmitLn("addw %dx, %ax");
            break;
        case 'L':
            EmitLn("addl %edx, %eax");
            break;
        default:
            EmitLn("addl %edx, %eax");
            break;
    }
}

/* subtrai o primario do topo da pilha */
void GenSub(char type)
{
    switch(type) {
        case 'B':
            EmitLn("subb %dl, %al");
            EmitLn("neg %al");
            break;
        case 'W':
            EmitLn("subw %dx, %ax");
            EmitLn("neg %ax");
            break;
        case 'L':
            EmitLn("subl %edx, %eax");
            EmitLn("neg %eax");
            break;
        default:
            EmitLn("subl %edx, %eax");
            EmitLn("neg %eax");
            break;
    }
}

/* multiplica o topo da pilha do primario (word) */
void GenMul()
{
    EmitLn("imulw %dx, %ax");
}

/* multiplica o topo da pilha do primario (Long) */
void GenLongMul()
{
    EmitLn("imull %edx, %eax");
}

void GenDiv()
{
    EmitLn("Divisao ainda nao implementada");
}

void GenLongDiv()
{
    EmitLn("Divisao ainda nao implementada!");
}

/* multiplica o topo da pilha com o primario */
char PopMul(char src_type, char dst_type)
{
    Pop(src_type);
    char type = SameType(src_type, dst_type);
    Convert(type, 'W', 'd');
    Convert(type, 'W', 'a');
    if (type == 'L') {
        GenLongMul(type);
    } else {
        GenMul();
    }

    if (type == 'B') {
        type = 'W';
    } else {
        type = 'L';
    }
    return type;
}

/* divide o topo da pilha com o primario */
char PopDiv(char src_type, char dst_type)
{
    char type;
    Pop(src_type);
    Convert(src_type, 'L', 'd');
    if (src_type == 'L' || dst_type == 'L') {
        Convert(dst_type, 'L', 'a');
        GenLongDiv();
        type = 'L';
    } else {
        Convert(dst_type, 'w', 'a');
        GenDiv();
        type = src_type;
    }
    return type;
}

char Term();
char Expression();
void Assignment();
char Factor();
void DoBlock();
void BeginBlock();
void Alloc(char name, char type);
void Decl(void);
void TopDecls(void);

void Header();
void Prolog();
void Epilog();

void Block();

char Unop();
char Add(char type);
char Subtract(char type);
char Multiply(char type);
char Divide(char type);

/* analisa e traduz uma expressao */
char Expression()
{
    char type;
    if (IsAddop(Look)) {
        type = Unop();
    } else {
        type = Term();
    }

    while(IsAddop(Look)) {
        Push(type);
        switch (Look) {
            case '+':
                type = Add(type);
                break;
            case '-':
                type = Subtract(type);
                break;
            default:
                break;
        }
    }
    return type;
}

char Term()
{
    char type = Factor();
    while(IsMulop(Look)) {
        Push(type);
        switch (Look) {
            case '*':
                type = Multiply(type);
                break;
            case '/':
                type = Divide(type);
                break;
            default:
                break;
        }
    }
    return type;
}

/* analisa e traduz um Fatorador */
char Factor()
{
    char type;
    if (Look == '(') {
        Match('(');
        type = Expression();
        Match(')');
    } else if (IsAlpha(Look)) {
        type = Load(GetName());
    } else {
        type = LoadNum(GetNum());
    }
    return type;
}

/* processa um termo com operador unario */
char Unop()
{
    Clear();
    return 'W';
}

char Add(char type)
{
    Match('+');
    return PopAdd(type, Term());
}

char Subtract(char type)
{
    Match('-');
    return PopSub(type, Term());
}

char Multiply(char type)
{
    Match('*');
    return PopMul(type, Factor());
}

char Divide(char type)
{
    Match('/');
    return PopDiv(type, Factor());
}


/* analisa e traduz um atribuição  */
void Assignment()
{
    char name = GetName();
    Match('=');
    Store(name, Expression());
}

void Block()
{
    while(Look != '.') {
        Assignment();
        Fin();
    }
}

/* analise e traduz um bloco */
void DoBlock()
{
    while(strchr("e", Look) == NULL) {
        Assignment();
        Fin();
    }
}

/* analise e traduz o começo de um bloco */
void BeginBlock()
{
    Match('b');
    Fin();
    DoBlock();
    Match('e');
    Fin();
}


/* Gera um codigo para alocação de uma variavel */
void AllocVar(char name, char type)
{
    char *p = "";
    switch(type) {
        case 'B':
            p = "byte";
            break;
        case 'W':
            p = "word";
            break;
        case 'L':
            p = "long";
            break;
        default:
            break;
    }
    printf("%c:\t.%s 0\n", name, p);
}

/* Aloca espaço para variavel */
void Alloc(char name, char type)
{
    AddEntry(name, type);
    AllocVar(name, type);
}

/* analise e traduz dados de declaração */
void Decl(void)
{
    char type = GetName();
    Alloc(GetName(), type);
}

/* analisa e traduz declarações globais */
void TopDecls(void)
{
    printf(".section .data\n");
    char tmp_buf[MAX_BUF];
    while(Look != 'B') {
        switch(Look) {
            case 'b':
            case 'w':
            case 'l':
                Decl();
                break;
            default:
                sprintf(tmp_buf, " Palavra-chave nao reconhecida : %c", Look);
                Abort(tmp_buf);
                break;
        }
        Fin();
    }
}

void Header()
{
    printf(".global _start\n");
}

void Prolog()
{
    EmitLn(".section .text");
    EmitLn("_start:");
}

void Epilog()
{
    EmitLn("movl %eax, %ebx");
    EmitLn("movl $1, %eax");
    EmitLn("int $0x80");
}
void Interface()
{
    printf("-------------- Compilador para C --------------\n");

}
int main(int argc, char *argv[])
{
    Interface();
    Init();
    TopDecls();
    Match('B');
    Fin();
    Block();
    DumpTable();
    return 0;
}
