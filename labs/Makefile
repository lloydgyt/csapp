lab = archlab-handout shlab-handout malloclab-handout
$(lab): % : %.tar
	rm -f $<:Zone.Identifier
	tar xvf $<
	rm -f $<
	git add $@
	git commit # this is to avoid accidental commit
