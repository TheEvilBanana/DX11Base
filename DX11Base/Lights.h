#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct DirectionalLight {
	XMFLOAT4 diffuseColor;
	XMFLOAT3 direction;

	void SetLightValues(XMFLOAT4 _diffuseColor, XMFLOAT3 _direction) {
		diffuseColor = _diffuseColor;
		direction = _direction;
	}
};

struct AmbientLight {
	XMFLOAT4 ambientColor;

	void SetLightValues(XMFLOAT4 _ambientColor) {
		ambientColor = _ambientColor;
	}
};