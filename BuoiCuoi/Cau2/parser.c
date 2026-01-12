void compileAssignSt(void) {
  // Biến đếm số lượng biến vế trái và biểu thức vế phải
  int varCount = 0;
  int expCount = 0;

  // --- PHẦN 1: QUÉT VẾ TRÁI (DANH SÁCH BIẾN) ---
  while (1) {
      // 1. Phải là một định danh (tên biến)
      if (lookAhead->tokenType == TK_IDENT) {
          // Ăn token biến (ở đây bạn có thể thêm logic kiểm tra biến đã khai báo chưa)
          eat(TK_IDENT); 
          varCount++; // Tăng số lượng biến tìm thấy
          
          // 2. Nếu thấy dấu phẩy, tiếp tục vòng lặp để tìm biến tiếp theo
          if (lookAhead->tokenType == SB_COMMA) {
              eat(SB_COMMA);
              continue;
          } else {
              // Không thấy dấu phẩy nữa -> Hết danh sách biến
              break; 
          }
      } else {
          // Nếu không phải biến mà cũng không dừng -> Lỗi
          error(ERR_INVALIDSTATEMENT, "Variable identifier expected");
          break;
      }
  }

  // --- PHẦN 2: QUÉT DẤU GÁN (:=) ---
  eat(SB_ASSIGN);

  // --- PHẦN 3: QUÉT VẾ PHẢI (DANH SÁCH BIỂU THỨC) ---
  while (1) {
      // Phân tích biểu thức (logic có sẵn của chương trình dịch)
      compileExpression();
      expCount++; // Tăng số lượng biểu thức tìm thấy

      // Nếu thấy dấu phẩy, tiếp tục tìm biểu thức tiếp theo
      if (lookAhead->tokenType == SB_COMMA) {
          eat(SB_COMMA);
          continue;
      } else {
          break; // Hết danh sách biểu thức
      }
  }

  // --- PHẦN 4: KIỂM TRA NGỮ NGHĨA ---
  // Số lượng biến bên trái phải bằng số lượng giá trị bên phải
  if (varCount != expCount) {
      error(ERR_INVALIDSTATEMENT, "Number of variables and expressions must match");
  }
}