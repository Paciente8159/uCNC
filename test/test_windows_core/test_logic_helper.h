#ifndef TEST_LOGI_HELPER_H
#define TEST_LOGI_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

    typedef enum
    {
        EQ,
        NOT_EQ,
        EMPTY,
        CONTAIN,
        START_WITH,
        END_WITH
    } logic_str_op_t;

    static bool test_logic_str(const char *left, logic_op_t op, const char *right)
    {
        switch (op)
        {
        case EQ:
            return ((strlen(left) == strlen(right)) && (strcmp(left, right) == 0));

        case NOT_EQ:
            return ((strlen(left) != strlen(right)) || (strcmp(left, right) != 0));

        case EMPTY:
            return left == NULL || left[0] == '\0';

        case CONTAIN:
            return (left && right && strstr(left, right) != NULL);

        case START_WITH:
            return (left && right && strncmp(left, right, strlen(right)) == 0);

        case END_WITH:
            if (!left || !right)
                return false;
            {
                size_t len_left = strlen(left);
                size_t len_right = strlen(right);
                if (len_right > len_left)
                    return false;
                return strcmp(left + (len_left - len_right), right) == 0;
            }
        }

        return false;
    }

#ifdef __cplusplus
}
#endif

#endif