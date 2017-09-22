Red [
	Title:   "Gammar Editor"
	Author:  "Przemyslaw Delewski"
	Needs:	 'View
]

#include %lrlibrary.red

system/view/silent?: yes

emptyGraph: { digraph grammar {}}
grammarTxt: {}

previousGrammar: load grammarTxt

generateGraph: function [grammar] [
	stateCollection: make StateCollection [ 
		itemSetIds: make hash![] idToItemSets: make hash![] setId: make integer! 1 
	]

	mainRule: make block![]
	mainRuleLHS: first grammar
	mainRuleRHS: second grammar

	if (none? mainRuleLHS) or (none? mainRuleRHS) [ write %grammar.dot emptyGraph ]

	append mainRule mainRuleLHS
	append mainRule mainRuleRHS
	prin "mainRule -> : "
	print mold mainRule

	edgeSet: make block![]

	if (not none? mainRuleLHS) and (not none? mainRuleRHS) [
		append edgeSet generateLR0ItemsSet mainRule grammar stateCollection
	]

	write %grammar.dot generateDot grammar stateCollection edgeSet
	callCommand: "dot grammar.dot -Tpng -o grammar.png"
	call/wait callCommand
	graph: load %grammar.png					
	return graph
]

editorView: layout[
	title "Grammar Editor"
	backdrop #2C3339
	across
	
	source: area #13181E 410x500 no-border grammarTxt font [
		name: font-fixed
		size: 9
		color: hex-to-rgb #9EBACB
	]

	panel 500x500 #13181E  react [
		print source/text
		if not empty? source/text 
		[               
			grammar: try [load source/text]
			either error? grammar [ print "error" ] 
			[
				print "good"
				either not equal? previousGrammar grammar 
				[
					graph: generateGraph grammar                                        
					previousGrammar: copy grammar					
				] 
				[print "equal"]
			]
		]		
		attempt/safer [face/pane: layout/tight/only load {image graph loose}]                
	]
	return 
]

lastSizeWin: 1x1
lastSizeSource: 1x1
lastSizeGraphPanel: 1x1
started: 0

scale: function [currentSize lastSize] [
	result: make block![]
	v1: make float! first currentSize 
	v2: make float! first lastSize
	append result v1 / v2
	v1: make float! second currentSize 
	v2: make float! second lastSize
	append result v1 / v2
	return result
]

editorView/flags: ['resize]

editorView/actors: make face! [		
		on-resize: func [f e] [
			either started == 0 [
				started: 1
				lastSizeWin: f/size
				lastSizeSource: f/pane/1/size
				lastSizeGraphPanel: f/pane/2/size
			]
			[
				currentWinSize: f/size
				currentSourceSize: f/pane/1/size
				currentGraphPanelSize: f/pane/2/size

				currentScale: scale currentWinSize lastSizeWin

				f/pane/1/size/x: to-integer make float! (first currentSourceSize) * (first currentScale) - 10 
				f/pane/1/size/y: to-integer make float! (second currentSourceSize) * (second currentScale)
				f/pane/2/size/x: to-integer make float! (first currentGraphPanelSize) * (first currentScale) + 10
				f/pane/2/size/y: to-integer make float! (second currentGraphPanelSize) * (second currentScale)

				f/pane/2/offset/x: to-integer make float! (first currentSourceSize) * (first currentScale) + 10
				lastSizeWin: f/size
				lastSizeSource: f/pane/1/size
				lastSizeGraphPanel: f/pane/2/size
			]
		]
]

view editorView

