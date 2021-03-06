# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes
moo.otypes.load_types('cmdlib/cmd.jsonnet')
moo.otypes.load_types('rcif/cmd.jsonnet')
moo.otypes.load_types('appfwk/cmd.jsonnet')
moo.otypes.load_types('appfwk/app.jsonnet')
moo.otypes.load_types('daqmoduletest/conf.jsonnet')

# Import new types
import dunedaq.cmdlib.cmd as bcmd # base command,
import dunedaq.appfwk.cmd as cmd # AddressedCmd,
import dunedaq.appfwk.app as app # AddressedCmd,
import dunedaq.rcif.cmd as rc # Addressed run control Cmd,
import dunedaq.daqmoduletest.conf as daq_test_conf

from appfwk.utils import mcmd, mrccmd, mspec

import json
import math

def generate(
        QUEUE_PAIRS = 1,
        BYTES_TO_SEND = 4096,
        OUTPUT_DIR = "output"
    ):

    # Define modules and queues
    queue_bare_specs = []
    for i in range(QUEUE_PAIRS):
        queue_bare_specs.append(app.QueueSpec(inst="queue_"+str(i), kind="FollySPSCQueue", capacity=100))

    # Only needed to reproduce the same order as when using jsonnet
    queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))

    mod_specs = []
    for i in range(QUEUE_PAIRS):
        mod_specs.append(mspec("prod_"+str(i), "RandomProducer", [app.QueueInfo(name="outputQueue", inst="queue_"+str(i), dir="output")]))
        mod_specs.append(mspec("cons_"+str(i), "Consumer", [app.QueueInfo(name="inputQueue", inst="queue_"+str(i), dir="input")]))

    init_specs = app.Init(queues=queue_specs, modules=mod_specs)

    jstr = json.dumps(init_specs.pod(), indent=4, sort_keys=True)
    print(jstr)

    initcmd = rc.RCCommand(
        id=bcmd.CmdId("init"),
        entry_state=rc.State("NONE"),
        exit_state=rc.State("INITIAL"),
        data=init_specs
    )

    startcmd = mrccmd("start", "INITIAL", "RUNNING", [
                (".*", daq_test_conf.Conf(message_size=BYTES_TO_SEND, output_dir=OUTPUT_DIR))
            ])

    stopcmd = mrccmd("stop", "RUNNING", "INITIAL", [
                (".*", None),
            ])

    start_measurement_cmd = mrccmd("start_measurement", "RUNNING", "MEASURING", [
        (".*", None),
    ])

    stop_measurement_cmd = mrccmd("stop_measurement", "MEASURING", "RUNNING", [
        (".*", None),
    ])


    jstr = json.dumps(initcmd.pod(), indent=4, sort_keys=True)
    print(jstr)


    # Create a list of commands
    cmd_seq = [initcmd, startcmd, stopcmd, start_measurement_cmd, stop_measurement_cmd]

    # Print them as json (to be improved/moved out)
    jstr = json.dumps([c.pod() for c in cmd_seq], indent=4, sort_keys=True)
    return jstr

# Generate a suitable pinning for epdtdi05
def generate_pinfile(
        QUEUE_PAIRS = 1,
        PINNING_CONF = 'epdtdi105_neighboring'
    ):
    pinnings = {}
    if (PINNING_CONF == 'epdtdi105_neighboring'):
        for i in range(QUEUE_PAIRS):
                pinnings['prod_'+str(i)] = [32+i*2]
                pinnings['cons_'+str(i)] = [33+i*2]
    elif (PINNING_CONF == 'epdtdi105_same_core'):
        for i in range(QUEUE_PAIRS):
                pinnings['prod_'+str(i)] = [2*i+1]
                pinnings['cons_'+str(i)] = [2*i+33]
    elif (PINNING_CONF == 'ascending'):
        for i in range(QUEUE_PAIRS):
                pinnings['prod_'+str(i)] = [2*i]
                pinnings['cons_'+str(i)] = [2*i+1]

    jstr = json.dumps(pinnings, indent=4)
    return jstr

if __name__ == '__main__':
    CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

    import click

    @click.command(context_settings=CONTEXT_SETTINGS)
    @click.option('-q', '--queue-pairs', default=1)
    @click.option('-b', '--bytes_to_send', default=4096)
    @click.option('-o', '--output_dir', default='output')
    @click.option('-p', '--pinning_conf', default="no_pinning")
    @click.argument('json_file', type=click.Path(), default='app.json')
    def cli(queue_pairs, bytes_to_send, output_dir, pinning_conf, json_file):
        """
          JSON_FILE: Input raw data file.
          JSON_FILE: Output json configuration file.
        """

        with open(json_file, 'w') as f:
            f.write(generate(
                    QUEUE_PAIRS = queue_pairs,
                    BYTES_TO_SEND = bytes_to_send,
                    OUTPUT_DIR = output_dir
                ))

        if (pinning_conf != 'no_pinning'):
            with open('pinnings.json', 'w') as f:
                f.write(generate_pinfile(QUEUE_PAIRS=queue_pairs, PINNING_CONF=pinning_conf))

        print(f"'{json_file}' generation completed.")

    cli()