#include "location.h"
#include "parser.tab.h"

Location::Location(YYLTYPE yylloc)
{
  start_line_ = yylloc.first_line;
  start_col_ = yylloc.first_column;
  end_line_ = yylloc.last_line;
  end_col_ = yylloc.last_column;
}
