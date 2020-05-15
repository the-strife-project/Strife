OUTPUT=../iso
FILES=$(OUTPUT)/font $(OUTPUT)/splash.txt $(OUTPUT)/shell.txt

.PHONY: all

all: $(FILES)

$(OUTPUT)/%: %
	cp -rv $< $@
