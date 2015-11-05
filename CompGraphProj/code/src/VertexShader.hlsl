cbuffer ConstantBuffer : register( b0 ) {
    matrix ModelViewProjection;
};


struct VsInput {
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};


struct VsOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};


VsOutput main( VsInput In ) {
    VsOutput Out;
    Out.pos = mul( In.pos, ModelViewProjection );

    // get some color from texture coords
	Out.color = float4( In.uv.x, In.uv.y, 0.5, 1 );
	return Out;
}
