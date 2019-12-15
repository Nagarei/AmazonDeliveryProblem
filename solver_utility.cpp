#include "solver.h"
#include <cmath>

int64_t distance[2 * N_MAX + 1][2 * N_MAX + 1];
void init_distance()
{
	for (int32_t i = 0; i < 2*N; i++)
		for (int32_t j = i; j < 2 * N; ++j)
	{
		const auto& p1 = tasks[i / 2][i % 2];
		const auto& p2 = tasks[j / 2][j % 2];
		const auto& p1x = p1[0];
		const auto& p1y = p1[1];
		const auto& p2x = p2[0];
		const auto& p2y = p2[1];
		distance[i][j] = distance[j][i] =
			static_cast<int64_t>(sqrt((p1x - p2x) * (p1x - p2x) + (p1y - p2y) * (p1y - p2y)) + 0.5);
	}
}

