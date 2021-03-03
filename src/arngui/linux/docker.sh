#!/bin/bash

case $1 in
build)
    docker build -t cpcsdk/arnold .
    ;;
run)
    docker run -it  \
    --privileged=true \
    --rm=true \
    -h arnoldemu \
   	-e DISPLAY=$DISPLAY \
	-v /tmp/.X11-unix:/tmp/.X11-unix \
	-v /dev/snd:/dev/snd \
	-e PULSE_SERVER="tcp:localhost:64713" \
    --volume=$(pwd):/home/arnold/arnoldemu \
    cpcsdk/arnold
    ;;
configure_host)
    # Tentative to have sound in the docker
    sudo apt-get install paprefs
    echo 'Launch PulseAudio Preferences, go to the "Network Server" tab, and check the "Enable network access to local sound devices" checkbox'
    read tmp
    sudo service pulseaudio restart
    if ! test -f ~/.ssh/id_rsa.pub
    then
        ssh-keygen
    fi
esac
