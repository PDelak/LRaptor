Red [
	Title:   "Gammar Editor"
	Author:  "Przemyslaw Delewski"
	Needs:	 'View
]

#include %lrlibrary.red
#include %grammar-parser.red

empty-graph: { digraph grammar {}}
grammar-txt: {}

previous-grammar: load grammar-txt

generate-graph: function [grammar] [
	state-collection: make StateCollection [ 
		itemSetIds: make hash![] idToItemSets: make hash![] setId: make integer! 1 
	]

	main-rule: make block![]
	main-rule-lhs: first grammar
	main-rule-rhs: second grammar

	if (none? main-rule-lhs) or (none? main-rule-rhs) [ write %grammar.dot empty-graph ]

	append main-rule main-rule-lhs
	append main-rule main-rule-rhs
	prin "main-rule -> : "
	print mold main-rule

	edge-set: make block![]

	if (not none? main-rule-lhs) and (not none? main-rule-rhs) [
		append edge-set generateLR0ItemsSet main-rule grammar state-collection
	]

	write %grammar.dot generateDot grammar state-collection edge-set
	call-command: "dot grammar.dot -Tpng -o grammar.png"
	call/wait call-command
	graph: load %grammar.png					
	return graph
]

editor-view: layout[
	title "Grammar Editor"
	backdrop #2C3339
	across
	
	source: area #13181E 410x500 no-border grammar-txt font [
		name: font-fixed
		size: 9
		color: hex-to-rgb #9EBACB
	]

	graphPanel: panel 500x500 #13181E 
	react [
		grammar: []	
		print source/text
		if (empty? source/text) and (not equal? previous-grammar grammar ) [
			
			graph: generate-graph grammar                                        
			previous-grammar: copy grammar
		]
		if not empty? source/text 
		[               
			internal-repr: make block![]
			result: parse-grammar source/text internal-repr
			print result
			if result = true [
				print mold internal-repr
				grammar: internal-repr
			]
            
			either error? grammar [ print "error" ] 
			[
				print "good"
				either not equal? previous-grammar grammar 
				[
					graph: generate-graph grammar                                        
					previous-grammar: copy grammar					
				] 
				[print "equal"]
			]
		]		
		attempt/safer [face/pane: layout/tight/only load {image graph loose}]                
	]

	return 
]

editor-view/flags: ['resize]

print mold editor-view/pane/2

editor-view/actors: make face! [		
		on-resize: func [f e] [            
		        ; source area takes 40 percent of whole screen     
				f/pane/1/size/x: f/size/x * 40 / 100
				; y size is subtracted by 20
				f/pane/1/size/y: f/size/y - 20
				; graph panel takes 60 percent
				f/pane/2/size/x: f/size/x * 60 / 100 - 25
				f/pane/2/size/y: f/size/y - 20

				f/pane/2/offset/x: f/pane/1/offset/x + f/pane/1/size/x + 5	
				append grammar-txt ""		
		]
]

view editor-view
