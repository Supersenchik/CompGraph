struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

cbuffer ConstantBuffer : register(b0) {
	matrix model;
}

VS_OUTPUT main(float4 pos : POSITION, float4 color : COLOR) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul(pos, model);
	output.pos.x = output.pos.x / 1.6f;
	output.color = color;
	return output;
}