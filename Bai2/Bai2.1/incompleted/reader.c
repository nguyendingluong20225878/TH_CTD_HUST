/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include "reader.h"

FILE *inputStream;/*giữ luồng đầu vào */
int lineNo, colNo;/*lineNo: dòng htai, colNo: cột htai*/
int currentChar;/*kí tự vừa đọc*/

int readChar(void) {/*đọc*/
  currentChar = getc(inputStream);/*đọc kí tụ tiếp theo*/
  colNo ++;/*tăng cột*/
  if (currentChar == '\n') {/*kiểm tra kí tự xuống dòng*/
    lineNo ++;
    colNo = 0;
  }
  return currentChar;
}

int openInputStream(char *fileName) {
  inputStream = fopen(fileName, "rt");/*read + text mode*/
  if (inputStream == NULL)
    return IO_ERROR;
  lineNo = 1;
  colNo = 0;
  readChar();
  return IO_SUCCESS;
}

void closeInputStream() {
  fclose(inputStream);
}

