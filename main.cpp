#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "src/context.h"

//extern "C" transfer_t jump_pcontext( pcontext_t const to, void * vp);
//extern "C" pcontext_t make_pcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

void z(transfer_t caller) {
    caller = jump_pcontext(caller.fctx, caller.data);
    printf("f1 end\n");
    printf("caller= data=%p, fctx=%p\n", caller.data, caller.fctx);
    jump_pcontext(caller.fctx, caller.data);
}

void f1(transfer_t caller) {
    printf("caller= data=%p, fctx=%p\n", caller.data, caller.fctx);
    printf("f1 begin\n");
    int * tmp = (int*)caller.data;
    ++*tmp;
    z(caller);
}

char sp1[4096];
char sp2[4096];

int main1() {
  pcontext_t fc1 = make_pcontext(sp1 + sizeof(sp1), sizeof(sp1), f1);
  printf("fc1=%p\n", fc1);
  int arg  = 0;
  transfer_t x = jump_pcontext(fc1, &arg);
  printf("ret= data=%p, fctx=%p\n", x.data, x.fctx);
  jump_pcontext(x.fctx, x.data);

  printf("end,,,ret= data=%p, fctx=%p\n", x.data, x.fctx);
  return 0;
}


void f() {
    LOG_TRACE << "called f";
}

int main2() {
    p::thread::ThreadContext* tc = p::thread::ThreadContext::NewThreadContext();
    tc->jump(f);

    LOG_TRACE << "main_return,";

    tc->jump(f);

    stop_log();

    return 0;
}

void ff(transfer_t caller) {
    printf("caller= data=%p, fctx=%p\n", caller.data, caller.fctx);
}

int main3() {
  pcontext_t fc1 = make_pcontext(sp1 + sizeof(sp1), sizeof(sp1), ff);
  printf("fc1=%p\n", fc1);
  transfer_t x = jump_pcontext(fc1, &fc1);
  printf("return = data=%p, fctx=%p\n", x.data, x.fctx);
  return 0;
}

int main() {
    main2();
    return 0;
}
