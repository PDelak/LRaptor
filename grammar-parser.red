Red [
  Title:   "Basic grammar parser"
  Author:  "Przemyslaw Delewski"
]    

parse-grammar: function[txt internal-repr] [
  grammar-internal-repr: make block![]
  new-rule: make block![]
  new-set-rules: make block![]

  digit:   charset "0123456789"
  letters: charset [#"a" - #"z" #"A" - #"Z"]
  delimiter: charset [#"^"" #"'"]
  special: charset "+-/*!@^#^^$%&^(^)^"^{^}"
  chars:   union letters digit
  word:    [some chars]
  string: [delimiter some [word | special] delimiter]
  ws: charset reduce [space tab]
  new-line: charset reduce [cr lf]
  word-or-string: [any ws [word | string]]
  words: [some word-or-string]
  rhs-rule: [
       [copy value words 
         (
           words-bl: split value #" "
           foreach w words-bl [ append new-rule w ]
           append/only new-set-rules new-rule 
         ) 
         any ws  "|" any ws (  new-rule: make block![]) rhs-rule] 
       | [words] 
  ]

  lhs-rule: [copy lhs-value word (append grammar-internal-repr lhs-value) any ws "->" any ws]

  rule: [(new-set-rules: make block![]
          new-rule: make block![])    
         lhs-rule     
         rhs-rule 
         (append/only grammar-internal-repr new-set-rules)        
         any ws
         ";"
         any new-line
         any ws
         
  ]

  grammar: [any [new-line | ws] [some rule] any new-line]

  success: parse txt grammar
  append internal-repr grammar-internal-repr
  return success
]
