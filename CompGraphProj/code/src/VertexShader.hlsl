float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return float4(pos.x-0.5, pos.y, pos.z, pos.w);
}