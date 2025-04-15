lab = archlab-handout
$(lab): % : %.tar
	rm -f $<:Zone.Identifier
	tar xvf $<
	rm -f $<
	git add -A
	git commit -m "assignment: $@"
