Red [
  Title:   "Grammar parser driver"
  Author:  "Przemyslaw Delewski"
]    

#include %grammar-parser.red

internal-repr: make block![]

print parse-grammar {
  start -> expr | statement;
  expr -> expr '+' expr | expr '*' expr ;
  statement -> '1';
} internal-repr

print mold internal-repr

internal-repr: make block![]

print parse-grammar {
  start -> statement | expr;

  expr-> '1' | '2';

  statement -> 0;

} internal-repr

print mold internal-repr
