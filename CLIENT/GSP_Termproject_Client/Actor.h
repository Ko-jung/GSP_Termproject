#pragma once

#include <Windows.h>
#include <atlimage.h>
#include <string>

class Actor
{
public:
	Actor(bool IsPossess = true);
	virtual ~Actor() {}

	void Update(float elapsedTime);
	void Draw(HDC& memdc);

	void Move(WPARAM wParam);
	//void SetLocation(POINT location) { Location.x = location.x; Location.y = location.y; }
	//POINT GetLocation(POINT location) { Location.x = location.x; Location.y = location.y; }
	void SetLocation(float x, float y) { X = x; Y = y; }
	void GetLocation(float& x, float& y) { x = X; y = Y; }
	void SetName(const char* name) { Name = name; }

protected:
	//POINT Location;
	float X, Y;
	float Speed;
	CImage Img;

	std::string Name;
};

