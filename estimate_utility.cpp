#include "Estimater.h"
#include "solver.h"
#include <vector>
#include <execution>
#include <algorithm>
#include <array>
#include "utility.h"

std::vector<std::vector<TreeIndexType>> Prim(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum)
{
	std::vector<std::vector<TreeIndexType>> result(remvmax);
	std::vector<std::pair<int64_t, int32_t>> dist_from_tree(remvmax, { INF,-1 });
	dist_from_tree[remv.back()].first = 0;
	dist_from_tree[remv.back()].second = remv.back();
	out_distsum = 0;
	for (size_t COUNT = remv.size(); COUNT > 0; --COUNT)
	{
		auto from_iter = std::min_element(std::execution::par_unseq, remv.begin(), remv.end(), [&](int32_t a, int32_t b) {
			return dist_from_tree[a].first < dist_from_tree[b].first;
		});
		if (from_iter != --remv.end()) {
			std::swap(*from_iter, remv.back());
		}
		//for (size_t i = 0; i < remv.size(); ++i)
		//{
		//	if (dist_from_tree[remv[i]] < mincost) {
		//		mincost = dist_from_tree[remv[i]];
		//		index = i;
		//	}
		//}
		//if (index != remv.size() - 1) {
		//	std::swap(remv[index], remv.back());
		//}
		auto from = remv.back();
		remv.pop_back();

		out_distsum += dist_from_tree[from].first;
		if (from != dist_from_tree[from].second) {
			result[from].emplace_back((int16_t)dist_from_tree[from].second);
			result[dist_from_tree[from].second].emplace_back((int16_t)from);
		}
		//for(const auto& to : remv){
		std::for_each(std::execution::par_unseq, remv.begin(), remv.end(), [&](const int32_t& to) {
			if (distance_[from][to] < dist_from_tree[to].first) {
				dist_from_tree[to].first = distance_[from][to];
				dist_from_tree[to].second = from;
			}
		//}
		});
	}

	return std::move(result);
}

int64_t get_next_bruteforce_dp(int8_t have, size_t posi, uint32_t mask, const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX], std::vector<std::array<std::array<int64_t, 20>, M_MAX>>& dp)
{
	auto pos = remv[posi];
	if (mask == ((1ui32<< remv.size())-1)) {
		return distance[2 * N][pos];
	}
	auto& memo = dp[mask][have][posi];
	if (memo != -1) {
		return memo;
	}
	memo = INF;
	for (size_t i = 0; i < remv.size(); i++)
	{
		if (mask & (1<<i)) { continue; }

		int8_t next_have = have;
		if ((remv[i] & 1) == 0) {
			//from
			if (have >= M) {
				continue;
			}
			++next_have;
		}
		else {
			//to
			if (!finished[remv[i] - 1]) {
				bool visited = false;
				for (size_t j = 0; j < remv.size(); j++)
				{
					if (mask & (1 << j)) {
						if (remv[j] == remv[i] - 1) {
							visited = true;
							break;
						}
					}
				}
				if (!visited) {
					continue;
				}
			}
			--next_have;
		}
		memo = std::min(memo, distance[pos][remv[i]] + get_next_bruteforce_dp(next_have, i, mask | (1 << i), remv, finished, dp));
	}
	return memo;
}
int64_t get_next_bruteforce_impl(int8_t have, int32_t , const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX], int32_t nextv)
{
	assert(remv.size() <= 20);
	std::array<std::array<int64_t, 20>, M_MAX> init_value;
	for (size_t j = 0; j < M_MAX; j++) {
		for (size_t k = 0; k < 20; k++) {
			init_value[j][k] = -1;
		}
	}
	std::vector<std::array<std::array<int64_t,20>, M_MAX>> dp(size_t(1) << remv.size(), init_value);
	for (size_t i = 0; i < remv.size(); i++)
	{
		if (remv[i] == nextv) {
			int8_t next_have = have;
			if ((nextv & 1) == 0) {
				++next_have;
			}
			else {
				--next_have;
			}
			return get_next_bruteforce_dp(next_have, i, (1 << i), remv, finished, dp);
		}
	}
	assert(false);
	return 0;
}
