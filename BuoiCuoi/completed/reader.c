/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include "reader.h"

FILE *inputStream; // Con trỏ file đầu vào
int lineNo, colNo; // Biến toàn cục lưu dòng và cột hiện tại
int currentChar;   // Ký tự hiện tại đang đọc

// Hàm đọc một ký tự từ luồng nhập
int readChar(void) {
  currentChar = getc(inputStream); // Đọc 1 ký tự
  colNo ++; // Tăng số cột
  if (currentChar == '\n') { // Nếu gặp ký tự xuống dòng
    lineNo ++; // Tăng số dòng
    colNo = 0; // Reset số cột về 0
  }
  return currentChar; // Trả về ký tự vừa đọc
}

// Hàm mở file mã nguồn để đọc
int openInputStream(char *fileName) {
  inputStream = fopen(fileName, "rt"); // Mở file chế độ read text
  if (inputStream == NULL)
    return IO_ERROR;
  lineNo = 1;
  colNo = 0;
  readChar(); // Đọc ngay ký tự đầu tiên để sẵn sàng
  return IO_SUCCESS;
}

// Hàm đóng file
void closeInputStream() {
  fclose(inputStream);
}


//File này chịu trách nhiệm đọc từng ký tự từ file nguồn,
// đồng thời theo dõi số dòng (lineNo) và số cột (colNo) để báo lỗi chính xác vị trí.