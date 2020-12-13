SOURCE=final.tex
OUTPUT_DIR=../docs

.PHONY: all
all:
	pdflatex --output-directory=$(OUTPUT_DIR) $(SOURCE)

.PHONY: clean
clean:
	rm $(OUTPUT_DIR)/*.aux $(OUTPUT_DIR)/*.log $(OUTPUT_DIR)/*.out $(OUTPUT_DIR)/final.pdf

.PHONY: watch
watch:  # Recompile on updates to the source file
	while true; do \
		fswatch *.tex | make; \
	done
