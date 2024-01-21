void kernel Block_UpdateSpeed(const int width, global float3* wave){
	private float left = wave[get_global_id(0) - 1].z;
	private float up = wave[get_global_id(0) - width].z;
	private float right = wave[get_global_id(0) + 1].z;
	private float down = wave[get_global_id(0) + width].z;
	private float sum = (left + down + right + up);
	wave[get_global_id(0)].y += ((sum / 4 - wave[get_global_id(0)].z) / wave[get_global_id(0)].x);
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
	wave[get_global_id(0)].z += wave[get_global_id(0)].y;
}
