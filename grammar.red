Red []

grammar: [
  "S" [["E"]] 
  "E" [["B" "+" "B"]]
  "B" [["1"]]
]

item: make object! [
  ruleLHS:
  ruleRHSIndex:
  dotPosition: none
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
      foreach varElem variable [
        either empty? varElem [prin "epsilon"] [prin " " prin varElem prin " "]
      ]
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

{
  generate all LR0Items from given grammar
}
generateLR0Items: function[grammar] [
  LR0Items: []

  foreach [lhs rhs] grammar [
    lhs: lhs
    rhs: rhs
    ruleIndex: make integer! 1
    foreach rhsBlock rhs [
      dotPos: make Integer! 1
      foreach rhsElem rhsBlock [
        ; add item except final one
        append LR0Items make item [ ruleLHS: lhs ruleRHSIndex: ruleIndex dotPosition: dotPos ]
        dotPos: dotPos + 1
      ]
      ; add final item
      append LR0Items make item [ ruleLHS: lhs ruleRHSIndex: ruleIndex dotPosition: dotPos ]
      ruleIndex: ruleIndex + 1
    ]
    
  ]
  return LR0Items
]

; print all items exist in LR0Items
printLR0Items: function [LR0Items grammar] [
  foreach item LR0Items [
    prin item/ruleLHS
    prin " -> "
    rule: select grammar item/ruleLHS
    rhs: pick rule item/ruleRHSIndex
    dotPos: make integer! 1
    foreach variable rhs [
      if dotPos == item/dotPosition [
        prin "."
      ]
      either empty? variable [prin " epsilon "] 
      [ 
        prin " "
        prin variable
        prin " "
      ]
      dotPos: dotPos + 1
    ]

    ; condition for final item
    if dotPos == item/dotPosition [ prin "." ]

    dotPos: 1
    print ""
  ]
]

LR0Closure: function [LR0Items grammar] [
  
  openSet: make block![]
  resultSet: make block![]

  append openSet LR0Items
  
  while [not tail? openSet] [        
  
    item: first openSet    
    append resultSet item
    remove openSet
        
    rhs: select grammar item/ruleLHS 
    if empty? rhs [continue]
        
    rhsRule: pick rhs item/ruleRHSIndex
    if empty? rhsRule [ continue ]

    variable: pick rhsRule item/dotPosition

    if empty? variable [ continue ]

    nextProduction: select grammar variable
  
    if empty? nextProduction [continue]

    repeat index length? nextProduction [
      newItem: make item [ruleLHS: variable ruleRHSIndex: index dotPosition: 1]
      if not empty? find head openSet newItem [ continue ]
      
      if not empty? find head resultSet newItem [ continue ]
   
      append openSet newItem
    ]
  ]

  return resultSet
]

LR0Goto: function[LR0Items token grammar] [
  gotoSet: make block![]
  foreach item LR0Items [
    rhs: select grammar item/ruleLHS
    rule: pick rhs item/ruleRHSIndex
    ruleLen: length? rule
    if ruleLen < item/dotPosition [continue]
    nextToken: pick rule item/dotPosition
    if equal? nextToken token [
       newItem: make item [ruleLHS: item/ruleLHS ruleRHSIndex: item/ruleRHSIndex dotPosition: item/dotPosition + 1]
       append gotoSet newItem
    ]
  ]
  return LR0Closure gotoSet grammar
]

{
  generate LR canonical collection items
}
generateLR0ItemsSet: function[mainRule grammar stateCollection] [
  
  mainRuleLhs: first mainRule
  mainRuleRhs: second mainRule
  print mold mainRuleRhs
  rhs: select grammar mainRuleLhs
  index: make integer! 1
  foreach e rhs [
    if equal? e mainRuleRhs [ break ]
    index: index + 1
  ]  
  initialItem: make item [ruleLHS: mainRuleLhs ruleRHSIndex: index dotPosition: 1]
  
  initialSet: make block![]
  append initialSet initialItem
  startItemSet: LR0Closure initialSet grammar
  
  printLR0Items startItemSet grammar
]

addRule grammar "A" ["0"]
addRule grammar "A" ["1"]
addRule grammar "A" [""]
addRule grammar "E" ["E" "*" "E"]
addRule grammar "E" ["E" "/" "E"]

printGrammar grammar

LR0Items: generateLR0Items grammar

;printLR0Items LR0Items grammar

;print mold LR0Items

LR0Items1: []

append LR0Items1 make item [ ruleLHS: "A" ruleRHSIndex: 1 dotPosition: 1 ]

LR0Result: LR0Closure LR0Items1 grammar

print "LR0closure:"
printLR0Items LR0Result grammar

print "LR0goto:"
gotoResult: LR0Goto LR0Items1 "0" grammar 

printLR0Items gotoResult grammar

stateCollection: []
generateLR0ItemsSet ["E" ["E" "*" "E"]] grammar stateCollection
