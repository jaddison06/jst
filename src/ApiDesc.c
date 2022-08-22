#include "ApiDesc.h"

ApiDesc loadDesc(char* fname) {
    ApiDesc out;

    INIT(out.types);
    INIT(out.actions);

    return out;
}