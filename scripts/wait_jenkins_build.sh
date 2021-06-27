#!/bin/bash

# this calls make jenkins_fetch_build_branch in a loop intil it returns 0
# after this happens, we call make jenkins_checkout_output_branch, and then make jenkins_copy_files
# this loop is finicky because make jenkins_fetch_build_branch will only return 0 one time, ie if you were to call it AGAIN
# after the return 0, you go back to non zero return code

# asume that cwd was set so that we are in a make folder

echo "checking every N seconds for build results"

until make --no-print-directory jenkins_fetch_build_branch
do
  echo "Looking again for build done"
  sleep 5
done

make jenkins_checkout_output_branch && make jenkins_copy_files

