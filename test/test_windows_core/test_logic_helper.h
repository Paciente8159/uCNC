#ifndef TEST_LOGI_HELPER_H
#define TEST_LOGI_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

    typedef enum
    {
        EQ,
        NOT_EQ,
        GT,
        GT_EQ,
        LT,
        LT_EQ
    } logic_op_t;

    static const char *logic_op_name[6] = {"==", "!=", ">", ">=", "<", "<="};


    static bool test_logic_flt(float left, logic_op_t op, float right)
    {
        switch (op)
        {
        case EQ:
            return (left == right);
        case NOT_EQ:
            return (left != right);
        case GT:
            return (left > right);
        case GT_EQ:
            return (left >= right);
        case LT:
            return (left < right);
        case LT_EQ:
            return (left <= right);
        }

        return false;
    }

    static bool test_logic_int(long int left, logic_op_t op, long int right)
    {
        switch (op)
        {
        case EQ:
            return (left == right);
        case NOT_EQ:
            return (left != right);
        case GT:
            return (left > right);
        case GT_EQ:
            return (left >= right);
        case LT:
            return (left < right);
        case LT_EQ:
            return (left <= right);
        }

        return false;
    }

#ifdef __cplusplus
}
#endif

#endif