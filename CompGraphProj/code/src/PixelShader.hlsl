float4 main( float4 pos : SV_POSITION ) : SV_TARGET
{
	float fLimiter = 500.0f;
	float dist = pos.x*pos.x + pos.y*pos.y;
	dist = (dist % fLimiter) / fLimiter;
	return float4(dist, 0.0f, dist, 1.0f);
	//return float4(1.0f, 1.0f, 0.0f, 1.0f);
}