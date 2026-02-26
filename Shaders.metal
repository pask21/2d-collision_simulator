#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
};

vertex VertexOut vertexShader(uint vertexID [[vertex_id]],
                              const device float4* vertexArray [[buffer(0)]]) {
    VertexOut out;
    out.position = vertexArray[vertexID];
    return out;
}

fragment float4 fragmentShader() {
    return float4(1.0, 0.0, 0.0, 1.0); // rosso
}