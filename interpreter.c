#include <stdio.h>
#include "calc3.h"
#ifdef __APPLE__
#include "y.tab.h"
#else
#include "calc3.tab.h"
#endif

double ex(nodeType *p) {
  if (!p) return 0;
  switch (p->type) {
    case typeCon:
      return p->con.value;
    case typeId:
      return sym[p->id.i];
    case typeOpr:
      switch (p->opr.oper) {
        case WHILE:
          while (ex(p->opr.op[0])) ex(p->opr.op[1]);
          return 0;
        case IF:
          if (ex(p->opr.op[0]))
            ex(p->opr.op[1]);
          else if (p->opr.nops > 2)
            ex(p->opr.op[2]);
          return 0;
        case PRINT:
          printf("%f\n", ex(p->opr.op[0]));
          return 0;
        case ';':
          ex(p->opr.op[0]);
          return ex(p->opr.op[1]);
        case '=':
          return sym[p->opr.op[0]->id.i] = ex(p->opr.op[1]);
        case UMINUS:
          return -ex(p->opr.op[0]);
        case '+':
          return ex(p->opr.op[0]) + ex(p->opr.op[1]);
        case '-':
          return ex(p->opr.op[0]) - ex(p->opr.op[1]);
        case '*':
          return ex(p->opr.op[0]) * ex(p->opr.op[1]);
        case '/':
          return ex(p->opr.op[0]) / ex(p->opr.op[1]);
        case '%':
        return (int)ex(p->opr.op[0]) % (int)ex(p->opr.op[1]);
        case '<':
          return ex(p->opr.op[0]) < ex(p->opr.op[1]);
        case '>':
          return ex(p->opr.op[0]) > ex(p->opr.op[1]);
        case PLUSE:
          return sym[p->opr.op[0]->id.i] += ex(p->opr.op[1]);
        case MINUSE:
          return sym[p->opr.op[0]->id.i] -= ex(p->opr.op[1]);
        case MULE:
          return sym[p->opr.op[0]->id.i] *= ex(p->opr.op[1]);
        case DIVE:
          return sym[p->opr.op[0]->id.i] /= ex(p->opr.op[1]);
        case GE:
          return ex(p->opr.op[0]) >= ex(p->opr.op[1]);
        case LE:
          return ex(p->opr.op[0]) <= ex(p->opr.op[1]);
        case NE:
          return ex(p->opr.op[0]) != ex(p->opr.op[1]);
        case EQ:
          return ex(p->opr.op[0]) == ex(p->opr.op[1]);
      }
  }
  return 0;
}
