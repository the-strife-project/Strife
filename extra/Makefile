OUTPUT=../iso
FILES=$(OUTPUT)/font $(OUTPUT)/splash.txt $(OUTPUT)/shell.txt $(OUTPUT)/identify.txt

.PHONY: all

all: $(FILES)

$(OUTPUT)/%: %
	cp -rv $< $@
