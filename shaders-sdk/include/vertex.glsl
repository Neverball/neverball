layout ( std430, binding=3) buffer ssbo3 {Stride3f verts[];};
layout ( std430, binding=4) buffer ssbo4 {int indics[];};
layout ( std430, binding=5) buffer ssbo5 {Stride3f norms[];};
layout ( std430, binding=6) buffer ssbo6 {Stride2f texcs[];};
layout ( std430, binding=7) buffer ssbo7 {int mats[];};
layout ( std430, binding=16) buffer ssbo16 {Stride4f vcolors[];};