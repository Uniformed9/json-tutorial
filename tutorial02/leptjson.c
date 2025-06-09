#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include<errno.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')
#define ISDIGIT1TO9(ch) ((ch)>='1'&&(ch)<='9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
static int lept_parse_number_judge(const char* str) {
    const char* p = str;
    int flag = 0;
    if (*p=='-') {
        p++;
    }
    //检查整数部分
    if (*p == '0') {
        p++;
        flag = 1;
    }
    else if(ISDIGIT(*p)) {
        while (ISDIGIT(*p)) {
            p++;
        }
    }
    else {
        return 0;
    }
    //检查小数点,有小数点就必须有整数，这里可以有前导0
    if (*p=='.') {
        //先匹配数字，然后再匹配
        p++;
        if (ISDIGIT(*p)) {
            while (ISDIGIT(*p)) {
                p++;
            }
        }
        else return 0;
    }
    if (*p == 'e'|| *p == 'E') {
        p++;
        if (*p == '-' || *p == '+') {
            p++;
        }
        if (ISDIGIT(*p)) {
            while (ISDIGIT(*p)) {
                p++;
            }
        }
    }
    return *p == '\0'?1:(flag==1?2:0);
}
static int lept_parse_number(lept_context* c, lept_value* v) {
    if (!(ISDIGIT(*c->json) || *c->json == '-'))return LEPT_PARSE_INVALID_VALUE;
    char* end;
    double num;

    errno = 0;
    int ret = lept_parse_number_judge(c->json);
    /* \TODO validate number */
    if (ret==0) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    else if (ret == 2) {
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    else {
        num= strtod(c->json, &end);
        if (errno == ERANGE) {
            if (num != 0) {
                return LEPT_PARSE_NUMBER_TOO_BIG;
            }
           
        }
        v->n = num;
        c->json = end;
        v->type = LEPT_NUMBER;
    }
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   
            return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
