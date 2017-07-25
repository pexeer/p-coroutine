
#ifndef P_THREAD_PCONTEXT_H
#define P_THREAD_PCONTEXT_H

typedef void *pcontext_t;

struct transfer_t {
    pcontext_t fctx;
    void *data;
};

#ifdef __cplusplus
extern "C" {
#endif

transfer_t jump_pcontext(pcontext_t const to, void *vp);
pcontext_t make_pcontext(void *sp, size_t size, void (*fn)(transfer_t));

#ifdef __cplusplus
};
#endif

#endif // P_THREAD_PCONTEXT_H
