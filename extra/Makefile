OUTPUT=../iso
FILES=$(OUTPUT)/font

.PHONY: all

all: $(FILES)

$(OUTPUT)/%: %
	cp -rv $< $@
