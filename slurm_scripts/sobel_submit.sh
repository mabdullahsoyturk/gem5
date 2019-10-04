#!/bin/bash
#
# CompecTA (c) 2018
#
#
# TODO:
#   - Set name of the job below changing  value.
#   - Set the requested number of nodes (servers) with --nodes parameter.
#   - Set the requested number of tasks (cpu cores) with --ntasks parameter. (Total accross all nodes)
#   - Select the partition (queue) you want to run the job in:
#     - shorter : For jobs that have maximum run time of 120 mins. Has higher priority.
#     - short   : For jobs that have maximum run time of 1 days. Has higher priority.
#     - mid     : For jobs that have maximum run time of 3 days. Lower priority than short.
#     - long    : For jobs that have maximum run time of 7 days. Lower priority than long.
#     - longer  : For testing purposes, queue has 15 days limit but only 2 nodes.
#     - cuda    : For CUDA jobs. Solver that can utilize CUDA acceleration can use this queue. 15 days limit.
#   - Set the required time limit for the job with --time parameter.
#     - Acceptable time formats include "minutes", "minutes:seconds", "hours:minutes:seconds", "days-hours", "days-hours:minutes" and "days-hours:minutes:seconds"
#   - Put this script and all the input file under the same directory.
#   - Set the required parameters, input/output file names below.
#   - If you do not want mail please remove the line that has --mail-type and --mail-user. If you do want to get notification emails, set your email address.
#   - Put this script and all the input file under the same directory.
#   - Submit this file using:
#      sbatch slurm_submit.sh
#
# -= Resources =-
#
#SBATCH --job-name=sobel
#SBATCH --account=users
#SBATCH --nodes=1
#SBATCH --ntasks=18
#SBATCH --partition=short
#SBATCH --time=1440
#SBATCH --output=%j-slurm.out

# #SBATCH --mail-type=ALL
# #SBATCH --mail-user=muhammetabdullahsoyturk@gmail.com

INPUT_FILE=""

######### DON'T DELETE BELOW THIS LINE ########################################
source /etc/profile.d/zzz_cta.sh
echo "source /etc/profile.d/zzz_cta.sh"
######### DON'T DELETE ABOW THIS LINE #########################################

# Module File
#echo "Loading Foo..."
#module load foo
#echo

echo ""
echo "======================================================================================"
env
echo "======================================================================================"
echo ""

echo "======================================================================================"
# Set stack size to unlimited
echo "Setting stack size to unlimited..."
ulimit -s unlimited
ulimit -l unlimited
ulimit -a
echo
echo "======================================================================================"

echo "Running..."
echo "======================================================================================"
echo "Starting new Job"
python3 run.py --random --bench-name=sobel --sobel-input=/cta/users/masoyturk/FaultModel/gem5/tests/test-progs/sobel/figs/input.grey --sobel-output=/cta/users/masoyturk/FaultModel/gem5/tests/test-progs/sobel/golden.bin
RET=$?
echo "Job finished. Return code is $RET"
echo ""

