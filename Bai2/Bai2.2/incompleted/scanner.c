/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 * * CHỈNH SỬA ĐỂ HỖ TRỢ: STRING, BYTES, // COMMENT, %, **, REPEAT, UNTIL
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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

void skipBlank() {
  while (charCodes[currentChar] == CHAR_SPACE)
    readChar();
}

/**
 * @brief Bỏ qua comment kiểu Pascal (* ... *).
 */
void skipComment() {
  readChar(); // Đọc ký tự sau '(' (là '*')
  readChar(); // Đọc ký tự nội dung comment đầu tiên

  while (currentChar != EOF) {
    if (charCodes[currentChar] == CHAR_TIMES) { // Gặp '*'
      readChar(); // Đọc ký tự sau '*'

      if (charCodes[currentChar] == CHAR_RPAR) { // Nếu ký tự sau '*' là ')'
        readChar(); // Đọc ký tự sau '*)' (Kết thúc comment)
        return;
      }
    } else {
      readChar(); // Đọc ký tự bình thường tiếp theo
    }
  }
  
  error(ERR_ENDOFCOMMENT, lineNo, colNo);
}

/**
 * @brief Bỏ qua comment kiểu C++ (// đến cuối dòng).
 * Giả định rằng CHAR_SLASH đại diện cho '/'.
 */
void skipLineComment() {
    readChar(); // Đã đọc '/' đầu tiên ở getToken, giờ đọc '/' thứ hai
    
    while (currentChar != EOF && currentChar != '\n') {
        readChar();
    }
    // Nếu currentChar là '\n', readChar() tiếp theo sẽ bắt đầu dòng mới. 
    // Không cần gọi readChar() ở đây vì getToken sẽ gọi skipBlank nếu gặp '\n' (CHAR_SPACE).
}

Token* readIdentKeyword(void) {
  int count = 0;
  int startLine = lineNo; 
  int startCol = colNo;

  Token* token = makeToken(TK_IDENT, startLine, startCol);

  while (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT) {
    
    if (count < MAX_IDENT_LEN) {
      token->string[count] = currentChar;
    }
    
    count++;
    readChar();
  }

  if (count > MAX_IDENT_LEN) {
      token->string[MAX_IDENT_LEN] = '\0';
  } else {
      token->string[count] = '\0';
  }

  if (count > MAX_IDENT_LEN) {
    error(ERR_IDENTTOOLONG, startLine, startCol);
  } 
  
  // Kiểm tra Từ khóa (bao gồm REPEAT, UNTIL, BYTES, STRING mới)
  TokenType type = checkKeyword(token->string);

  if (type != TK_NONE) {
    token->tokenType = type;
  }
  
  return token;
}

Token* readNumber(void) {
  int count = 0;
  long long val = 0;
  
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

    if (count < MAX_IDENT_LEN) { 
        token->string[count] = currentChar;
    } 
    
    count++;
    readChar();
  }
  
  if (count > MAX_IDENT_LEN) {
      token->string[MAX_IDENT_LEN] = '\0';
  } else {
      token->string[count] = '\0';
  }

  if (numberIsTooLong) {
      token->tokenType = TK_NONE; 
      error(ERR_IDENTTOOLONG, startLine, startCol); 
  } else {
    token->value = (int)val; 
  }
  
  return token;
}

Token* readConstChar(void) {
  int startLine = lineNo; 
  int startCol = colNo;
  Token *token = makeToken(TK_CHAR, startLine, startCol);

  readChar(); // Đọc qua dấu nháy đơn mở đầu (')
  
  if (currentChar == EOF) { 
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, startLine, startCol);
    return token;
  }
    
  token->string[0] = currentChar;
  token->string[1] = '\0';

  readChar(); // Đọc ký tự sau ký tự nội dung
  
  if (currentChar == EOF) { 
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, startLine, startCol);
    return token;
  }

  if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    readChar(); // Đọc qua dấu nháy đơn kết thúc
    return token;
  } else {
    token->tokenType = TK_NONE;
    error(ERR_INVALIDCHARCONSTANT, startLine, startCol);
    return token;
  }
}

/**
 * @brief Đọc hằng số chuỗi có định dạng "string".
 * Giả định CHAR_DOUBLEQUOTE đã được định nghĩa.
 * @return Con trỏ tới Token (TK_STRING).
 */
Token* readString(void) {
    // Giả định CHAR_DOUBLEQUOTE là mã ký tự cho "
    int startLine = lineNo; 
    int startCol = colNo;
    Token *token = makeToken(TK_STRING, startLine, startCol);
    int count = 0;
    
    readChar(); // Bỏ qua dấu nháy kép mở đầu (")
    
    while (currentChar != EOF && charCodes[currentChar] != CHAR_DOUBLEQUOTE) {
        if (count < MAX_IDENT_LEN) { // Tái sử dụng MAX_IDENT_LEN làm giới hạn độ dài chuỗi
            token->string[count] = currentChar;
            count++;
        } else {
            // Xử lý chuỗi quá dài (giả định cắt và báo lỗi nếu cần)
        }
        readChar();
    }
    
    token->string[count] = '\0';
    
    if (currentChar == EOF) {
        token->tokenType = TK_NONE;
        error(ERR_INVALIDCHARCONSTANT, startLine, startCol); // Tái sử dụng mã lỗi
        return token;
    }
    
    readChar(); // Bỏ qua dấu nháy kép kết thúc (")
    return token;
}


Token* getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  ln = lineNo; 
  cn = colNo;

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: 
    skipBlank(); 
    return getToken();
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
    
  // Xử lý Times ('*') và Power ('**')
  case CHAR_TIMES: 
    readChar();
    if (charCodes[currentChar] == CHAR_TIMES) { // Gặp '**'
        token = makeToken(SB_POWER, ln, cn); // SB_POWER mới
        readChar();
        return token;
    } else {
        token = makeToken(SB_TIMES, ln, cn);
        return token;
    }
    
  // Xử lý Slash ('/') và Comment dòng ('//')
  case CHAR_SLASH: 
    readChar();
    if (charCodes[currentChar] == CHAR_SLASH) { // Gặp '//'
        skipLineComment();
        return getToken();
    } else {
        token = makeToken(SB_SLASH, ln, cn);
        return token;
    }
    
  // Xử lý Modulo ('%')
  case CHAR_PERCENT: // Giả định CHAR_PERCENT là '%'
    token = makeToken(SB_MOD, ln, cn); // SB_MOD mới
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
    
  // Xử lý Hằng số Chuỗi (Giả định CHAR_DOUBLEQUOTE là mã ký tự cho '"')
  case CHAR_DOUBLEQUOTE:
    return readString();

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

  case CHAR_LT:
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) { // Gặp '<='
      token = makeToken(SB_LE, ln, cn);
      readChar();
      return token;
    } else if (charCodes[currentChar] == CHAR_GT) { // Gặp '<>'
      token = makeToken(SB_NEQ, ln, cn);
      readChar();
      return token;
    } else {
      token = makeToken(SB_LT, ln, cn);
      return token;
    }

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
    
  case CHAR_LPAR:
    readChar();
    if (charCodes[currentChar] == CHAR_TIMES) { // Gặp '(*'
      skipComment();
      return getToken();
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
    readChar();
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
  
  // THÊM: TK_STRING và TK_BYTES
  case TK_STRING: printf("TK_STRING(\"%s\")\n", token->string); break; 
  case TK_BYTES: printf("TK_BYTES\n"); break; 

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
  // THÊM: REPEAT và UNTIL
  case KW_REPEAT: printf("KW_REPEAT\n"); break; 
  case KW_UNTIL: printf("KW_UNTIL\n"); break; 

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
  // THÊM: MOD và POWER
  case SB_MOD: printf("SB_MOD (%%)\n"); break; 
  case SB_POWER: printf("SB_POWER (**)\n"); break; 
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

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