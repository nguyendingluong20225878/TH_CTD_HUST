/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include "instructions.h"

#define MAX_BLOCK 50 // Kích thước khối đọc ghi file

// Tạo một khối chứa mã lệnh (CodeBlock) với kích thước tối đa
CodeBlock* createCodeBlock(int maxSize) {
  CodeBlock* codeBlock = (CodeBlock*) malloc(sizeof(CodeBlock));
  // Cấp phát mảng lệnh
  codeBlock->code = (Instruction*) malloc(maxSize * sizeof(Instruction));
  codeBlock->codeSize = 0; // Chưa có lệnh nào
  codeBlock->maxSize = maxSize;
  return codeBlock;
}

// Giải phóng bộ nhớ của CodeBlock
void freeCodeBlock(CodeBlock* codeBlock) {
  free(codeBlock->code);
  free(codeBlock);
}

// Hàm cốt lõi: Thêm một lệnh vào cuối danh sách lệnh
int emitCode(CodeBlock* codeBlock, enum OpCode op, WORD p, WORD q) {
  Instruction* bottom = codeBlock->code + codeBlock->codeSize; // Lấy vị trí trống tiếp theo

  if (codeBlock->codeSize >= codeBlock->maxSize) return 0; // Tràn bộ nhớ

  bottom->op = op; // Mã lệnh (OpCode)
  bottom->p = p;   // Tham số 1 (ví dụ: level)
  bottom->q = q;   // Tham số 2 (ví dụ: offset hoặc giá trị)
  codeBlock->codeSize ++;
  return 1;
}

// Các hàm wrapper để sinh từng loại lệnh cụ thể (gọi emitCode)
// DC_VALUE thường là 0 (Don't Care value) khi tham số đó không được dùng
int emitLA(CodeBlock* codeBlock, WORD p, WORD q) { return emitCode(codeBlock, OP_LA, p, q); }
int emitLV(CodeBlock* codeBlock, WORD p, WORD q) { return emitCode(codeBlock, OP_LV, p, q); }
int emitLC(CodeBlock* codeBlock, WORD q) { return emitCode(codeBlock, OP_LC, DC_VALUE, q); }
int emitLI(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_LI, DC_VALUE, DC_VALUE); }
int emitINT(CodeBlock* codeBlock, WORD q) { return emitCode(codeBlock, OP_INT, DC_VALUE, q); }
int emitDCT(CodeBlock* codeBlock, WORD q) { return emitCode(codeBlock, OP_DCT, DC_VALUE, q); }
int emitJ(CodeBlock* codeBlock, WORD q) { return emitCode(codeBlock, OP_J, DC_VALUE, q); }
int emitFJ(CodeBlock* codeBlock, WORD q) { return emitCode(codeBlock, OP_FJ, DC_VALUE, q); }
int emitHL(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_HL, DC_VALUE, DC_VALUE); }
int emitST(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_ST, DC_VALUE, DC_VALUE); }
int emitCALL(CodeBlock* codeBlock, WORD p, WORD q) { return emitCode(codeBlock, OP_CALL, p, q); }
int emitEP(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_EP, DC_VALUE, DC_VALUE); }
int emitEF(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_EF, DC_VALUE, DC_VALUE); }
// ... (Các lệnh tính toán số học tương tự, không dùng tham số p, q)
int emitRC(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_RC, DC_VALUE, DC_VALUE); }
int emitRI(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_RI, DC_VALUE, DC_VALUE); }
int emitWRC(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_WRC, DC_VALUE, DC_VALUE); }
int emitWRI(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_WRI, DC_VALUE, DC_VALUE); }
int emitWLN(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_WLN, DC_VALUE, DC_VALUE); }
int emitAD(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_AD, DC_VALUE, DC_VALUE); }
int emitSB(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_SB, DC_VALUE, DC_VALUE); }
int emitML(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_ML, DC_VALUE, DC_VALUE); }
int emitDV(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_DV, DC_VALUE, DC_VALUE); }
int emitNEG(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_NEG, DC_VALUE, DC_VALUE); }
int emitCV(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_CV, DC_VALUE, DC_VALUE); }
int emitEQ(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_EQ, DC_VALUE, DC_VALUE); }
int emitNE(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_NE, DC_VALUE, DC_VALUE); }
int emitGT(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_GT, DC_VALUE, DC_VALUE); }
int emitLT(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_LT, DC_VALUE, DC_VALUE); }
int emitGE(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_GE, DC_VALUE, DC_VALUE); }
int emitLE(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_LE, DC_VALUE, DC_VALUE); }

int emitBP(CodeBlock* codeBlock) { return emitCode(codeBlock, OP_BP, DC_VALUE, DC_VALUE); }

// Hàm in một lệnh đơn ra màn hình (dạng hợp ngữ - Assembly mnemonic)
void printInstruction(Instruction* inst) {
  switch (inst->op) {
  case OP_LA: printf("LA %d,%d", inst->p, inst->q); break; // Load Address level, offset
  case OP_LV: printf("LV %d,%d", inst->p, inst->q); break; // Load Value level, offset
  case OP_LC: printf("LC %d", inst->q); break;             // Load Constant value
  case OP_LI: printf("LI"); break;                         // Load Indirect
  case OP_INT: printf("INT %d", inst->q); break;           // Increment Stack Pointer
  case OP_DCT: printf("DCT %d", inst->q); break;           // Decrement Stack Pointer
  case OP_J: printf("J %d", inst->q); break;               // Jump to address
  case OP_FJ: printf("FJ %d", inst->q); break;             // False Jump to address
  case OP_HL: printf("HL"); break;                         // Halt
  case OP_ST: printf("ST"); break;                         // Store
  case OP_CALL: printf("CALL %d,%d", inst->p, inst->q); break; // Call procedure
  case OP_EP: printf("EP"); break;                         // Exit Procedure
  case OP_EF: printf("EF"); break;                         // Exit Function
  // ... Các lệnh in/nhập/tính toán
  case OP_RC: printf("RC"); break;
  case OP_RI: printf("RI"); break;
  case OP_WRC: printf("WRC"); break;
  case OP_WRI: printf("WRI"); break;
  case OP_WLN: printf("WLN"); break;
  case OP_AD: printf("AD"); break;
  case OP_SB: printf("SB"); break;
  case OP_ML: printf("ML"); break;
  case OP_DV: printf("DV"); break;
  case OP_NEG: printf("NEG"); break;
  case OP_CV: printf("CV"); break;
  case OP_EQ: printf("EQ"); break;
  case OP_NE: printf("NE"); break;
  case OP_GT: printf("GT"); break;
  case OP_LT: printf("LT"); break;
  case OP_GE: printf("GE"); break;
  case OP_LE: printf("LE"); break;
  case OP_BP: printf("BP"); break; // Break Point
  default: break;
  }
}

// Hàm duyệt và in toàn bộ khối lệnh
void printCodeBlock(CodeBlock* codeBlock) {
  Instruction* pc = codeBlock->code;
  int i;
  for (i = 0 ; i < codeBlock->codeSize; i ++) {
    printf("%d:  ",i); // In số thứ tự dòng lệnh (Instruction Pointer)
    printInstruction(pc);
    printf("\n");
    pc ++;
  }
}

// Đọc mã lệnh từ file vào bộ nhớ
void loadCode(CodeBlock* codeBlock, FILE* f) {
  Instruction* code = codeBlock->code;
  int n;
  codeBlock->codeSize = 0;
  while (!feof(f)) {
    n = fread(code, sizeof(Instruction), MAX_BLOCK, f);
    code += n;
    codeBlock->codeSize += n;
  }
}

// Lưu mã lệnh từ bộ nhớ ra file
void saveCode(CodeBlock* codeBlock, FILE* f) {
  fwrite(codeBlock->code, sizeof(Instruction), codeBlock->codeSize, f);
}

//File này định nghĩa cách tạo ra một lệnh, cách lưu trữ và in ấn các lệnh đó. Đây là phần "Backend" thấp nhất.