#!/usr/bin/env python
# coding: utf-8

import os, string
import time
import argparse
import sys
from subprocess import Popen, PIPE
import pexpect
import json
import pandas as pd
import subprocess

parser = argparse.ArgumentParser(description='Run storage tests for daqmoduletest')
#parser.add_argument('--bytes_total', '-b', type=str, required=True, help='Total bytes to write to disk', default=4096)
#parser.add_argument('--num_queues', '-q', type=str, required=True, help='Number of consumer/producer pairs', default=1)
parser.add_argument('--output_dir', '-o', type=str, required=True, help='Directory where consumers write temporary output files')
parser.add_argument('--pinning_conf', '-p', type=str, required=False, help='Pinning configuration', default="no_pinning")
parser.add_argument('--num_runs', '-r', type=int, required=False, help='Number of runs to execute for each unique configuration', default=1)
parser.add_argument('--time_to_run', '-t', type=int, required=False, help='Time to run the application before stopping it (in seconds)', default=10)
parser.add_argument('--time_for_warmup', '-w', type=int, required=False, help='Time to write before starting the real measurement (in seconds)', default=5)
parser.add_argument('result_file', help='Result file')


try:
  args = parser.parse_args()
except:
  parser.print_help()
  sys.exit(0)

# config parameters
pinning_conf = args.pinning_conf
output_dir = args.output_dir
bytes_total =  [2**x for x in range(12,31)]
num_queues = [1, 2, 4, 8, 16]
num_runs = args.num_runs
time_for_warmup = args.time_for_warmup
time_to_run = args.time_to_run

child = pexpect.spawn('bash')

# Setup the environment
print('Setup environment')
child.sendline('source daq-buildtools/dbt-setup-env.sh')
child.expect("DBT setuptools loaded")
child.sendline('dbt-setup-runtime-environment')
child.expect('This script has been sourced successfully')
#child.sendline('export DUNEDAQ_OPMON_INTERVAL=1')


# Build the project
print('Build project')
commit_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], cwd='sourcecode/daqmoduletest').decode('ascii').strip()
print('Commit hash is: ' + commit_hash)
child.sendline('dbt-build.sh --install')
child.expect('Installation complete.')

df = pd.DataFrame()

for n in num_queues:
    for bytes in bytes_total:
        # Generate configuration
        print('Generate config for ' + str(n) + ' queues and ' + str(bytes) + " bytes")
        child.sendline('python3 sourcecode/daqmoduletest/python/create_config.py -q ' + str(n) + ' -b ' + str(bytes) + ' -o ' + str(output_dir) + ' -p ' + pinning_conf)
        child.expect('generation completed.')

        for i in range(num_runs):
            # Run the program
            print('Run program')
            child.sendline('daq_application -n hallo -c stdin://./app.json')
            child.expect('Available commands: | init | start | stop')
            child.sendline('init')
            child.expect('Command init execution resulted with: 1 OK')
            if (args.pinning_conf != "no_pinning"):
                if (os.system("python3 sourcecode/daqmoduletest/python/balancer.py --process daq_application --pinfile pinnings.json") != 0):
                    print("Failed to set pinning")
                    sys.exit('Failed to set pinning')

            child.sendline('start')
            child.expect('Command start execution resulted with: 1 OK')
            time.sleep(time_for_warmup)
            child.sendline('start_measurement')
            child.expect('Command start_measurement execution resulted with: 1 OK')
            time.sleep(time_to_run)
            child.sendline('stop_measurement')
            child.expect('Command stop_measurement execution resulted with: 1 OK')
            # For big message sizes and queue numbers stopping can take very long
            #child.sendline('stop')
            #child.expect('Command stop execution resulted with: 1 OK')
            child.sendline('\003')

            print('Get results')
            for j in range(n):
                file = open('runs/cons_' + str(j) + '_log.jsonl', 'r')
                lines = file.readlines()
                result = json.loads(lines[-1])
                if not result['completed_measurement']:
                    print("run was not completed successfully")
                    sys.exit('Run was not completed successfully')
                print(result)
                result['n_queues'] = n
                result['total_num_bytes'] = bytes
                result['run_number'] = i
                result['consumer_number'] = j
                result['pinning_conf'] = pinning_conf
                result['commit_hash'] = commit_hash
                df = df.append(result, ignore_index=True)

        print("Run completed")

df.to_csv(args.result_file, index=False)
print('All runs completed')

