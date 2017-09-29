Red [
  Title:  "Basic LR driver"
  Author: "Przemyslaw Delewski"
]

#include %lr-library.red

grammar: make block! [
  "S" [["E"]] 
  "E" [["E" "+" "E"]["1"]]
]

stateCollection: make StateCollection [ itemSetIds: make hash![] idToItemSets: make hash![] setId: make integer! 1 ]

edgeSet: generateLR0ItemsSet ["S" ["E"]] grammar stateCollection

write %grammar.dot generateDot grammar stateCollection edgeSet
