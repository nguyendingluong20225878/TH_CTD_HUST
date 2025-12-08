/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

// Khai báo các hàm giải phóng bộ nhớ để dùng trong nội bộ
void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

// Các biến toàn cục quản lý Bảng Ký hiệu
SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

// Hàm tạo kiểu nguyên thủy Integer (TP_INT)
Type* makeIntType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_INT;
  type->arraySize = 0;
  type->elementType = NULL;
  return type;
}

// Hàm tạo kiểu nguyên thủy Character (TP_CHAR)
Type* makeCharType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_CHAR;
  type->arraySize = 0;
  type->elementType = NULL;
  return type;
}

// Hàm tạo kiểu mảng (TP_ARRAY)
Type* makeArrayType(int arraySize, Type* elementType) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_ARRAY;
  type->arraySize = arraySize;
  type->elementType = elementType;
  return type;
}

// Hàm tạo bản sao (deep copy) của một Type
Type* duplicateType(Type* type) {
  Type * newType = (Type*) malloc(sizeof(Type));
  newType->typeClass = type->typeClass;
  newType->arraySize = type->arraySize;
  
  // Nếu là mảng, phải gọi đệ quy để duplicate kiểu phần tử (deep copy)
  if (type->typeClass == TP_ARRAY) {
    newType->elementType = duplicateType(type->elementType);
  } else {
    newType->elementType = NULL; 
  }

  return newType;
}

// Hàm so sánh hai kiểu (Type): trả về 1 nếu giống, 0 nếu khác
int compareType(Type* type1, Type* type2) {
  // 1. So sánh lớp kiểu (TP_INT, TP_CHAR, TP_ARRAY)
  if (type1->typeClass != type2->typeClass) {
    return 0;
  }

  if (type1->typeClass == TP_ARRAY) {
    // 2. Nếu là mảng, phải so sánh kích thước và kiểu phần tử (đệ quy)
    if (type1->arraySize != type2->arraySize)
      return 0;
      
    return compareType(type1->elementType, type2->elementType);
  }

  // 3. Nếu là TP_INT hoặc TP_CHAR, lớp kiểu giống nhau là đủ
  return 1;
}

// Hàm giải phóng bộ nhớ cấp phát cho Type
void freeType(Type* type) {
  // KHÔNG free các kiểu cơ sở toàn cục (intType, charType)
  if (type == intType || type == charType) return;
  
  if (type != NULL) {
    if (type->typeClass == TP_ARRAY) {
      freeType(type->elementType); // Giải phóng đệ quy kiểu phần tử
    }
    free(type);
  }
}

/******************* Constant utility ******************************/

// Hàm tạo giá trị hằng số kiểu Integer
ConstantValue* makeIntConstant(int i) {
  ConstantValue * constValue = (ConstantValue*) malloc(sizeof(ConstantValue));
  constValue->type = TP_INT;
  constValue->intValue = i;
  return constValue;
}

// Hàm tạo giá trị hằng số kiểu Character
ConstantValue* makeCharConstant(char ch) {
  ConstantValue * constValue = (ConstantValue*) malloc(sizeof(ConstantValue));
  constValue->type = TP_CHAR;
  constValue->charValue = ch;
  return constValue;
}

// Hàm tạo bản sao của một ConstantValue
ConstantValue* duplicateConstantValue(ConstantValue* v) {
  ConstantValue * constValue = (ConstantValue*) malloc(sizeof(ConstantValue));
  constValue->type = v->type;
  if (v->type == TP_INT)
    constValue->intValue = v->intValue;
  else if (v->type == TP_CHAR)
    constValue->charValue = v->charValue;

  return constValue;
}

/******************* Object utilities ******************************/

// Hàm tạo một Scope mới
Scope* createScope(Object* owner, Scope* outer) {
  Scope* scope = (Scope*) malloc(sizeof(Scope));
  scope->objList = NULL; // Danh sách đối tượng trong scope này
  scope->owner = owner; // Đối tượng sở hữu scope này (Function, Procedure, Program)
  scope->outer = outer; // Scope bao ngoài
  return scope;
}

// Hàm tạo đối tượng Chương trình (OBJ_PROGRAM)
Object* createProgramObject(char *programName) {
  Object* program = (Object*) malloc(sizeof(Object));
  strcpy(program->name, programName);
  program->kind = OBJ_PROGRAM;
  program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
  // Chương trình là scope ngoài cùng (outer = NULL)
  program->progAttrs->scope = createScope(program,NULL); 
  symtab->program = program; // Cập nhật con trỏ chương trình trong SymTab

  return program;
}

// Hàm tạo đối tượng Hằng số (OBJ_CONSTANT)
Object* createConstantObject(char *name) {
  Object* c = (Object *) malloc(sizeof(Object));
  strcpy(c->name, name);
  c->kind = OBJ_CONSTANT;
  c->constAttrs = (ConstantAttributes *) malloc(sizeof(ConstantAttributes));
  c->constAttrs->value = NULL; 
  return c;
}

// Hàm tạo đối tượng Kiểu (OBJ_TYPE)
Object* createTypeObject(char *name) {
  Object* t = (Object*) malloc(sizeof(Object));
  strcpy(t->name, name);
  t->kind = OBJ_TYPE;
  t->typeAttrs = (TypeAttributes*) malloc(sizeof(TypeAttributes));
  t->typeAttrs->actualType = NULL; 
  return t;
}

// Hàm tạo đối tượng Biến (OBJ_VARIABLE)
Object* createVariableObject(char *name) {
  Object* v = (Object*) malloc(sizeof(Object));
  strcpy(v->name, name);
  v->kind = OBJ_VARIABLE;
  v->varAttrs = (VariableAttributes*) malloc(sizeof(VariableAttributes));
  v->varAttrs->type = NULL; 
  v->varAttrs->scope = symtab->currentScope; // Lưu lại scope hiện tại
  return v;
}

// Hàm tạo đối tượng Hàm (OBJ_FUNCTION)
Object* createFunctionObject(char *name) {
  Object* f = (Object*) malloc(sizeof(Object));
  strcpy(f->name, name);
  f->kind = OBJ_FUNCTION;
  f->funcAttrs = (FunctionAttributes*) malloc(sizeof(FunctionAttributes));
  f->funcAttrs->paramList = NULL; // Danh sách tham số
  f->funcAttrs->returnType = NULL; // Kiểu trả về
  // Scope của hàm nằm trong scope hiện tại
  f->funcAttrs->scope = createScope(f, symtab->currentScope); 
  return f;
}

// Hàm tạo đối tượng Thủ tục (OBJ_PROCEDURE)
Object* createProcedureObject(char *name) {
  Object* p = (Object*) malloc(sizeof(Object));
  strcpy(p->name, name);
  p->kind = OBJ_PROCEDURE;
  p->procAttrs = (ProcedureAttributes*) malloc(sizeof(ProcedureAttributes));
  p->procAttrs->paramList = NULL; // Danh sách tham số
  // Scope của thủ tục nằm trong scope hiện tại
  p->procAttrs->scope = createScope(p, symtab->currentScope); 
  return p;
}

// Hàm tạo đối tượng Tham số (OBJ_PARAMETER)
Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
  Object* param = (Object*) malloc(sizeof(Object));
  strcpy(param->name, name);
  param->kind = OBJ_PARAMETER;
  param->paramAttrs = (ParameterAttributes*) malloc(sizeof(ParameterAttributes));
  param->paramAttrs->function = owner; // Hàm/Thủ tục sở hữu tham số
  param->paramAttrs->kind = kind; // Tham biến (VAR) hay Tham trị (VALUE)
  param->paramAttrs->type = NULL; 
  return param;
}

// Hàm giải phóng bộ nhớ cho một Object
void freeObject(Object* obj) {
  if (obj == NULL) return;

  switch (obj->kind) {
  case OBJ_CONSTANT:
    if (obj->constAttrs->value != NULL) free(obj->constAttrs->value);
    free(obj->constAttrs);
    break;
  case OBJ_VARIABLE:
    free(obj->varAttrs);
    break;
  case OBJ_TYPE:
    free(obj->typeAttrs);
    break;
  case OBJ_PARAMETER:
    free(obj->paramAttrs);
    break;
  case OBJ_PROGRAM:
    freeScope(obj->progAttrs->scope); 
    free(obj->progAttrs);
    break;
  case OBJ_FUNCTION:
    freeScope(obj->funcAttrs->scope); // Đã giải phóng paramList (node) bên trong freeScope
    free(obj->funcAttrs);
    break;
  case OBJ_PROCEDURE:
    freeScope(obj->procAttrs->scope); // Đã giải phóng paramList (node) bên trong freeScope
    free(obj->procAttrs);
    break;
  }
  
  free(obj);
}

// Hàm giải phóng bộ nhớ cho một Scope
void freeScope(Scope* scope) {
  if (scope != NULL) {
    // Giải phóng danh sách tham số (chỉ node, vì Object đã nằm trong objList)
    if (scope->owner->kind == OBJ_FUNCTION)
        freeReferenceList(scope->owner->funcAttrs->paramList);
    if (scope->owner->kind == OBJ_PROCEDURE)
        freeReferenceList(scope->owner->procAttrs->paramList);

    // Giải phóng danh sách Object trong scope và các Object bên trong
    freeObjectList(scope->objList);
    free(scope);
  }
}

// Hàm giải phóng danh sách ObjectNode và các Object mà nó trỏ tới (dùng cho objList)
void freeObjectList(ObjectNode *objList) {
  ObjectNode *current = objList;
  ObjectNode *next;
  while (current != NULL) {
    next = current->next;
    freeObject(current->object); // Giải phóng Object
    free(current);              // Giải phóng Node
    current = next;
  }
}

// Hàm chỉ giải phóng danh sách ObjectNode, KHÔNG giải phóng Object (dùng cho paramList)
void freeReferenceList(ObjectNode *objList) {
  ObjectNode *current = objList;
  ObjectNode *next;
  while (current != NULL) {
    next = current->next;
    // KHÔNG gọi freeObject(current->object)
    free(current); // Chỉ giải phóng Node
    current = next;
  }
}

// Hàm thêm một Object vào cuối danh sách objList
void addObject(ObjectNode **objList, Object* obj) {
  ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
  node->object = obj;
  node->next = NULL;
  if ((*objList) == NULL) 
    *objList = node;
  else {
    ObjectNode *n = *objList;
    while (n->next != NULL) 
      n = n->next;
    n->next = node;
  }
}

// Hàm tìm kiếm Object theo tên trong danh sách objList
Object* findObject(ObjectNode *objList, char *name) {
  ObjectNode * currentNode = objList;
  while(currentNode != NULL) {
    if (strcmp(currentNode->object->name, name) == 0) {
      return currentNode->object;
    }
    currentNode = currentNode->next;
  }

  return NULL;
}

/******************* others ******************************/

// Hàm khởi tạo Bảng Ký hiệu (Symbol Table) và các đối tượng Built-in
void initSymTab(void) {
  Object* obj;
  Object* param;

  symtab = (SymTab*) malloc(sizeof(SymTab));
  symtab->globalObjectList = NULL;
  symtab->currentScope = NULL;
  
  // Các hàm và thủ tục Built-in (Global Object List)

  obj = createFunctionObject("READC");
  obj->funcAttrs->returnType = makeCharType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createFunctionObject("READI");
  obj->funcAttrs->returnType = makeIntType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEI");
  param = createParameterObject("i", PARAM_VALUE, obj);
  param->paramAttrs->type = makeIntType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEC");
  param = createParameterObject("ch", PARAM_VALUE, obj);
  param->paramAttrs->type = makeCharType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITELN");
  addObject(&(symtab->globalObjectList), obj);

  // Tạo các kiểu cơ sở toàn cục
  intType = makeIntType();
  charType = makeCharType();
}

// Hàm dọn dẹp và giải phóng bộ nhớ cho toàn bộ SymTab
void cleanSymTab(void) {
  // Giải phóng đối tượng chương trình và các scope con
  freeObject(symtab->program); 
  // Giải phóng các đối tượng Built-in
  freeObjectList(symtab->globalObjectList); 
  // Giải phóng các kiểu cơ sở toàn cục
  free(intType); 
  free(charType);
  free(symtab);
}

// Hàm vào một khối (Scope) mới
void enterBlock(Scope* scope) {
  symtab->currentScope = scope;
}

// Hàm thoát khỏi khối (Scope) hiện tại (quay về Scope ngoài)
void exitBlock(void) {
  symtab->currentScope = symtab->currentScope->outer;
}

// Hàm khai báo một Object vào Scope hiện tại
void declareObject(Object* obj) {
  // Nếu là Tham số (Parameter), thêm vào paramList của Function/Procedure
  if (obj->kind == OBJ_PARAMETER) {
    Object* owner = symtab->currentScope->owner;
    switch (owner->kind) {
    case OBJ_FUNCTION:
      addObject(&(owner->funcAttrs->paramList), obj);
      break;
    case OBJ_PROCEDURE:
      addObject(&(owner->procAttrs->paramList), obj);
      break;
    default:
      break;
    }
  }
 
  // Thêm vào danh sách Object của Scope hiện tại
  addObject(&(symtab->currentScope->objList), obj);
}