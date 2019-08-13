#!/usr/bin/env python3

# Reads RFID tags from an RC522 module attached to the Raspberry Pi. Prints a single line with the
# tag id for each tag that was scanned. Automatically removes duplicate ids unless the id was not
# observed for some time.

import atexit
import sys
import time

from pirc522 import RFID

duplicate_tag_timeout = 5  # Seconds.

reader = RFID()


def exit_handler():
    reader.cleanup()


def main():
    atexit.register(exit_handler)

    last_tag = None
    last_detection_time = None

    while True:
        reader.wait_for_tag()
        (error, _) = reader.request()
        if error:
            continue

        (error, uid) = reader.anticoll()
        if error:
            continue

        tag = "".join([str(x) for x in uid])
        if tag == last_tag:
            # Do not print duplicate tags, unless it was not observed for some time.
            if (time.time() - last_detection_time) < duplicate_tag_timeout:
                continue

        last_tag = tag
        last_detection_time = time.time()

        print(tag)
        sys.stdout.flush()


if __name__ == "__main__":
    main()
