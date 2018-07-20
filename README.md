# linto-audio-recorder

The aim of this project is to produce a functional module able to record voice in real-time on a Raspberry type embedded system.
It is able to retrieve data extracted from the microphone and save the binary flow in a file or a named pipe.
It communicates with the system via an MQTT local bus.

### Before starting

The following libraries (and APIs) are necessary for the proper functioning of the program:

* Pulse Audio : <https://freedesktop.org/software/pulseaudio/doxygen/index.html>
* PAHO MQTT : <https://github.com/eclipse/paho.mqtt.c>
* JSMN in C : <https://github.com/zserge/jsmn>

See INSTALL.md.

### Compilation

Use only the "make" command on the target in the corresponding folder.

### Use

Currently, this program uses Pulse Audio default device. You can easily change it with
```
pacmd set-default-source <PA name>
```
To list the available input options, use
```
pacmd list-sources
```

Configuration data are available in the config.conf file.
To start the program, just run the Python script using
```
/usr/bin/python3 audio_recorder.py --mode X
```

```
./audio_recorder.py --mode X
```

X is <file> or <pipe>.
