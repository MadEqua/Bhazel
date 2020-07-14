#include "bzpch.h"

#include "Camera.h"
#include "Core/Utils.h"


namespace BZ {

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float near, float far) {
    this->left = left;
    this->right = right;
    this->bottom = bottom;
    this->top = top;
    this->near = near;
    this->far = far;

    projectionMatrix = Utils::ortho(left, right, bottom, top, near, far);
}


PerspectiveCamera::PerspectiveCamera(float fovy, float aspectRatio, float near, float far) {
    fovy = fovy;
    aspectRatio = aspectRatio;
    near = near;
    far = far;

    projectionMatrix = Utils::perspective(fovy, aspectRatio, near, far);
}
}