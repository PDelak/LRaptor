Red [ Needs: 'View ]

#include %lrlibrary.red

system/view/silent?: yes

grammarTxt: {}
previousGrammar: load grammarTxt

editorView: [
	title "Grammar Editor"
	backdrop #2C3339
	across
	
	source: area #13181E 410x400 no-border grammarTxt font [
		name: font-fixed
		size: 9
		color: hex-to-rgb #9EBACB
	]

	panel 500x500 #2C3339 react [
		print source/text
		if not empty? source/text [               
			grammar: try [load grammarTxt]
			either error? grammar [ print "error" ] 
			[
				print "good"
				if not equal? previousGrammar grammar [
					stateCollection: make StateCollection [ itemSetIds: make hash![] idToItemSets: make hash![] setId: make integer! 1 ]
					mainRule: make block![]
					append mainRule first grammar
					append mainRule second grammar
					prin "mainRule -> : "
					print mold mainRule
					edgeSet: generateLR0ItemsSet mainRule grammar stateCollection
					write %grammar.dot generateDot grammar stateCollection edgeSet
					callCommand: "dot grammar.dot -Tpng -o grammar.png"
					call/wait callCommand
					graph: load %grammar.png		
					previousGrammar: copy grammar
				]
			]
		]
		attempt/safer [face/pane: layout/tight/only load {image graph}]                
	]
	return 
]

view/flags editorView 'resize
	