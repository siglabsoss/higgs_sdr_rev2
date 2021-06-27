#####
#
#  This makefile is the main entry point 
#  for automated tests UNDER this folder
#

.PHONY: all test_random test_random_multi clean_test_random test clean cleanall
.PHONY: test_compile_only test_compile_production
.PHONY: random_test_run

all:
	@echo ""
	@echo "This folder has subfolders with tests"
	@echo "cd into one of them and run make"
	@echo ""


# what happens if we disble this
# test_eth_bug

# disabled due to d-engine
# test_dma_torture
# test_eth_ring
# test_backpressure_2
# test_backpressure_4
# test_backpressure_5
# test_feedback_bus_26   # seems like I'm still developing on this test

# test_feedback_bus
RANDOMIZED_TESTS = \
test_dma_fft \
test_feedback_bus_2 \
test_feedback_bus_10 \
test_forward_1 \
test_feedback_bus \
test_cs11_pushback_2 \
test_feedback_bus_11 \
test_uart_frame_2 \
test_feedback_bus_24 \
test_issue_112 \
test_issue_112_b \
test_fork_1

# only passes sometimes on master3
# 

# we put : before the foreach, which is a bash no-op,
# then inside the loop, we begin each command with &&.  this results
# in commands that look like
#
#  : && cmd1 && cmd2
#
# without something like this we end up with a 
# leading or trailing && which is an error.
# Previously these were terminated with ; WHICH WAS IGNORING failures
#
define invoke_rnd_test_build
: $(foreach ttest,$(RANDOMIZED_TESTS), && make -C $(ttest) pre_work )
endef

# only builds directories that have a 'seeds.txt' in them
define invoke_rnd_seed_test_build
: $(foreach ttest,$(RANDOMIZED_TESTS), &&  [ -f $(ttest)/seeds.txt ] && make -C $(ttest) pre_work || true  )
endef

define invoke_rnd_test_run
: $(foreach ttest,$(RANDOMIZED_TESTS), && make -C $(ttest) run test_post_check )
endef

define invoke_rnd_test_clean
: $(foreach ttest,$(RANDOMIZED_TESTS), && make -C $(ttest) clean )
endef

# test_seeds internally already runs "run test_post_check"
define invoke_test_seeds
: $(foreach ttest,$(RANDOMIZED_TESTS), && make -C $(ttest) test_seeds )
endef

.PHONY: test_seeds only_test_seeds

# fixme how to run collapse automated tests and normal tests
# into variables?
# how to collapse into a loop?

test_random_build:
	$(call invoke_rnd_test_build)

test_seed_build:
	$(call invoke_rnd_seed_test_build)

test_random:
	make test_random_build
	make random_test_run

unfailing_test_random:
	make test_random_build
	make unfailing_random_test_run

test_random_multi:
	make test_random_build
	make random_test_run
	make random_test_run

test_seeds:
	make test_seed_build
	make only_test_seeds

# not used at the moment we would like to know right away
# when the random ones fail
unfailing_test_random_multi:
	make test_random_build
	make unfailing_random_test_run
	make unfailing_random_test_run



random_test_run:
	$(call invoke_rnd_test_run)

unfailing_random_test_run:
	@make generated_test_random.sh
	./generated_test_random.sh

clean_test_random:
	$(call invoke_rnd_test_clean)

# Edited this list so that inject and cs30 dma tests are first
# These are simple tests which verify that the testbench is functioning
# properly

#
# test_mapmov_select

# disabled due to d-engine
# test_vmem_1
# test_ook_1
# test_ook_3
# test_inject_cs21
# test_cs20_dma_2
# test_ring_occupancy_1

TEST_LIST=\
test_inject_cs22 \
test_inject_cs31 \
test_mem_inject_1 \
test_ring_occupancy_2 \
test_cs20_dma \
test_all_csr \
test_eth_dma \
test_dma_slicer \
test_vmem_0 \
test_vmem_2 \
test_vmem_3 \
test_vmem_opt_order \
test_vmalloc \
test_vmalloc_compile \
test_cir_buf_2 \
test_ring_inout \
test_ring_simple \
test_slice_0 \
test_fft_lib_1 \
test_fill_lib \
test_mover_lib \
test_mapper_lib \
test_cs11_pushback \
test_hw_mul \
test_memory_manager \
test_feedback_bus_7 \
test_feedback_bus_8 \
test_ring_3 \
test_uart_frame_3 \
test_uart_frame_5 \
test_tx_9 \
test_bootloader

define invoke_test_build
: $(foreach ttest,$(TEST_LIST), && make -C $(ttest) test )
endef

define invoke_test_run
: $(foreach ttest,$(TEST_LIST), && make -C $(ttest) run test_post_check )
endef

define invoke_no_clean_test
: $(foreach ttest,$(TEST_LIST), && make -C $(ttest) no_clean_test test_post_check )
endef

define invoke_test_clean
: $(foreach ttest,$(TEST_LIST), && make -C $(ttest) clean )
endef

define invoke_test_vall
: $(foreach ttest,$(TEST_LIST) $(RANDOMIZED_TESTS), && make -C $(ttest) vall )
endef

define invoke_test_vall_subtree
: $(foreach ttest,$(TEST_LIST) $(RANDOMIZED_TESTS), && make -C $(ttest) vall_subtree )
endef

define invoke_test_vclean
: $(foreach ttest,$(TEST_LIST) $(RANDOMIZED_TESTS), && make -C $(ttest) vclean )
endef

.PHONY: $(TEST_LIST)


# from the list of targets, create a list with
# the string pre_work_ prepended to the FOLDER name
# this gives us a target we can use with -j20 below
# I couldnt figure out how to do "-j" when using -C for different submake folders
# in any other way

PRE_WORK_PREFIX=pre_work_
PRE_WORK_TARGETS=$(addprefix $(PRE_WORK_PREFIX),$(TEST_LIST) $(RANDOMIZED_TESTS))
PRE_WORK_TARGET_STRIPPED=$(@:$(PRE_WORK_PREFIX)%=%)


$(PRE_WORK_TARGETS):
	make -C $(PRE_WORK_TARGET_STRIPPED) pre_work

# adjust jobs here
pre_work:
	$(MAKE) $(PRE_WORK_TARGETS)

test2:
	make test_compile_only    # should be quick
	$(call invoke_test_build)
	$(call invoke_test_run)
	make test_random # invoke submake for randomized tests

# as of writing, jenkins.defs in the root directory, calls
# make clean, make test, in this directory
test:
	@echo "-------------------------------------------------------------"
	@echo "-"
	@echo "- Starting Automated Tests of Verilated code"
	@echo "-"
	@echo "-------------------------------------------------------------"
	@echo ""
	@echo ${TEST_LIST}
	@echo "-------------------------------------------------------------"
	@echo ""
	make -C ../../ vall
	make test_compile_only    # should be quick
	make test_compile_production
	make -j3 pre_work
	# Only run tests 1:1 now that parallel compile is done
	# If unfailing mode means that if any fail, we keep going
	# However if any fail in normal tests, we will not get to the random ones
	make unfailing_no_clean_test
	make unfailing_test_random

no_clean_test:
	$(call invoke_no_clean_test)

unfailing_no_clean_test:
	@make generated_test.sh
	./generated_test.sh


clean:
	$(call invoke_test_clean)
	make clean_test_random # invoke clean submake for randomized tests

cleanall:
	for d in ./*/ ; do (cd "$d" && make clean); done


###################
#
#  Compile only tests
#  these are folders that are meant to be bootloaded to the board
#  and checked by a human.  They do not have self-tests however
#  they need to be able to compile, so we try a compile
#  under jenkins to catch any errors or changes to code
#


test_compile_only:
	@echo ""
# 	make -C test_rx_chain vall
# 	make -C test_energy vall
# 	make -C verify_edge vall
# 	make -C verify_fft_lib_opt vall
# 	make -C test_nco vall

###################
#
#  Compile production
#  these are folders that are meant to be bootloaded to the board
#  for our production experiments they need to be able to compile
#

test_compile_production:
	make -C test_mover_9 vall
	make -C test_tx_9 vall

.PHONY: vall vclean

vall:
	$(call invoke_test_vall_subtree)

vclean:
	$(call invoke_test_vclean)




only_test_seeds:
	$(call invoke_test_seeds)


###################
#
#  Unfailing version of test.  We use the 
#    https://github.com/kward/shunit2/
#  Library to allow tests to keep running after a fail
#



# define write_no_clean_test_file
# : $(foreach ttest,$(TEST_LIST), && make -C $(ttest) no_clean_test test_post_check )
# endef


# we require that every test be prefixed with test_
# at time of writing, every folder want to use already is like this
# HOWEVER we may want to change this in the future so we prefix
# with test
define foow
: $(foreach ttest,$(TEST_LIST), && echo test___$(ttest) )
endef

define write_out_test_bash_file
: $(foreach ttest,$(2), \
&& echo test___$(ttest)\(\) { >> $(1)\
&& echo make -C $(ttest) $(3) test_post_check >> $(1)\
&& echo } >> $(1)\
&& echo >> $(1)\
)
endef

# Write a new bash script
# This target gos through the test variable list and writes the file out
# We only need to re-generate the file if this Makefile has changed
generated_test.sh: Makefile
	@echo "#!/bin/bash" > $@
	@echo "" >> $@
	@echo "" >> $@
	@$(call write_out_test_bash_file, $@, $(TEST_LIST), no_clean_test)
	@echo "SHUNIT_COLOR=none" >> $@
	@echo "" >> $@
	@echo "# Source test library (include it)" >> $@
	@echo ". ../../scripts/shunit2/shunit2" >> $@
	@echo "" >> $@
	@chmod +x ./$@
	@echo "writing $@"

generated_test_random.sh: Makefile
	@echo "#!/bin/bash" > $@
	@echo "" >> $@
	@echo "" >> $@
	@$(call write_out_test_bash_file, $@, $(RANDOMIZED_TESTS), run)
	@echo "SHUNIT_COLOR=none" >> $@
	@echo "" >> $@
	@echo "# Source test library (include it)" >> $@
	@echo ". ../../scripts/shunit2/shunit2" >> $@
	@echo "" >> $@
	@chmod +x ./$@
	@echo "writing $@"
