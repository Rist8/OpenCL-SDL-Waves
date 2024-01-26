void kernel Block_UpdateSpeed(const int width, __global float3* wave){
	#define left wave[get_global_id(0) - 1].z
	#define up wave[get_global_id(0) - width].z
	#define right wave[get_global_id(0) + 1].z
	#define down wave[get_global_id(0) + width].z
	#define sum (left + down + right + up)
	wave[get_global_id(0)].y += ((sum / 4 - wave[get_global_id(0)].z) / wave[get_global_id(0)].x);
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
	wave[get_global_id(0)].z += wave[get_global_id(0)].y;
}
