Red []

grammar: [
  "S" [["E"]] "E" [["E" "+" "B"]]
  "B" [["1"]]
]

selectRule: function[grammar name] [
  rule: select grammar name
  return rule 
]

printGrammar: function [grammar] [
  foreach [lhs rhs] grammar [
    prin [lhs]
    prin " -> "
    index: make integer! 0

    foreach variable rhs [
      if index > 0 [ prin " | " ]
      either empty? variable [ prin "epsilon" ] [ prin variable ]

      index: index + 1
    ]
   print ""
  ]
]

addRule: function[grammar name rhs][    
  presentRule: selectRule grammar name
  either not empty? presentRule [
    append/only presentRule rhs
  ] [
    rule: []
    rhsBlock: []
    append/only rhsBlock rhs
    append rule name
    append/only rule rhsBlock
    append grammar rule
  ]
]


addRule grammar "A" ["0"]
addRule grammar "A" ["1"]
addRule grammar "E" ["E" "*" "E"]
addRule grammar "E" ["E" "/" "E"]
printGrammar grammar