#pragma once

#define GLM_FORCE_SWIZZLE
//#define OPENCL_SUPPORT

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <ctime>
#include <chrono>
#include <array>
#include <random>
#include <memory>
#include <sstream>
#include <windows.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/component_wise.hpp"
#include "lodepng/lodepng.h"
#include "phantom/phantom.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef OPENCL_SUPPORT
#define USE_OPENCL 1
#include <vexcl/vexcl.hpp>
#include <vexcl/vector.hpp>
#else
#include <sorter.h>
#endif
