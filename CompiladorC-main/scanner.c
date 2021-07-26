/* retorna um identificador */

void getName(char *name)

{

                int i;

                if (!isalpha(look))

                               expected("Name");

                for (i = 0; isalnum(look); i++) {

                               if (i >= MAXNAME)

                                               error("Identifier too long.");

                               name[i] = toupper(look);

                               nextChar();

                }

                name[i] = '\0';

}

/* retorna um número */

void getNum(char *num)

{

                int i;

                if (!isdigit(look))

                               expected("Integer");

                for (i = 0; isdigit(look); i++) {

                               if (i >= MAXNUM)

                                               error("Integer too large.");

                               num[i] = look;

                               nextChar();

                }

                num[i] = '\0';

}
