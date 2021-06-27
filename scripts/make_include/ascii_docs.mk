

# Make a list of all txt files
ASCII_TXT_FILES = $(wildcard ascii_docs/*.txt)

# Convert the list of txt files into a list of png files
# these png files don't exist yet but we create them
ASCII_PNG_FILES := $(ASCII_TXT_FILES:.txt=.png)


# foo:
# 	@echo $(ASCII_TXT_FILES)
# 	@echo $(ASCII_PNG_FILES)

ascii: $(ASCII_PNG_FILES)


ascii_docs/%.png: ascii_docs/%.txt
	java -jar scripts/ditaa/ditaa0_9.jar -o $^
	@echo ""
	@echo $^
	@echo $@
