/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // Cần thiết cho INT_MAX để kiểm tra tràn số

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


// Khai báo các biến toàn cục từ các module khác
extern int lineNo;
extern int colNo;
extern int currentChar;
extern CharCode charCodes[];

/***************************************************************/

/**
 * @brief Bỏ qua các ký tự khoảng trắng liên tiếp.
 */
void skipBlank() {
  while (charCodes[currentChar] == CHAR_SPACE)
    readChar();
}

void skipComment() {
  // Bỏ qua ký tự mở đầu '(*'
  readChar(); // Đọc ký tự sau '(' (là '*')
  readChar(); // Đọc ký tự nội dung comment đầu tiên

  while (currentChar != EOF) {
    if (charCodes[currentChar] == CHAR_TIMES) { // Gặp '*'
      readChar(); // Đọc ký tự sau '*'

      if (charCodes[currentChar] == CHAR_RPAR) { // Nếu ký tự sau '*' là ')'
        readChar(); // Đọc ký tự sau '*)' (Kết thúc comment)
        return;
      }
      // Nếu không phải ')', tiếp tục vòng lặp
    } else {
      readChar(); // Đọc ký tự bình thường tiếp theo
    }
  }
  
  // Gặp EOF mà chưa kết thúc comment
  error(ERR_ENDOFCOMMENT, lineNo, colNo);
}

Token* readIdentKeyword(void) {
  int count = 0;
  // Lưu vị trí BẮT ĐẦU của token
  int startLine = lineNo; 
  int startCol = colNo;

  Token* token = makeToken(TK_IDENT, startLine, startCol);

  while (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT) {
    
    // Chỉ lưu ký tự vào chuỗi nếu chưa vượt quá giới hạn (tránh tràn bộ đệm)
    if (count < MAX_IDENT_LEN) {
      token->string[count] = currentChar;
    }
    
    count++; // Luôn đếm độ dài thực
    readChar();
  }

  // Kết thúc chuỗi (chuỗi có thể bị cắt)
  if (count > MAX_IDENT_LEN) {
      token->string[MAX_IDENT_LEN] = '\0';
  } else {
      token->string[count] = '\0';
  }

  // Báo lỗi nếu quá dài
  if (count > MAX_IDENT_LEN) {
    error(ERR_IDENTTOOLONG, startLine, startCol);
  } 
  
  // Kiểm tra Từ khóa
  TokenType type = checkKeyword(token->string);

  // Gán loại token
  if (type != TK_NONE) {
    token->tokenType = type;
  }
  
  return token;
}

Token* readNumber(void) {
  int count = 0;
  long long val = 0; // Dùng long long để kiểm tra tràn số int (32-bit)
  
  int startLine = lineNo; 
  int startCol = colNo;
  
  Token* token = makeToken(TK_NUMBER, startLine, startCol);
  int numberIsTooLong = 0;
  
  while (charCodes[currentChar] == CHAR_DIGIT) {
    int digit = currentChar - '0';
    
    // Kiểm tra tràn số (Overflow Check)
    if (val > (INT_MAX / 10) || (val == (INT_MAX / 10) && digit > (INT_MAX % 10))) {
        numberIsTooLong = 1;
    } else {
        val = val * 10 + digit;
    }

    // Xử lý lưu chuỗi (giả sử dùng MAX_IDENT_LEN là giới hạn chuỗi)
    if (count < MAX_IDENT_LEN) { 
        token->string[count] = currentChar;
    } 
    
    count++; // Luôn đếm độ dài thực
    readChar();
  }
  
  // Kết thúc chuỗi
  if (count > MAX_IDENT_LEN) {
      token->string[MAX_IDENT_LEN] = '\0';
  } else {
      token->string[count] = '\0';
  }

  // Xử lý lỗi và gán giá trị
  if (numberIsTooLong) {
      token->tokenType = TK_NONE; 
      error(ERR_IDENTTOOLONG, startLine, startCol); // Dùng ERR_IDENTTOOLONG như lỗi tạm thời cho "number too long"
  } else {
    // Gán giá trị (chỉ gán nếu không xảy ra overflow)
    token->value = (int)val; 
  }
  
  return token;
}


Token* readConstChar(void) {
  // Lưu vị trí BẮT ĐẦU của token (dấu nháy đơn đầu tiên)
  int startLine = lineNo; 
  int startCol = colNo;
  Token *token = makeToken(TK_CHAR, startLine, startCol);

  readChar(); // Đọc qua dấu nháy đơn mở đầu (')
  
  // 1. Kiểm tra EOF sau dấu nháy đơn mở đầu
  if (currentChar == EOF) { 
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, startLine, startCol);
    return token;
  }
    
  // 2. Gán ký tự nội dung
  token->string[0] = currentChar;
  token->string[1] = '\0'; // Luôn kết thúc chuỗi

  readChar(); // Đọc ký tự sau ký tự nội dung
  
  // 3. Kiểm tra EOF sau ký tự nội dung
  if (currentChar == EOF) { 
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, startLine, startCol);
    return token;
  }

  // 4. Kiểm tra ký tự kết thúc (phải là dấu nháy đơn)
  if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    readChar(); // Đọc qua dấu nháy đơn kết thúc
    return token;
  } else {
    // Thiếu dấu nháy đơn đóng
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, startLine, startCol);
    return token;
  }
}


Token* getToken(void) {
  Token *token;
  int ln, cn; // ln, cn: Vị trí BẮT ĐẦU của Token

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  ln = lineNo; 
  cn = colNo; // Lưu vị trí bắt đầu token

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: 
    skipBlank(); 
    return getToken(); // Đệ quy để tìm token tiếp theo
  case CHAR_LETTER: 
    return readIdentKeyword();
  case CHAR_DIGIT: 
    return readNumber();
    
  case CHAR_PLUS: 
    token = makeToken(SB_PLUS, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_MINUS: 
    token = makeToken(SB_MINUS, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_TIMES: 
    token = makeToken(SB_TIMES, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_SLASH: 
    token = makeToken(SB_SLASH, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_EQ: 
    token = makeToken(SB_EQ, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_COMMA: 
    token = makeToken(SB_COMMA, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_PERIOD: 
    token = makeToken(SB_PERIOD, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_SEMICOLON: 
    token = makeToken(SB_SEMICOLON, ln, cn);
    readChar(); 
    return token;
    
  case CHAR_SINGLEQUOTE: 
    return readConstChar();
    
  // Xử lý tổ hợp ký hiệu: Colon (':') và Assign (':=')
  case CHAR_COLON:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) { // Gặp ':='
      token = makeToken(SB_ASSIGN, ln, cn);
      readChar();
      return token;
    } else {
      token = makeToken(SB_COLON, ln, cn);
      return token;
    }

  // Xử lý tổ hợp ký hiệu: LT ('<'), LE ('<='), NEQ ('<>')
  case CHAR_LT:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) { // Gặp '<='
      token = makeToken(SB_LE, ln, cn);
      readChar();
      return token;
    } else if (charCodes[currentChar] == CHAR_GT) { // Gặp '<>' (Không bằng)
      token = makeToken(SB_NEQ, ln, cn);
      readChar();
      return token;
    } else {
      token = makeToken(SB_LT, ln, cn);
      return token;
    }

  // Xử lý tổ hợp ký hiệu: GT ('>'), GE ('>=')
  case CHAR_GT:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) { // Gặp '>='
      token = makeToken(SB_GE, ln, cn);
      readChar();
      return token;
    } else {
      token = makeToken(SB_GT, ln, cn);
      return token;
    }
    
  // Xử lý Dấu ngoặc đơn trái ('(') và Comment mở đầu ('(*')
  case CHAR_LPAR:
    readChar(); // Đọc ký tự tiếp theo
    if (charCodes[currentChar] == CHAR_TIMES) { // Gặp '(*'
      skipComment();
      return getToken(); // Tiếp tục tìm token tiếp theo (đệ quy)
    } else {
      token = makeToken(SB_LPAR, ln, cn);
      return token;
    }
    
  case CHAR_RPAR:
    token = makeToken(SB_RPAR, ln, cn);
    readChar();
    return token;
    
  case CHAR_UNKNOWN:
  default:
    token = makeToken(TK_NONE, ln, cn);
    error(ERR_INVALIDSYMBOL, ln, cn);
    readChar(); // Bỏ qua ký tự lỗi và tiếp tục
    return token;
  }
}


void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%d)\n", token->value); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

/**
 * @brief Chức năng quét chính: mở file, đọc và in tất cả các token.
 * @param fileName Tên file mã nguồn.
 * @return IO_SUCCESS hoặc IO_ERROR.
 */
int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}