#pragma once

struct POSITION
{
	float X;
	float Y;

	POSITION() : X(0.f), Y(0.f) {};
	POSITION(float x, float y) : X(x), Y(y) {};
};

struct RectF
{
	float left;
	float top;
	float right;
	float bottom;


	RectF() : left(0.f), top(0.f), right(0.f), bottom(0.f) {};
	RectF(float l, float t, float r, float b) :
		left(l), top(t), right(r), bottom(b) {};
};