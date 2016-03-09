DOC_DIRS = html

documentation: $(DOC_DIRS)
	@echo OK
	
$(DOC_DIRS): doc-config
	doxygen doc-config

clean :
	rm -rf $(DOC_DIRS)


