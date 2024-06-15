#pragma once

constexpr int WINWIDTH		= 800;
constexpr int WINHEIGHT		= 800;

constexpr int BOARDSIZE		= 20;

#define SINGTON(className)\
static className* Instance()\
{\
	static className inst;\
	return &inst;\
};