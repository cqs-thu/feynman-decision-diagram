.PHONY: build clean help test

MAKE_FLAGS=-j $$(nproc)

BUILD_TYPE := Release
STATS := OFF

build:
	@echo "\nBuild"
	@mkdir -p build/ && cd build/ && \
           cmake -D CMAKE_BUILD_TYPE=$(BUILD_TYPE) \
                 -D BDD_BENCHMARK_STATS=$(STATS) ..

	@echo "\n\nInstall CUDD"
	@[ -d "build/cudd/" ] || (cd external/cudd && autoreconf \
        && ./configure --prefix ${CURDIR}/build/cudd/ --enable-obj \
		&& make MAKEINFO=true && make install)

	# @echo "\n\nInstall Sylvan"
	# @[ -d "build/sylvan/" ] || (cd external/sylvan && autoreconf \
    #     && ./configure --prefix ${CURDIR}/build/sylvan/ --enable-obj \
	# 	&& make MAKEINFO=true && make install)

	@mkdir -p out/

	@echo "\n\nBuild BDD Benchmarks"
#	@cd build/ && for package in 'adiar' 'buddy' 'cal' 'cudd' 'sylvan' ; do
	@cd build/ && for package in 'cudd'; do \
		mkdir -p ../out/$$package ; \
		mkdir -p ../out/$$package/bdd ; \
		make ${MAKE_FLAGS} $$package'_circuit_bdd' ; \
	done

	@echo "\n"

test:
	@echo "\nUnit Tests"
	@cd build/ && make circuit_tests
	@./build/test/circuit_tests

clean:
	@cd external/cudd && git clean -qdf && git checkout .
	@rm -rf build/
	@rm -rf out/

clean/out:
	@rm -f out/**/*.out

help:
	@echo ""
	@echo "Feynman Decision Diagram Benchmarks"
	@echo "================================================================================"

	@echo ""
	@echo "Compilation"
	@echo "-----------"

	@echo ""
	@echo "build:"
	@echo ""
	@echo "   + BUILD_TYPE=[Release, Debug, RelWithDebInfo, MinSizeRel] (default: Release)"
	@echo "   | Type of build, i.e. the compiler settings to use."
	@echo ""
	@echo "   + STATS=[ON,OFF] (default: OFF)"
	@echo "   | Whether to build all BDD packages with their statistics turned on. If 'ON'"
	@echo "   | then time measurements are unusable."

	@echo ""
	@echo "clean:"
	@echo "   Remove all build artifacts and the 'out/' folder."

	@echo ""
	@echo "clean/out:"
	@echo "   Remove *.out files in the 'out/' folder."

	@echo ""
	@echo "--------------------------------------------------------------------------------"
	@echo ""
	@echo "Benchmarks"
	@echo "----------"
	@echo ""
	@echo "circuit:"
	@echo "   Runs the BDD simulation of quantum circuits."
	@echo "   It can work with the following two optional arguments."
	@echo ""
	@echo "   + V=[adiar, buddy, cal, cudd, sylvan] (default: cudd)"
	@echo "   | BDD package to use."
	@echo ""
	@echo "   + M=<int> (default: 128)"
	@echo "   | Memory (MiB) to dedicate to the BDD package."

	@echo ""
	@echo "--------------------------------------------------------------------------------"
	@echo ""
	@echo "Other"
	@echo "-----"
	@echo ""
	@echo "help:"
	@echo "   Prints this hopefully helpful piece of text."
	@echo ""
	@echo "--------------------------------------------------------------------------------"
	@echo ""

# Variant (adiar, buddy, cal, cudd, sylvan)
V:=cudd

# Memory
M:=128

circuit:
	$(MAKE) circuit/bdd

circuit/bdd:
	@$(subst VARIANT,$(V),./build/src/VARIANT_circuit_bdd -M $(M) 2>&1 | tee -a out/VARIANT/bdd/circuit.out)
