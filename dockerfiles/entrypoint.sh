#!/bin/bash
set -e

echo "[ -r /usr/share/bash-completion/bash_completion   ] && . /usr/share/bash-completion/bash_completion" >> /root/.bashrc

if [ -d "/usr/local/cuda/bin" ]; then
    echo "Found nvidia"
    export PATH=${PATH}:"/usr/local/cuda/bin":"/usr/projects/bin"
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:"/usr/lib/nvidia":"/usr/lib/projects/x86_64-linux-gnu"
fi

if [ -z "$(which setup_robotology_tdd.sh)" ] ; then
    echo "File setup_robotology_tdd.sh not found."
    exit 1
fi

source setup_robotology_tdd.sh

# If a CMD is passed, execute it
exec "$@"

