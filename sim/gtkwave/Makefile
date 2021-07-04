all: bin

.PHONY: all bin

bin: test_process show_op

test_process: src/test_process.c
	gcc src/test_process.c -o test_process

show_op: src/show_op.c
	g++ src/show_op.c -o show_op

clean:
	rm -f test_process show_op

help:
	@echo ""
	@echo "These can be used with gtkwave to filter outputs"
	@echo ""
