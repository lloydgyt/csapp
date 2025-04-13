%.asm: %.o
	objdump -d $< > $@

%.o: %.s
	gcc -c $<

%_raw.txt: %.c # % -> phase[n]
	cat $< | ./hex2raw > $@
