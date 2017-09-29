Red [
  Title:  "LR library - Bottom Up parsing engine"
  Author: "Przemyslaw Delewski"
]

; item represent LR0 item
; LR0 item is a grammar rule
; and dot position
; Grammar rule is identified by two things
; ruleLHS - a key in hashtable
; and ruleRHSIndex - right hand side of rule can contain 
; several alternatives
item: make object! [
  ruleLHS:
  ruleRHSIndex:
  dotPosition: none
]

; state collection object
; contains mappings between itemSet and ids
; and ids and item sets
StateCollection: make object! [
  itemSetIds: make hash![]
  idToItemSets: make hash![]
  setId: make integer! 1
]

; return wether itemSet exists already in stateCollection
checkItemSetExists: function [itemSet stateCollection] [
  ; there is convesion from object to string due to problem
  ; with searching via object
  ; actually I don't know if that's bug
  ; or real limitation or my lack of knowledge how to 
  ; use it correcly
  return select stateCollection/itemSetIds to-string itemSet
]

; check wether item set identified by id exists already in stateCollection
checkItemSetExistsById: function [id stateCollection] [
  return select stateCollection/idToItemSets id
]

; adds item set to stateCollection
addItemSet: function [itemSet stateCollection] [
  entry: make block![]
  ; as described in checkItemSetExists
  ; itemSet object is converted to string forth and back
  append/only entry to-string itemSet
  append entry stateCollection/setId
  append stateCollection/itemSetIds entry

  entryIdToItemSet: make block![]
  append entryIdToItemSet stateCollection/setId
  append/only entryIdToItemSet itemSet
  append stateCollection/idToItemSets entryIdToItemSet 

  stateCollection/setId: stateCollection/setId + 1
]

; selects specific grammar rule identified by name from grammar
selectRule: function[grammar name] [
  rule: select grammar name
  return rule 
]

; dumps grammar
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

; adds new rule to grammar
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


; generate all LR0Items from given grammar
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

; closure of LR0 items 
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

; LR0 goto function takes LR0 item set token and grammar
; and return another item set 
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

; get grammar and return all tokens
tokens: function[grammar] [  
  tokenSet: make block![]
  foreach [lhs rhs] grammar [
    append tokenSet lhs
    foreach rule rhs [
      foreach var rule [
        append tokenSet var
      ]
    ]
  ]
  return unique tokenSet
]

; helper function used to build an edge 
; from item set to item set via token
makeEdge: function [from to token] [
  edge: make block![]
  append edge from
  append edge to
  append edge token
  return edge
]


; generate LR canonical collection items
generateLR0ItemsSet: function[mainRule grammar stateCollection] [
  
  edgeSet: make block![]

  mainRuleLhs: first mainRule
  mainRuleRhs: second mainRule
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
 
  addItemSet startItemSet stateCollection
 
  openItems: make block![]

  append/only openItems startItemSet
  
  while [not tail? openItems] [
    itemSet: first openItems
    remove openItems    
    foreach token tokens grammar [
      gotoSet: make block! []
      gotoSet: LR0Goto itemSet token grammar
    
      if empty? gotoSet [continue]

      if none? checkItemSetExists gotoSet stateCollection [
        addItemSet gotoSet stateCollection        
        append/only openItems gotoSet
      ]
      
      from: checkItemSetExists itemSet stateCollection
      to: checkItemSetExists gotoSet stateCollection

      append/only edgeSet makeEdge from to token
    ]    
  ]
  return edgeSet
]

; print all items exist in LR0Items
serializeLR0Items: function [LR0Items grammar] [
  output: make string! ""
  foreach item LR0Items [
    append output item/ruleLHS
    append output " -> "
    rule: select grammar item/ruleLHS
    rhs: pick rule item/ruleRHSIndex
    dotPos: make integer! 1
    foreach variable rhs [
      if dotPos == item/dotPosition [
        append output "."
      ]
      either empty? variable [append output " epsilon "] 
      [ 
        append output  " "
        append output variable
        append output " "
      ]
      dotPos: dotPos + 1
    ]

    ; condition for final item
    if dotPos == item/dotPosition [ append output "." ]

    dotPos: 1
    append output "^/"
  ]
  return output
]

; generate graphviz dot graph representation
; by taking grammar stateCollection and edgeSet
generateDot: function [grammar stateCollection edgeSet] [
  output: make string! ""
  append output "digraph grammar {^/"
  append output "bgcolor=transparent; ^/"
  append output "node [color=lightblue,style=filled fontname = ^"font-fixed^" fontsize=11]; ^/"
  foreach edge edgeSet [
    from: first edge stateCollection
    to: second edge stateCollection
    fromItemSet: checkItemSetExistsById first edge stateCollection
    toItemSet: checkItemSetExistsById second edge stateCollection
    token: ""
    append output from grammar 
    append output "->"
    append output to grammar 
    append output " [label="
    append output "^""
    append output third edge
    append output "^""
    append output " color=lightblue fontcolor=white fontname = ^"font-fixed^" fontsize=11 "
    append output "]^/"
    append output "^/"
    append output from
    append output " [label=^""
    append output serializeLR0Items fromItemSet grammar
    append output "^" "
    append output "color=lightblue ]"
    append output "^/"
    append output to
    append output " [label=^""
    append output serializeLR0Items toItemSet grammar
    append output "^" "
    append output "color=lightblue ]"
    append output "^/"
  ]
  append output "}^/"
  return output
]

test1: function[] [
  testGrammar: [
    "S" [["E"]] 
    "E" [["E" "+" "E"]["1"]]
  ]

  addRule testGrammar "A" ["0"]
  addRule testGrammar "A" ["1"]
  addRule testGrammar "A" [""]
  addRule testGrammar "E" ["E" "*" "E"]
  addRule testGrammar "E" ["E" "/" "E"]

  printGrammar testGrammar

]

test2: function[] [
  grammar: make block![
    "S" [["E"]] 
    "E" [["E" "+" "E"]["1"]]
  ]
  LR0Items: generateLR0Items grammar
  ;printLR0Items LR0Items grammar
  ;print mold LR0Items
  LR0Items1: []
  append LR0Items1 make item [ ruleLHS: "E" ruleRHSIndex: 2 dotPosition: 1 ]
  LR0Result: LR0Closure LR0Items1 grammar
  print "LR0closure:"
  printLR0Items LR0Result grammar
  print "LR0goto:"
  ;print mold LR0Items1
  gotoResult: LR0Goto LR0Items1 "0" grammar 
  printLR0Items gotoResult grammar
  ;print mold stateCollection
  ;addItemSet LR0Result stateCollection
  ;addItemSet LR0Result stateCollection
  ;print mold stateCollection
  ;print checkItemSetExists LR0Result stateCollection
  ;print mold checkItemSetExistsById 1 stateCollection  
]

test1
test2 

