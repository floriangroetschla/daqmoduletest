#!/usr/bin/env python
# coding: utf-8

import os, string
from subprocess import Popen, PIPE
import pexpect
import json
import pandas as pd

# config parameters
output_file = "output"
bytes_total =  [2**x for x in range(12,31)]
#num_queues = [1, 2, 4, 8, 16]
num_queues = [1]
num_runs = 5

child = pexpect.spawn('bash')

# Setup the environment
print('Setup environment')
child.sendline('source daq-buildtools/dbt-setup-env.sh')
child.expect("DBT setuptools loaded")
child.sendline('dbt-setup-runtime-environment')
child.expect('This script has been sourced successfully')


# Build the project
print('Build project')

child.sendline('dbt-build.sh --install')
child.expect('Installation complete.')

df = pd.DataFrame()

for n in num_queues:
    for bytes in bytes_total:
        # Generate configuration
        print('Generate config for ' + str(n) + ' queues and ' + str(bytes) + " bytes")
        bytes_per_queue = int(bytes / n)
        child.sendline('python3 sourcecode/daqmoduletest/python/create_config.py -q ' + str(n) + ' -b ' + str(bytes) + ' -o ' + str(output_file))
        child.expect('generation completed.')

        for i in range(num_runs):
            # Run the program
            print('Run program')
            child.sendline('daq_application -n ciao -c stdin://./app.json')
            child.expect('Available commands: | init | start | stop')
            child.sendline('init')
            child.expect('Command init execution resulted with: 1 OK')
            child.sendline('start')
            child.expect('"completed": true,')
            child.sendline('stop')
            child.expect('Command stop execution resulted with: 1 OK')
            child.sendline('\003')

            print('Get result')
            file = open('runs/cons_0_log.jsonl', 'r')
            lines = file.readlines()
            result = json.loads(lines[-1])
            if not result['completed']:
                print('Run was not successfull')
                exit
            print(result)
            result['n_queues'] = n
            result['num_bytes'] = bytes
            result['run_number'] = i
            df = df.append(result, ignore_index=True)

        print("Run completed")

df.to_csv('results.csv', index=False)
print('All runs completed')

