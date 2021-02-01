OUTPUT=../iso
FILES=$(OUTPUT)/font $(OUTPUT)/splash.txt $(OUTPUT)/identify.txt

.PHONY: all

all: $(FILES)

$(OUTPUT)/%: %
	cp -rv $< $@
