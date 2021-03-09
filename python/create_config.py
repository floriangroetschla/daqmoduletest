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

# Import new types
import dunedaq.cmdlib.cmd as bcmd # base command,
import dunedaq.appfwk.cmd as cmd # AddressedCmd,
import dunedaq.appfwk.app as app # AddressedCmd,
import dunedaq.rcif.cmd as rc # Addressed run control Cmd,

from appfwk.utils import mcmd, mrccmd, mspec

import json
import math

def generate(
        QUEUE_PAIRS = 1
    ):

    # Define modules and queues
    queue_bare_specs = []
    for i in range(QUEUE_PAIRS):
        queue_bare_specs.append(app.QueueSpec(inst="queue_"+str(i), kind="FollySPSCQueue", capacity=1))

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
                (".*", None),
            ])

    stopcmd = mrccmd("stop", "RUNNING", "INITIAL", [
                (".*", None),
            ])


    jstr = json.dumps(initcmd.pod(), indent=4, sort_keys=True)
    print(jstr)


    # Create a list of commands
    cmd_seq = [initcmd, startcmd, stopcmd]

    # Print them as json (to be improved/moved out)
    jstr = json.dumps([c.pod() for c in cmd_seq], indent=4, sort_keys=True)
    return jstr

if __name__ == '__main__':
    CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

    import click

    @click.command(context_settings=CONTEXT_SETTINGS)
    @click.option('-q', '--queue-pairs', default=1)
    @click.argument('json_file', type=click.Path(), default='app.json')
    def cli(queue_pairs, json_file):
        """
          JSON_FILE: Input raw data file.
          JSON_FILE: Output json configuration file.
        """

        with open(json_file, 'w') as f:
            f.write(generate(
                    QUEUE_PAIRS = queue_pairs
                ))

        print(f"'{json_file}' generation completed.")

    cli()