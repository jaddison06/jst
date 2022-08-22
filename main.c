#include "src/ApiDesc.h"

int main() {
    ApiDesc desc = loadApiDesc("api.yaml");
    freeApiDesc(desc);
    return 0;
}