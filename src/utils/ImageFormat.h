#pragma once

#include <string>
#include <string_view>

inline std::string ImageMimeType(std::string_view imageFormat)
{
	return imageFormat == "jpg" ? "image/jpeg" : "image/" + std::string(imageFormat);
}
