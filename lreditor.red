Red [
	Title:   "Gammar Editor"
	Author:  "Przemyslaw Delewski"
	Needs:	 'View
]

#include %lr-library.red
#include %grammar-parser.red



empty-graph: { digraph grammar {}}

grammar-txt: {}
print grammar-txt
input-txt: {}
stack-txt: {}


previous-grammar: load grammar-txt

convert-grammar: function [grammar] [
  
  print "convert grammar"
  str: make string![]
    
  foreach [lhs rhs] grammar [
    foreach rhsRule rhs [
    	append str lhs
    	append str " "        	
    	append str rhsRule
    	append str "^/"
    ]  	  	
  ]
  print str
  return str
]

generate-graph: function [grammar] [
	s-collection: make state-collection [ 
		item-set-ids: make hash![] id-to-item-sets: make hash![] set-id: make integer! 1 
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
		append edge-set generate-lr0-items-set main-rule grammar s-collection
	]

	write %grammar.dot generate-dot grammar s-collection edge-set
	call-command: "dot grammar.dot -Tpng -o grammar.png"
	call/wait call-command
	graph: load %grammar.png					
	return graph
]

build-parse-tree: function [] [
]

editor-view: layout[
	title "LR Editor"
	backdrop #2C3339
	across
	
	source: area #13181E 410x500 no-border grammar-txt font [
		name: font-fixed
		size: 15
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
					convert-grammar grammar
					write %lrgrammar.txt grammar-txt
					write %grammar.txt convert-grammar grammar
				] 
				[
					print "equal"					
				]
			]
		]
		if not empty? inputSource/text [
			print inputSource/text
			inputValue: copy inputSource/text
			write %input.txt trim inputValue			
			;parseTree/text: inputSource/text
			call-command: "lrtreevisitor.exe"
			call/wait call-command
			ptree: read %parseTree.txt
			parseTree/text: ptree
		]
		if empty? inputSource/text [
			parseTree/text: ""
		]

		attempt/safer [face/pane: layout/tight/only load {image graph loose}]                
	] 

	inputSource: area #13181E 410x500 no-border input-txt font [
		name: font-fixed
		size: 15
		color: hex-to-rgb #9EBACB
	]
	
	parseTree: area #13181E 410x500 no-border input-txt font [
		name: font-fixed
		size: 20
		color: hex-to-rgb #9EBACB
	]

	return 
]

editor-view/flags: ['resize]

print mold editor-view/pane/1

editor-view/actors: make face! [		
		on-resize: func [f e] [            
		        ; source area takes 40 percent of whole screen     
				f/pane/1/size/x: f/size/x * 30 / 100
				; y size is subtracted by 20
				f/pane/1/size/y: f/size/y * 60 / 100 - 20
				; graph panel takes 60 percent
				f/pane/2/size/x: f/size/x * 35 / 100 - 25
				f/pane/2/size/y: f/size/y * 100 / 100 - 20

				f/pane/3/size/x: f/size/x * 30 / 100
				; y size is subtracted by 20
				f/pane/3/size/y: f/size/y * 10 / 100 - 20

				;f/pane/5/size/x: f/size/x * 30 / 100
				; y size is subtracted by 20
				;f/pane/5/size/y: f/size/y * 10 / 100 - 20

				f/pane/4/size/x: f/size/x * 35 / 100 - 5
				f/pane/4/size/y: f/size/y * 100 / 100 - 20


				f/pane/2/offset/x: f/pane/1/offset/x + f/pane/1/size/x + 5	
				f/pane/4/offset/x: f/pane/2/offset/x + f/pane/2/size/x + 5	

				f/pane/3/offset/x: f/pane/1/offset/x
				f/pane/3/offset/y: f/pane/1/offset/y + f/pane/1/size/y + 5	
				;f/pane/5/offset/x: f/pane/1/offset/x
				;f/pane/5/offset/y: f/pane/1/offset/y + f/pane/1/size/y + 5	
				;append grammar-txt ""		
				;append input-txt ""		
				;append stack-txt ""		
		]
]

view editor-view
