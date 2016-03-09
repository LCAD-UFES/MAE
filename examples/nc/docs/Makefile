DOC_DIRS = html latex man rtf

documentation: $(DOC_DIRS) 
	@echo OK
	
$(DOC_DIRS)	: doc-config
	dot -Gratio=fill -Gsize=10,10! -Tpng ../nc.dot -o ./images/nc_network.png
	doxygen doc-config

clean :
	rm -rf $(DOC_DIRS)


