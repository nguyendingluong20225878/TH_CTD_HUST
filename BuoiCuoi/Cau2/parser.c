// parser.c
//Sửa parser.c Thêm hàm xử lý compileRepeatSt và cập nhật compileStatement.
// Khai báo prototype ở đầu file hoặc file parser.h
void compileRepeatSt(void);

// Cài đặt hàm
void compileRepeatSt(void) {
  eat(KW_REPEAT);
  
  // Lưu nhãn vị trí bắt đầu vòng lặp để quay lại
  // CodeAddress startLabel = getCurrentCodeAddress(); (Dùng cho phần sinh mã)

  // Phân tích thân vòng lặp (cho phép nhiều lệnh)
  compileStatement(); 
  while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileStatement();
  }

  eat(KW_UNTIL);
  compileCondition(); // Phân tích điều kiện dừng
  
  // Sinh mã nhảy ngược lại nếu điều kiện sai (tùy logic sinh mã)
  // genFJ(startLabel); 
}

// Cập nhật compileStatement
void compileStatement(void) {
  switch (lookAhead->tokenType) {
    // ... các case cũ ...
    case KW_REPEAT:     // Thêm case này
      compileRepeatSt();
      break;
    // ...
  }
}