#ifndef _ASSERT_H
#define _ASSERT_H 1

#define ASSERT( exp )\
({\
        if(!exp) {\
                printf("assert failed at %s:%d", __FILE__, __LINE__);\
                printf(" inside %s\n", __FUNCTION__);\
                abort();\
        }\
})

#endif
