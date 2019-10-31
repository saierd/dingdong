#!/usr/bin/env python3

# Enable or disable a relay on an 8 relay board from Sequent Microsystems.

import sys

import relay8.relay8 as relay8


def main():
    relay_id = int(sys.argv[1])
    value = int(sys.argv[2])

    relay8.set(0, relay_id, value)


if __name__ == "__main__":
    main()
