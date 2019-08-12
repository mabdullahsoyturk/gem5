import subprocess

numbers_of_faults = ["1", "2", "4", "8", "16", "32", "64", "128"]

for number_of_faults in numbers_of_faults:
    script = 'python3 script.py --flags=Cache,FaultTrace --bench-name=matrix_mul --number-of-faults=' + number_of_faults + ' --number-of-experiments=1000'

    subprocess.call(script, shell=True)