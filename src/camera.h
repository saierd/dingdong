#pragma once

#include <cstdint>
#include <vector>

// Returns an image from the camera, encoded as JPEG. If there is no camera or an error, this return an empty array.
std::vector<std::uint8_t> takeCameraImage();
