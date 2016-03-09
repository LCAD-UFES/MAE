DOC_DIRS = html latex man rtf

documentation: $(DOC_DIRS) 
	@echo OK
	
$(DOC_DIRS)	: doc-config
	dot -Tpng ../src/y.dot -o mae_structure.png
	doxygen doc-config

clean :
	rm -rf $(DOC_DIRS)


