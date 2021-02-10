# This will improve a lot, don't worry

FIND=find . -mindepth 1 -maxdepth 1 -type d

.PHONY: all clean
all:
	$(FIND) -exec $(MAKE) -C {} \;
	touch .done

clean:
	$(FIND) -exec $(MAKE) -C {} clean \;
	rm -f .done
