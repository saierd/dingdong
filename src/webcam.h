#pragma once

#include <string>

// Restart the webcam device. This is done asynchronously. Use the waitForWebcam function to wait until the webcam is
// available again.
void restartWebcam();

// Wait until the webcam is available again after restarting.
void waitForWebcam();

std::string getWebcamDevice();
