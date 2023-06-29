# -*-Makefile-*-

# ---- Recipes relating to official Geant4 examples. See below for Nain4 recipes --------------------

# (Re)compile and run the example
run EXAMPLE='B1':
	#!/usr/bin/env sh
	just compile {{EXAMPLE}} &&
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
	just hack-cmake {{EXAMPLE}}
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

# Modify EXAMPLE's CMakeLists.txt to enable LSP to find the correct header files
hack-cmake EXAMPLE:
	#!/usr/bin/env sh
	sed -i '/^project[(]/r clangd-headers.cmake' {{EXAMPLE}}/CMakeLists.txt

# ------ nain4 recipes ----------------------------------------------------------------------------

# All the nain4 recipes' names should start with 'n4' for now, to avoid
# accidental clashes with the G4 example recipes. But we need a better scheme
# once the dust settles after importing nain4.

# Need to run each test in a separate process, otherwise the monolithic and
# persistent Geant4 Run and Kernel managers will make things go wrong.

# Run all tests in hand-written loop: more informative and colourful than ctest
n4test-all *FLAGS:
	just n4test '' {{FLAGS}}


# Run a selection of tests
n4test PATTERN *FLAGS: n4build
	#!/usr/bin/env bash
	cd nain4/build
	NPASSED=0
	NFAILED=0
	FAILED=
	while read -r testname
	do
		if ! ./nain4-tests "$testname" {{FLAGS}}; then
			FAILED=$FAILED"$testname"\\n
			NFAILED=$((NFAILED+1))
		else
			NPASSED=$((NPASSED+1))
		fi
	done < <(./nain4-tests {{PATTERN}} --list-test-names-only)
	if ! [ -z "$FAILED" ]; then
		printf "\\033[91m===========================================================================\n"
		printf "\\033[32m Passed $NPASSED tests, \\033[91m Failed $NFAILED\n\n"
		printf "\\033[91m Failures: \n\n$FAILED\n"
		printf "\\033[91m===========================================================================\n"
		printf "\\033[91mOVERALL: ============================== FAIL ==============================\n"
		printf "\\033[91m===========================================================================\n"
		printf "\\033[0m"
		exit 1
	else
		printf "\\033[32m Ran $NPASSED tests\n\n"
		printf "\\033[32m===========================================================================\n"
		printf "\\033[32mOVERALL: ============================== PASS ==============================\n"
		printf "\\033[32m===========================================================================\n"
		printf "\\033[0m"
	fi

n4build: n4cmake
	#!/usr/bin/env sh
	cd nain4/build && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo . && make -j

n4cmake:
	#!/usr/bin/env sh
	if ! [ -d nain4/build ]; then
		mkdir nain4/build
		cd    nain4/build
		cmake ..
	fi

n4clean:
	rm nain4/build -rf
