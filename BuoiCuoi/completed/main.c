/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "parser.h"
#include "codegen.h"

// Biến cờ (flag) để kiểm tra xem người dùng có muốn in mã máy ra màn hình không
int dumpCode = 0;

// Hàm in hướng dẫn sử dụng chương trình
void printUsage(void) {
  printf("Usage: kplc input output [-dump]\n");
  printf("   input: input kpl program\n");
  printf("   output: executable\n");
  printf("   -dump: code dump\n");
}

// Hàm phân tích tham số dòng lệnh (để tìm cờ -dump)
int analyseParam(char* param) {
  if (strcmp(param, "-dump") == 0) {
    dumpCode = 1;
    return 1;
  } 
  return 0;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  int i; 

  // Kiểm tra số lượng tham số đầu vào. Phải có ít nhất tên file input.
  if (argc <= 1) {
    printf("kplc: no input file.\n");
    printUsage();
    return -1;
  }

  // Phải có tên file output.
  if (argc <= 2) {
    printf("kplc: no output file.\n");
    printUsage();
    return -1;
  }

  // Kiểm tra các tham số tùy chọn (như -dump)
  for ( i = 3; i < argc; i ++) 
    analyseParam(argv[i]);

  // Khởi tạo bộ đệm chứa mã lệnh (Code Buffer) - xem codegen.c
  initCodeBuffer();

  // Bắt đầu quá trình biên dịch (gọi hàm compile trong parser.c)
  // Nếu trả về IO_ERROR nghĩa là không đọc được file
  if (compile(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }

  // Sau khi biên dịch xong, ghi mã máy ra file nhị phân (argv[2])
  if (serialize(argv[2]) == IO_ERROR) {
    printf("Can\'t write output file!\n");
    return -1;
  }

  // Nếu người dùng có thêm tham số -dump, in mã lệnh ra màn hình để kiểm tra
  if (dumpCode) printCodeBuffer();
    
  // Giải phóng bộ nhớ
  cleanCodeBuffer();

  return 0;
}