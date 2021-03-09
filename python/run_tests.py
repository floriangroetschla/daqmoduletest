#!/usr/bin/env python
# coding: utf-8

import os, string
from subprocess import Popen, PIPE
import pexpect

# config parameters
bytes_total =  2097152
num_queues = 1

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

# Generate configuration
print('Generate config')
bytes_per_queue = int(bytes_total / num_queues)
child.sendline('python3 sourcecode/daqmoduletest/python/create_config.py -q ' + str(num_queues) + ' -b ' + str(bytes_per_queue))
child.expect('generation completed.')

# Run the program
print('Run program')
child.sendline('daq_application -n ciao -c stdin://./app.json')
child.expect('Available commands: | init | start | stop')
child.sendline('init')
child.expect('Command init execution resulted with: 1 OK')
child.sendline('start')
#child.expect('Command start execution resulted with: 1 OK')
child.expect('"completed": true,')
child.sendline('stop')
child.expect('Command stop execution resulted with: 1 OK')
child.sendline('\003')


print('Tests completed')

