#include "ApiDesc.h"

#include <stdbool.h>
#include <stdio.h>

#include "thirdparty/libyaml/yaml.h"
#include "panic.h"

#define CopyString(dst, src) do { \
    dst = malloc(strlen(src) + 1); \
    strcpy(dst, src); \
} while (0)

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

static void freeYamlMapping(YamlMapping* root) {
    for (int i = 0; i < root->mapping.len; i++) {
        free(root->mapping.root[i].key);
        switch (root->mapping.root[i].type) {
            case Mapping: {
                freeYamlMapping(root->mapping.root[i].val.mapping);
                break;
            }
            case Scalar: {
                free(root->mapping.root[i].val.scalar);
                break;
            }
        }
    }
    DESTROY(root->mapping);
    free(root);
}

static YamlMapping* parseYaml(char* fname) {
    YamlMapping* out = newYamlMapping(NULL);
    INIT(out->mapping);
    
    YamlMapping* currentBlock = NULL;

#define CurrentEntry currentBlock->mapping.root[currentBlock->mapping.len - 1]

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
                if (currentBlock == NULL) {
                    currentBlock = out;
                } else {
                    YamlMapping* newMapping = newYamlMapping(currentBlock);
                    INIT(newMapping->mapping);
                    // assume value
                    CurrentEntry.type = Mapping;
                    CurrentEntry.val.mapping = newMapping;
                    currentBlock->childIdx++;
                    currentBlock = newMapping;
                }
                break;
            }
            case YAML_MAPPING_END_EVENT: {
                if (currentBlock != NULL) {
                    currentBlock = currentBlock->parent;
                }
                break;
            }
            case YAML_SCALAR_EVENT: {
                if (currentBlock->childIdx % 2 == 0) {
                    // key
                    YamlMappingEntry newEntry;
                    CopyString(newEntry.key, event.data.scalar.value);
                    APPEND(currentBlock->mapping, newEntry);
                } else {
                    // value
                    CurrentEntry.type = Scalar;
                    CopyString(CurrentEntry.val.scalar, event.data.scalar.value);
                }
                currentBlock->childIdx++;
                break;
            }
        }
        if (event.type != YAML_STREAM_END_EVENT) yaml_event_delete(&event);
    } while (event.type != YAML_STREAM_END_EVENT);
    
    yaml_event_delete(&event);

#undef CurrentEntry

    yaml_parser_delete(&parser);
    fclose(fh);

    return out;
}

static void printYaml(YamlMapping* root, int indent) {
#define INDENT() for (int j = 0; j < indent; j++) printf(" ")
    for (int i = 0; i < root->mapping.len; i++) {
        INDENT();
        printf("%s (%p): ", root->mapping.root[i].key, &root->mapping.root[i]);
        switch (root->mapping.root[i].type) {
            case Scalar: {
                printf("(scalar) ");
                printf("%s\n", root->mapping.root[i].val.scalar);
                break;
            }
            case Mapping: {
                printf("\n");
                INDENT();
                printf("(mapping at %p)\n", &root->mapping.root[i].val.mapping);
                printYaml(root->mapping.root[i].val.mapping, indent + 2);
                break;
            }
        }
    }
#undef INDENT
}

// same shit different day
void printDesc(ApiDesc root) {
    printf("--- TYPES ---\n");
    for (int i = 0; i < root.types.len; i++) {
        Type currentType = root.types.root[i];
        printf("%s:\n", currentType.name);
        for (int j = 0; j < currentType.members.len; j++) {
            TypedMember currentMember = currentType.members.root[j];
            printf("  %s %s\n", currentMember.type, currentMember.name);
        }
    }
    printf("--- ACTIONS ---\n");
    for (int i = 0; i < root.actions.len; i++) {
        Action currentAction = root.actions.root[i];
        if (currentAction.returnType != NULL) printf("%s ", currentAction.returnType);
        printf("%s(", currentAction.name);
        for (int j = 0; j < currentAction.params.len; j++) {
            TypedMember currentParam = currentAction.params.root[j];
            printf("%s %s", currentParam.type, currentParam.name);
            if (j != currentAction.params.len - 1) printf(", ");
        }
        printf(")\n");
    }
}

static YamlMappingEntry* mapGet(YamlMapping* root, char* key) {
    for (int i = 0; i < root->mapping.len; i++) {
        if (strcmp(root->mapping.root[i].key, key) == 0) return &root->mapping.root[i];
    }
    return NULL;
}

void freeApiDesc(ApiDesc desc) {
    for (int i = 0; i < desc.types.len; i++) {
        Type currentType = desc.types.root[i];
        free(currentType.name);
        for (int j = 0; j < currentType.members.len; j++) {
            TypedMember currentMember = currentType.members.root[j];
            free(currentMember.name);
            free(currentMember.type);
        }
        DESTROY(currentType.members);
    }

    for (int i = 0; i < desc.actions.len; i++) {
        Action currentAction = desc.actions.root[i];
        free(currentAction.name);
        if (currentAction.returnType != NULL) free(currentAction.returnType);
        for (int j = 0; j < currentAction.params.len; j++) {
            TypedMember currentParam = currentAction.params.root[j];
            free(currentParam.name);
            free(currentParam.type);
        }
        DESTROY(currentAction.params);
    }

    for (int i = 0; i < desc.objects.len; i++) {
        TypedMember currentObject = desc.objects.root[i];
        free(currentObject.name);
        free(currentObject.type);
    }

    DESTROY(desc.types);
    DESTROY(desc.actions);
    DESTROY(desc.objects);
}

ApiDesc loadApiDesc(char* fname) {
    ApiDesc out;

    INIT(out.types);
    INIT(out.actions);
    INIT(out.objects);

    YamlMapping* root = parseYaml(fname);

    // printYaml(root, 0);

    YamlMappingEntry* types = mapGet(root, "types");
    YamlMappingEntry* actions = mapGet(root, "actions");
    YamlMappingEntry* objects = mapGet(root, "objects");

    if (types != NULL) {
        if (types->type != Mapping) panic("'types' must be a key/value dictionary");
        for (int i = 0; i < types->val.mapping->mapping.len; i++) {
            YamlMappingEntry currentTypeRaw = types->val.mapping->mapping.root[i];
            if (currentTypeRaw.type != Mapping) panic("A type must correspond to a key/value dictionary of members : types");
            Type currentType;
            CopyString(currentType.name, currentTypeRaw.key);
            INIT(currentType.members);
            for (int j = 0; j < currentTypeRaw.val.mapping->mapping.len; j++) {
                YamlMappingEntry currentMemberRaw = currentTypeRaw.val.mapping->mapping.root[j];
                if (currentMemberRaw.type != Scalar) panic("A type must correspond to a key/value dictionary of members : types");
                TypedMember currentMember;
                CopyString(currentMember.name, currentMemberRaw.key);
                CopyString(currentMember.type, currentMemberRaw.val.scalar);
                APPEND(currentType.members, currentMember);
            }
            APPEND(out.types, currentType);
        }
    }
    if (actions != NULL) {
        if (actions->type != Mapping) panic("'actions' must be a key/value dictionary");
        for (int i = 0; i < actions->val.mapping->mapping.len; i++) {
            YamlMappingEntry currentActionRaw = actions->val.mapping->mapping.root[i];
            if (currentActionRaw.type != Mapping) panic("An action must correspond to a key/value dictionary of data");
            Action currentAction;
            CopyString(currentAction.name, currentActionRaw.key);
            YamlMappingEntry* returnTypeRaw = mapGet(currentActionRaw.val.mapping, "returns");
            if (returnTypeRaw != NULL) {
                if (returnTypeRaw->type != Scalar) panic("returnType must have a string value");
                CopyString(currentAction.returnType, returnTypeRaw->val.scalar);
            } else {
                currentAction.returnType = NULL;
            }
            INIT(currentAction.params);
            YamlMappingEntry* paramsRaw = mapGet(currentActionRaw.val.mapping, "params");
            if (paramsRaw != NULL) {
                if (paramsRaw->type != Mapping) panic("'params' must correspond to a key/value dictionary of members : types");
                for (int j = 0; j < paramsRaw->val.mapping->mapping.len; j++) {
                    YamlMappingEntry currentParamRaw = paramsRaw->val.mapping->mapping.root[j];
                    if (currentParamRaw.type != Scalar) panic("'params' must correspond to a key/value dictionary of members : types");
                    TypedMember currentParam;
                    CopyString(currentParam.name, currentParamRaw.key);
                    CopyString(currentParam.type, currentParamRaw.val.scalar);
                    APPEND(currentAction.params, currentParam);
                }
            }
            APPEND(out.actions, currentAction);
        }
    }
    if (objects != NULL) {
        if (objects->type != Mapping) panic("'objects' must be a key/value dictionary");
        for (int i = 0; i < objects->val.mapping->mapping.len; i++) {
            YamlMappingEntry currentObjectRaw = actions->val.mapping->mapping.root[i];
            if (currentObjectRaw.type != Scalar) panic("'objects' must correspond to a key/value dictionary of endpoints : types");
            TypedMember currentObject;
            CopyString(currentObject.name, currentObjectRaw.key);
            CopyString(currentObject.type, currentObjectRaw.val.scalar);
            APPEND(out.objects, currentObject);
        }
    }

    freeYamlMapping(root);

    return out;
}