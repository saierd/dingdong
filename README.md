A doorbell and intercom solution running on Raspberry Pi.

## Features

* Ringing the door from another instance or with a switch.
* Intercom system.
* Touchscreen interface.
* Access control using numeric keys and RFID tags.
* Using a PIR motion sensor for controlling the screen saver.

## Hardware Requirements

Each instance needs a Raspberry Pi with the following accessories. The software was currently tested using the Raspberry Pi 3 B+ and Raspberry Pi 4. Other models might or might not work.

- Raspbian Buster (earlier versions don't work).
- Connection to a local network.
- An official Raspberry Pi 7" touchscreen.
- A USB webcam with a microphone.
- A speaker on the Raspberry Pi audio jack.
- An RC522 RFID scanner (optional).
- A PIR motion sensor (optional).
- A relay shield from Sequent Microsystems (optional).

## Software Setup

### Cross-Compilation

The `build` directory contains a Docker image and a CMake toolchain file with all necessary dependencies to cross-compile the project for a Raspberry Pi. To build a `deb` package, simply run

```bash
./build/build.sh
```

### Installation

To install the project on a Raspberry Pi, first download and flash a recent [Raspbian](https://www.raspberrypi.org/downloads/raspbian/) to it (Raspbian Buster is required). Connect the Raspberry Pi to a network and install the deb package to it.

For convenience, there are some scripts in the `build` directory to simplify deployment over SSH. To use them, you will need to enable SSH on the Raspberry Pi by executing

```bash
sudo systemctl enable ssh
sudo systemctl start ssh
```

Note that enabling SSH might be a security issue. You should at least change the password of the `pi` user. Deploy your SSH key to the device using

```bash
ssh-copy-id pi@<IP ADDRESS>
```

on the host. You can then build and install a deb package by calling a single script:

```bash
./build/install_remote.sh pi@<IP ADDRESS>
```

The project is installed as a `systemd` service. It is enabled by default and will automatically start after a restart. You can manually execute

```bash
sudo systemctl start/stop dingdong.service
```

to control the service. The settings can be edited in `/opt/dingdong/settings/settings.json`.

### Dependencies

For build dependencies see the Dockerfile in the `build` directory. Building for x64 (for debugging purposes) requires the `amd64` version of the same libraries whose `armhf` version gets installed in the Docker image.

Runtime dependencies are declared in the `deb` package. In addition to the libraries required at build time these are:

- `gstreamer1.0-omx-rpi` for hardware accelerated video encoding.
- `gstreamer1.0-pulseaudio` for using PulseAudio sinks and sources in `gstreamer`. As a `gstreamer` plugin, this is not a shared library dependency, but only loaded at runtime.
- `pulseaudio` is used for all audio functionality in the project. Launch scripts assume that they can control audio settings through `pactl` and with PulseAudio environment variables.
- `sox` for the `play` binary that is used to generate beep tones.
- `wiringpi` for the `gpio` binary.
- `x11-xserver-utils` for the `xset` command that is used to control the screensaver.

## Hardware Setup

### Touchscreen

Connect the Raspberry Pi touchscreen as described in https://www.element14.com/community/docs/DOC-78156.

### RFID Scanner

The application supports using an RC522 RFID scanner with 13,56MHz RFID tags for access control.

Connect it as described in https://github.com/ondryaso/pi-rc522. For a reference with images you can also see https://pimylifeup.com/raspberry-pi-rfid-rc522/, but don't forget the IRQ pin which is not connected there.

### GPIO Actions

You can set up actions in the settings file. These can be executed after entering a key codes or scanning an RFID chip. For each action, add an entry to the `actions` object in the settings file.

```json
"actions": {
    "key": {
      "caption": "Some Action",
      "pin": 1,
      "duration": 1000,
      "order": 1,
      "remote": false
    }
}
```

Actions can have the following properties:

- The key of the action must be a unique name and is used to reference the action in the settings.
- `pin` is the number of a GPIO pin that gets set to high when the action is executed. See https://pinout.xyz for the pin layout of the Raspberry Pi. We use the BCM numbers of the pins.
- `duration` is the duration (in milliseconds) the pin gets set to high.
- `order` is a number that controls the order in which actions get shown on screen.
- If `remote` is set to `true`, this action can be executed from another instance while it has an open call with the instance of the action. This can be used to e.g. open the front door remotely.

You can also use a relay board from Sequent Microsystems (documentation can be found [here](https://www.robotshop.com/media/files/content/s/sqm/pdf/sequent-microsystems-8-relay-expansion-hat-raspberry-pi.pdf)). At the moment just one of them is supported using the address 0 (no jumpers set). An action's pin number refers to an output of the relay board instead of a GPIO pin when you add a flag `relay` to the action definition and set it to `true`.

### Ring Button

To allow ringing with a button (e.g. from your apartment door), connect a GPIO pin to ground through the button. Specify the pin and a ringtone for when the button gets pressed in the settings file.

### Motion Sensor

The application can control the screen saver with a PIR motion sensor. Connect it to any pin as described in https://thepihut.com/blogs/raspberry-pi-tutorials/raspberry-pi-gpio-sensing-motion-detection and enter the pin number in the settings file.

## License

TODO

This repository includes some external dependencies, which are licensed under different terms. See the license and readme files in the subdirectories.
