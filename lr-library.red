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
  rule-lhs:
  rule-rhs-index:
  dot-position: none
]

; state collection object
; contains mappings between item-set and ids
; and ids and item sets
state-collection: make object! [
  item-set-ids: make hash![]
  id-to-item-sets: make hash![]
  set-id: make integer! 1
]

; return wether item-set exists already in state-collection
check-item-set-exists: function [item-set state-collection] [
  ; there is convesion from object to string due to problem
  ; with searching via object
  ; actually I don't know if that's bug
  ; or real limitation or my lack of knowledge how to 
  ; use it correcly
  return select state-collection/item-set-ids to-string item-set
]

; check wether item set identified by id exists already in state-collection
check-item-set-exists-by-id: function [id state-collection] [
  return select state-collection/id-to-item-sets id
]

; adds item set to state-collection
add-item-set: function [item-set state-collection] [
  entry: make block![]
  ; as described in checkItemSetExists
  ; itemSet object is converted to string forth and back
  append/only entry to-string item-set
  append entry state-collection/set-id
  append state-collection/item-set-ids entry

  entry-id-to-item-set: make block![]
  append entry-id-to-item-set state-collection/set-id
  append/only entry-id-to-item-set item-set
  append state-collection/id-to-item-sets entry-id-to-item-set

  state-collection/set-id: state-collection/set-id + 1
]

; selects specific grammar rule identified by name from grammar
select-rule: function[grammar name] [
  rule: select grammar name
  return rule 
]

; dumps grammar
print-grammar: function [grammar] [
  foreach [lhs rhs] grammar [
    prin [lhs]
    prin " -> "
    index: make integer! 0

    foreach variable rhs [
      if index > 0 [ prin " | " ]
      foreach var-elem variable [
        either empty? var-elem [prin "epsilon"] [prin " " prin var-elem prin " "]
      ]
      index: index + 1
    ]
   print ""
  ]
]

; adds new rule to grammar
add-rule: function[grammar name rhs][    
  present-rule: select-rule grammar name
  either not empty? present-rule [
    append/only present-rule rhs
  ] [
    rule: []
    rhs-block: []
    append/only rhs-block rhs
    append rule name
    append/only rule rhs-block
    append grammar rule
  ]
]


; generate all lr0-items from given grammar
generate-lr0-items: function[grammar] [
  lr0-items: []

  foreach [lhs rhs] grammar [
    lhs: lhs
    rhs: rhs
    rule-index: make integer! 1
    foreach rhs-block rhs [
      dot-pos: make Integer! 1
      foreach rhsElem rhs-block [
        ; add item except final one
        append lr0-items make item [ rule-lhs: lhs rule-rhs-index: rule-index dot-position: dot-pos ]
        dot-pos: dot-pos + 1
      ]
      ; add final item
      append lr0-items make item [ rule-lhs: lhs rule-rhs-index: rule-index dot-position: dot-pos ]
      rule-index: rule-index + 1
    ]
    
  ]
  return lr0-items
]

; print all items exist in lr0-items
print-lr0-items: function [lr0-items grammar] [
  foreach item lr0-items [
    prin item/rule-lhs
    prin " -> "
    rule: select grammar item/rule-lhs
    rhs: pick rule item/rule-rhs-index
    dot-pos: make integer! 1
    foreach variable rhs [
      if dot-pos == item/dot-position [
        prin "."
      ]
      either empty? variable [prin " epsilon "] 
      [ 
        prin " "
        prin variable
        prin " "
      ]
      dot-pos: dot-pos + 1
    ]

    ; condition for final item
    if dot-pos == item/dot-position [ prin "." ]

    dot-pos: 1
    print ""
  ]
]

; closure of LR0 items 
lr0-closure: function [lr0-items grammar] [
  
  open-set: make block![]
  result-set: make block![]

  append open-set lr0-items
  
  while [not tail? open-set] [        
  
    item: first open-set    
    append result-set item
    remove open-set
        
    rhs: select grammar item/rule-lhs
    if empty? rhs [continue]
        
    rhs-rule: pick rhs item/rule-rhs-index
    if empty? rhs-rule [ continue ]

    variable: pick rhs-rule item/dot-position

    if empty? variable [ continue ]

    next-production: select grammar variable
  
    if empty? next-production [continue]

    repeat index length? next-production [
      new-item: make item [rule-lhs: variable rule-rhs-index: index dot-position: 1]
      if not empty? find head open-set new-item [ continue ]
      
      if not empty? find head result-set new-item [ continue ]
   
      append open-set new-item
    ]
  ]

  return result-set
]

; LR0 goto function takes LR0 item set token and grammar
; and return another item set 
lr0-goto: function[lr0-items token grammar] [
  goto-set: make block![]
  foreach item lr0-items [
    rhs: select grammar item/rule-lhs
    rule: pick rhs item/rule-rhs-index
    ruleLen: length? rule
    if ruleLen < item/dot-position [continue]
    next-token: pick rule item/dot-position
    if equal? next-token token [
       new-item: make item [rule-lhs: item/rule-lhs rule-rhs-index: item/rule-rhs-index dot-position: item/dot-position + 1]
       append goto-set new-item
    ]
  ]
  return lr0-closure goto-set grammar
]

; get grammar and return all tokens
tokens: function[grammar] [  
  token-set: make block![]
  foreach [lhs rhs] grammar [
    append token-set lhs
    foreach rule rhs [
      foreach var rule [
        append token-set var
      ]
    ]
  ]
  return unique token-set
]

; helper function used to build an edge 
; from item set to item set via token
make-edge: function [from to token] [
  edge: make block![]
  append edge from
  append edge to
  append edge token
  return edge
]


; generate LR canonical collection items
generate-lr0-items-set: function[main-rule grammar state-collection] [
  
  edge-set: make block![]

  main-rule-lhs: first main-rule
  main-rule-rhs: second main-rule
  rhs: select grammar main-rule-lhs
  index: make integer! 1
  
  foreach e rhs [
    if equal? e main-rule-rhs [ break ]
    index: index + 1
  ]  
  
  initial-item: make item [rule-lhs: main-rule-lhs rule-rhs-index: index dot-position: 1]
  
  initial-set: make block![]
  append initial-set initial-item
  start-item-set: lr0-closure initial-set grammar
 
  add-item-set start-item-set state-collection
 
  open-items: make block![]

  append/only open-items start-item-set
  
  while [not tail? open-items] [
    item-set: first open-items
    remove open-items    
    foreach token tokens grammar [
      goto-set: make block! []
      goto-set: lr0-goto item-set token grammar
    
      if empty? goto-set [continue]

      if none? check-item-set-exists goto-set state-collection [
        add-item-set goto-set state-collection        
        append/only open-items goto-set
      ]
      
      from: check-item-set-exists item-set state-collection
      to: check-item-set-exists goto-set state-collection

      append/only edge-set make-edge from to token
    ]    
  ]
  return edge-set
]

; print all items exist in lr0 items
serialize-lr0-items: function [lr0-items grammar] [
  output: make string! ""
  if none? lr0-items [ return output ]
  foreach item lr0-items [
    append output item/rule-lhs
    append output " -> "
    rule: select grammar item/rule-lhs
    rhs: pick rule item/rule-rhs-index
    dot-pos: make integer! 1
    foreach variable rhs [
      if dot-pos == item/dot-position [
        append output "."
      ]
      either empty? variable [append output " epsilon "] 
      [ 
        append output  " "
        append output variable
        append output " "
      ]
      dot-pos: dot-pos + 1
    ]

    ; condition for final item
    if dot-pos == item/dot-position [ append output "." ]

    dot-pos: 1
    append output "^/"
  ]
  return output
]

; generate graphviz dot graph representation
; by taking grammar state-collection and edge-set
generate-dot: function [grammar state-collection edge-set] [
  output: make string! ""
  append output "digraph grammar {^/"
  append output "bgcolor=transparent; ^/"
  append output "node [color=lightblue,style=filled fontname = ^"font-fixed^" fontsize=11]; ^/"
  foreach edge edge-set [
    from: first edge state-collection
    to: second edge state-collection
    from-item-set: check-item-set-exists-by-id first edge state-collection
    to-item-set: check-item-set-exists-by-id second edge state-collection
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
    append output serialize-lr0-items from-item-set grammar
    append output "^" "
    append output "color=lightblue ]"
    append output "^/"
    append output to
    append output " [label=^""
    append output serialize-lr0-items to-item-set grammar
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

  add-rule testGrammar "A" ["0"]
  add-rule testGrammar "A" ["1"]
  add-rule testGrammar "A" [""]
  add-rule testGrammar "E" ["E" "*" "E"]
  add-rule testGrammar "E" ["E" "/" "E"]

  print-grammar testGrammar

]

test2: function[] [
  grammar: make block![
    "S" [["E"]] 
    "E" [["E" "+" "E"]["1"]]
  ]
  
  lr0-items: generate-lr0-items grammar
  ;print-lr0-items lr0-items grammar
  ;print mold lr0-items
  lr0-items1: []
  
  append lr0-items1 make item [ rule-lhs: "E" rule-rhs-index: 2 dot-position: 1 ]

  lr0-result: lr0-closure lr0-items1 grammar
  print "LR0closure:"
  print-lr0-items lr0-result grammar
  print "LR0goto:"
  ;print mold lr0-items1
  goto-result: lr0-goto lr0-items1 "0" grammar 
  print-lr0-items goto-result grammar
  ;print mold state-collection
  ;add-item-set LR0Result state-collection
  ;add-item-set LR0Result state-collection
  ;print mold state-collection
  ;print check-item-set-exists lr0-result state-collection
  ;print mold check-item-set-exists-by-id 1 state-collection  
]

test1
test2 

