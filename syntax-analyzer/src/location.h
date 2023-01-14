#pragma once

struct YYLTYPE;

class Location {
public:
  Location(YYLTYPE *yylloc);

  int start_row() { return start_line_; }
  int start_col() { return start_col_; }

  int end_row() { return end_line_; }
  int end_col() { return end_col_; }

private:
  int start_line_;
  int start_col_;

  int end_line_;
  int end_col_;
};
