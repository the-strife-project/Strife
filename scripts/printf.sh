#!/bin/bash
dirname="$(pwd | rev | cut -d'/' -f1 | rev)"

if [ "$dirname" = "scripts" ]; then
	cd ..
fi

find projects/kernel/src -type f -regex '.*\(cpp\|hpp\)$' -exec grep -Hn printf {} \;
