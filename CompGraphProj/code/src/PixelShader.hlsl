struct VsOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};


float4 main( VsOutput In ) : SV_Target {
	return In.color;
}
