#pragma once

#include "vector.h"

typedef struct {
    char* name;
    char* type;
} TypedMember;

DECL_VEC(TypedMember, MemberList)

typedef struct {
    char* name;
    MemberList members;
} Type;

typedef struct {
    char* name;
    char* returnType;
    MemberList params;
} Action;

DECL_VEC(Type, ApiTypesList)
DECL_VEC(Action, ApiActionsList)

typedef struct {
    ApiTypesList types;
    ApiActionsList actions;
} ApiDesc;

ApiDesc loadDesc(char* fname);