layout( local_size_x = 64, local_size_y = 1, local_size_z = 1 ) in;

layout( std430, binding = 0 ) buffer MortoncodesBlock {
    ivec2 Mortoncodes[];
};

layout( std430, binding = 1 ) buffer NumBlock {
    ivec2 Range;
};

layout( std430, binding = 2 ) buffer NodesBlock {
    HlbvhNode Nodes[];
};

layout( std430, binding = 3 ) buffer LeafBlock {
    Leaf Leafs[];
};