/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include "reader.h"
#include "codegen.h"  

#define CODE_SIZE 10000
extern SymTab* symtab; // Biến toàn cục trỏ tới bảng ký hiệu

// Các đối tượng hàm/thủ tục dựng sẵn (nhập/xuất)
extern Object* readiFunction;
extern Object* readcFunction;
extern Object* writeiProcedure;
extern Object* writecProcedure;
extern Object* writelnProcedure;

CodeBlock* codeBlock; // Khối nhớ chứa mã lệnh được sinh ra

// Tính toán mức độ lồng nhau (nesting level) giữa phạm vi hiện tại và phạm vi định nghĩa biến.
// Dùng để xác định cần nhảy qua bao nhiêu Static Link để truy cập biến.
int computeNestedLevel(Scope* scope) {
  int level = 0;
  Scope* tmp = symtab->currentScope; // Bắt đầu từ phạm vi hiện tại
  while (tmp != scope) {
    tmp = tmp->outer; // Đi ngược ra phạm vi bao ngoài
    level ++;
  }
  return level;
}

// Sinh lệnh lấy địa chỉ của một biến (Variable)
void genVariableAddress(Object* var) {
  int level = computeNestedLevel(VARIABLE_SCOPE(var)); // Tính chênh lệch cấp độ
  int offset = VARIABLE_OFFSET(var); // Lấy vị trí (offset) của biến trong stack frame
  genLA(level, offset); // Sinh lệnh Load Address (LA)
}

// Sinh lệnh lấy giá trị của một biến
void genVariableValue(Object* var) {
  int level = computeNestedLevel(VARIABLE_SCOPE(var));
  int offset = VARIABLE_OFFSET(var);
  genLV(level, offset); // Sinh lệnh Load Value (LV)
}

// Sinh lệnh lấy địa chỉ của tham số (Parameter)
void genParameterAddress(Object* param) {
  int level = computeNestedLevel(PARAMETER_SCOPE(param));
  int offset = PARAMETER_OFFSET(param);
  genLA(level, offset);
}

// Sinh lệnh lấy giá trị của tham số
void genParameterValue(Object* param) {
  int level = computeNestedLevel(PARAMETER_SCOPE(param));
  int offset = PARAMETER_OFFSET(param);
  genLV(level, offset);
}

// Sinh lệnh lấy địa chỉ trả về của hàm (đặt tại offset cố định)
void genReturnValueAddress(Object* func) {
  int level = computeNestedLevel(FUNCTION_SCOPE(func));
  int offset = RETURN_VALUE_OFFSET;
  genLA(level, offset);
}

// Sinh lệnh lấy giá trị trả về của hàm
void genReturnValueValue(Object* func) {
  int level = computeNestedLevel(FUNCTION_SCOPE(func));
  int offset = RETURN_VALUE_OFFSET;
  genLV(level, offset);
}

// Xử lý gọi các thủ tục dựng sẵn (Write Integer, Write Char, Write Line)
void genPredefinedProcedureCall(Object* proc) {
  if (proc == writeiProcedure)
    genWRI(); // Sinh lệnh Write Integer
  else if (proc == writecProcedure)
    genWRC(); // Sinh lệnh Write Char
  else if (proc == writelnProcedure)
    genWLN(); // Sinh lệnh Write Line (xuống dòng)
}

// Xử lý gọi thủ tục người dùng định nghĩa
void genProcedureCall(Object* proc) {
  int level = computeNestedLevel(proc->procAttrs->scope->outer);
  genCALL(level, proc->procAttrs->codeAddress); // Sinh lệnh CALL tới địa chỉ code của thủ tục
}

// Xử lý gọi hàm dựng sẵn (Read Integer, Read Char)
void genPredefinedFunctionCall(Object* func) {
  if (func == readiFunction)
    genRI();
  else if (func == readcFunction)
    genRC();
}

// Xử lý gọi hàm người dùng định nghĩa
void genFunctionCall(Object* func) {
  int level = computeNestedLevel(func->funcAttrs->scope->outer);
  genCALL(level, func->funcAttrs->codeAddress);
}

// --- Các hàm wrapper bên dưới gọi tới hàm emit... trong instructions.c ---

void genLA(int level, int offset) {
  emitLA(codeBlock, level, offset); // Load Address: Tải địa chỉ biến lên đỉnh stack
}

void genLV(int level, int offset) {
  emitLV(codeBlock, level, offset); // Load Value: Tải giá trị biến lên đỉnh stack
}

void genLC(WORD constant) {
  emitLC(codeBlock, constant); // Load Constant: Tải hằng số lên đỉnh stack
}

void genLI(void) {
  emitLI(codeBlock); // Load Indirect: Tải giá trị từ địa chỉ đang nằm ở đỉnh stack
}

void genINT(int delta) {
  emitINT(codeBlock,delta); // Increment T: Tăng con trỏ stack (dành chỗ cho biến cục bộ)
}

void genDCT(int delta) {
  emitDCT(codeBlock,delta); // Decrement T: Giảm con trỏ stack
}

// Sinh lệnh nhảy không điều kiện (Jump)
Instruction* genJ(CodeAddress label) {
  Instruction* inst = codeBlock->code + codeBlock->codeSize;
  emitJ(codeBlock,label);
  return inst; // Trả về con trỏ tới lệnh này để sau này cập nhật nhãn (label) nếu cần
}

// Sinh lệnh nhảy khi sai (False Jump) - dùng cho IF, WHILE
Instruction* genFJ(CodeAddress label) {
  Instruction* inst = codeBlock->code + codeBlock->codeSize;
  emitFJ(codeBlock, label);
  return inst;
}

// Sinh lệnh dừng chương trình (Halt)
void genHL(void) {
  emitHL(codeBlock);
}

// Sinh lệnh lưu trữ (Store): Gán giá trị đỉnh stack vào địa chỉ nằm dưới đỉnh stack
void genST(void) {
  emitST(codeBlock);
}

// Sinh lệnh gọi hàm/thủ tục
void genCALL(int level, CodeAddress label) {
  emitCALL(codeBlock, level, label);
}

// Sinh lệnh thoát thủ tục (Exit Procedure)
void genEP(void) {
  emitEP(codeBlock);
}

// Sinh lệnh thoát hàm (Exit Function)
void genEF(void) {
  emitEF(codeBlock);
}

// Các lệnh tính toán số học và so sánh
void genRC(void) { emitRC(codeBlock); } // Đọc ký tự
void genRI(void) { emitRI(codeBlock); } // Đọc số nguyên
void genWRC(void) { emitWRC(codeBlock); } // Viết ký tự
void genWRI(void) { emitWRI(codeBlock); } // Viết số nguyên
void genWLN(void) { emitWLN(codeBlock); } // Xuống dòng
void genAD(void) { emitAD(codeBlock); } // Cộng (Add)
void genSB(void) { emitSB(codeBlock); } // Trừ (Subtract)
void genML(void) { emitML(codeBlock); } // Nhân (Multiply)
void genDV(void) { emitDV(codeBlock); } // Chia (Divide)
void genNEG(void) { emitNEG(codeBlock); } // Đảo dấu (Negative)
void genCV(void) { emitCV(codeBlock); } // Copy giá trị đỉnh stack
void genEQ(void) { emitEQ(codeBlock); } // So sánh bằng (Equal)
void genNE(void) { emitNE(codeBlock); } // So sánh khác (Not Equal)
void genGT(void) { emitGT(codeBlock); } // Lớn hơn (Greater Than)
void genGE(void) { emitGE(codeBlock); } // Lớn hơn hoặc bằng
void genLT(void) { emitLT(codeBlock); } // Nhỏ hơn
void genLE(void) { emitLE(codeBlock); } // Nhỏ hơn hoặc bằng

// Cập nhật nhãn đích cho lệnh nhảy (dùng khi quay lui - backpatching)
void updateJ(Instruction* jmp, CodeAddress label) {
  jmp->q = label;
}

// Cập nhật nhãn đích cho lệnh nhảy False
void updateFJ(Instruction* jmp, CodeAddress label) {
  jmp->q = label;
}

// Lấy địa chỉ lệnh hiện tại (độ dài code đã sinh)
CodeAddress getCurrentCodeAddress(void) {
  return codeBlock->codeSize;
}

// Kiểm tra xem có phải hàm chuẩn không
int isPredefinedFunction(Object* func) {
  return ((func == readiFunction) || (func == readcFunction));
}

// Kiểm tra xem có phải thủ tục chuẩn không
int isPredefinedProcedure(Object* proc) {
  return ((proc == writeiProcedure) || (proc == writecProcedure) || (proc == writelnProcedure));
}

// Khởi tạo bộ đệm mã lệnh
void initCodeBuffer(void) {
  codeBlock = createCodeBlock(CODE_SIZE);
}

// In toàn bộ mã lệnh ra màn hình (để debug)
void printCodeBuffer(void) {
  printCodeBlock(codeBlock);
}

// Giải phóng bộ nhớ
void cleanCodeBuffer(void) {
  freeCodeBlock(codeBlock);
}

// Ghi mã lệnh ra file nhị phân (để máy ảo chạy)
int serialize(char* fileName) {
  FILE* f;
  f = fopen(fileName, "wb");
  if (f == NULL) return IO_ERROR;
  saveCode(codeBlock, f);
  fclose(f);
  return IO_SUCCESS;
}



//File này chịu trách nhiệm chuyển đổi các thông tin
// từ bảng ký hiệu (Symbol Table) thành các lệnh mã máy ảo (Assembly).