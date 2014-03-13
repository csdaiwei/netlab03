#http://coolshell.cn/articles/3790.html#more-3790

%:
	@echo '$*=$($*)'

d-%:
	@echo '$*=$($*)'
	@echo '	origin = $(origin $*)'
	@echo '	 value = $(value  $*)'
	@echo '	flavor = $(flavor $*)'

