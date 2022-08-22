#include "src/ApiDesc.h"

int main() {
    ApiDesc desc = loadApiDesc("api.yaml");
    printDesc(desc);
    freeApiDesc(desc);
    return 0;
}