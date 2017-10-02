Red [
  Title:  "Basic LR driver"
  Author: "Przemyslaw Delewski"
]

#include %lr-library.red

grammar: make block! [
  "S" [["E"]] 
  "E" [["E" "+" "E"]["1"]]
]

s-collection: make state-collection [ item-set-ids: make hash![] id-to-item-sets: make hash![] set-id: make integer! 1 ]

edge-set: generate-lr0-items-set ["S" ["E"]] grammar s-collection

write %grammar.dot generate-dot grammar s-collection edge-set
