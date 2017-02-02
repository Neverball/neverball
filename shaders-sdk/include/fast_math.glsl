int pow2(in int d){
    return d <= 0 ? 1 : (2 << (d-1));
}

float pow2(in float d){
    return exp2(d);
}
