import subprocess

numbers_of_faults = ["1", "2", "4", "8", "16", "32", "64", "128"]

for i in range(len(numbers_of_faults)):
    script = 'python3 script.py --flags=Cache,FaultTrace --bench-name=matrix_mul --number-of-faults=' + numbers_of_faults[i] + ' --number-of-experiments=1000'

    subprocess.call(script, shell=True)
