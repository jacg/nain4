# -*-Makefile-*-

# (Re)compile and run the example
run EXAMPLE='B1':
	#!/usr/bin/env sh
	just compile {{EXAMPLE}}
	cd       {{EXAMPLE}}/build
	./example{{EXAMPLE}}

compile EXAMPLE:
	#!/usr/bin/env sh
	just copy {{EXAMPLE}}
	echo {{EXAMPLE}}
	mkdir -p {{EXAMPLE}}/build
	cd       {{EXAMPLE}}/build
	cmake ..
	make -j

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
