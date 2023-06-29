# -*-Makefile-*-

# (Re)compile and run the example
run EXAMPLE='B1':
	#!/usr/bin/env sh
	just compile {{EXAMPLE}}
	cd       {{EXAMPLE}}/build
	if [ -x ./example{{EXAMPLE}} ]
	then
		./example{{EXAMPLE}}
	elif [ -x ./{{EXAMPLE}} ]
	then
		./{{EXAMPLE}}
	else
		echo "Couldn't guess executable name."
		fail
	fi

compile EXAMPLE:
	#!/usr/bin/env sh
	just configure {{EXAMPLE}}
	cd       {{EXAMPLE}}/build
	make -j

configure EXAMPLE:
	#!/usr/bin/env sh
	just copy {{EXAMPLE}}
	mkdir -p {{EXAMPLE}}/build
	cd       {{EXAMPLE}}/build
	cmake ..

# Copy the source of the given example into the top-level directory, ready for compilation
copy EXAMPLE:
	#!/usr/bin/env sh
	if [ ! -d {{EXAMPLE}} ]
	then
		echo Copying source of example {{EXAMPLE}}
		cp -r --no-preserve=mode,ownership `just find {{EXAMPLE}}` .
	else
		echo Source of example {{EXAMPLE}} already present
	fi

# Find the location of the example in the nix store
find EXAMPLE:
	#!/usr/bin/env sh
	`echo find ${G4_EXAMPLES_DIR} -type d -name {{EXAMPLE}}`

# Remove all traces of the local copy of the example
expunge EXAMPLE:
	#!/usr/bin/env sh
	rm {{EXAMPLE}} -rf
