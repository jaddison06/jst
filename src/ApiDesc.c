#include "ApiDesc.h"

#include <stdbool.h>
#include <stdio.h>

#include "thirdparty/libyaml/yaml.h"
#include "panic.h"

typedef enum {
    Scalar,
    Mapping
} YamlValueType;

struct YamlMapping;

typedef struct {
    char* key;
    YamlValueType type;
    union {
        char* scalar;
        struct YamlMapping* mapping;
    } val;
} YamlMappingEntry;

DECL_VEC(YamlMappingEntry, YamlKVList)

typedef struct YamlMapping {
    int childIdx;
    struct YamlMapping* parent;
    YamlKVList mapping;
} YamlMapping;

static YamlMapping* newYamlMapping(YamlMapping* parent) {
    YamlMapping* out = malloc(sizeof(YamlMapping));
    out->parent = parent;
    out->childIdx = 0;
    return out;
}

static void freeYamlMapping(YamlMapping* mapping) {
    for (int i = 0; i < mapping->mapping.len; i++) {
        switch (mapping->mapping.root[i].type) {
            case Mapping: {
                freeYamlMapping(mapping->mapping.root[i].val.mapping);
                DESTROY(mapping->mapping);
                break;
            }
            case Scalar: {
                free(mapping->mapping.root[i].key);
                free(mapping->mapping.root[i].val.scalar);
                break;
            }
        }
        free(mapping);
    }
}

static YamlMapping* parseYaml(char* fname) {
    YamlMapping* out = newYamlMapping(NULL);
    INIT(out->mapping);
    
    YamlMapping* currentBlock = NULL;

    FILE* fh = fopen(fname, "rt");
    yaml_parser_t parser;
    yaml_event_t event;

    if (fh == NULL) panic("Failed to open file");
    if (!yaml_parser_initialize(&parser)) panic("Failed to init parser");

    yaml_parser_set_input_file(&parser, fh);

    do {
        if (!yaml_parser_parse(&parser, &event)) panic("Failed to get YAML event");

        switch (event.type) {
            case YAML_MAPPING_START_EVENT: {
                printf("Got mapping start: ");
                if (currentBlock == NULL) {
                    printf("First\n");
                    currentBlock = out;
                } else {
                    printf("Subsequent\n");
                    YamlMapping* newMapping = malloc(sizeof(YamlMapping));
                    newMapping->parent = currentBlock;
                    INIT(newMapping->mapping);
                    APPEND(currentBlock->mapping, newMapping);
                    currentBlock->childIdx++;
                    currentBlock = newMapping;
                }
                break;
            }
            case YAML_MAPPING_END_EVENT: {
                printf("Got mapping end\n");
                if (currentBlock != NULL) {
                    currentBlock = currentBlock->parent;
                } else {
                    printf("(first)\n");
                }
                break;
            }
            case YAML_SCALAR_EVENT: {
                printf("Got scalar: ");
                if (currentBlock->childIdx % 2 == 0) {
                    printf("Key\n");
                    // key
                    YamlMappingEntry newEntry;
                    newEntry.key = malloc(strlen(event.data.scalar.value) + 1);
                    strcpy(newEntry.key, event.data.scalar.value);
                    APPEND(currentBlock->mapping, newEntry);
                } else {
                    printf("Value\n");
                    // value
                    YamlMappingEntry* currentEntry = &currentBlock->mapping.root[currentBlock->mapping.len - 1];
                    currentEntry->type = Scalar;
                    currentEntry->val.scalar = malloc(strlen(event.data.scalar.value) + 1);
                    strcpy(currentEntry->val.scalar, event.data.scalar.value);
                }
                currentBlock->childIdx++;
                break;
            }
        }
    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_parser_delete(&parser);
    fclose(fh);

    return out;
}

static void printYaml(YamlMapping* root, int indent) {
    for (int i = 0; i < root->mapping.len; i++) {
        for (int j = 0; j < indent; j++) printf(" ");
        printf("%s: ", root->mapping.root[i].key);
        switch (root->mapping.root[i].type) {
            case Scalar: {
                printf("%s\n", root->mapping.root[i].val.scalar);
                break;
            }
            case Mapping: {
                printf("\n");
                printYaml(root->mapping.root[i].val.mapping, indent + 2);
                break;
            }
        }
    }
}

ApiDesc loadDesc(char* fname) {
    ApiDesc out;

    INIT(out.types);
    INIT(out.actions);

    YamlMapping* root = parseYaml(fname);

    printYaml(root, 0);

    freeYamlMapping(root);

    return out;
}