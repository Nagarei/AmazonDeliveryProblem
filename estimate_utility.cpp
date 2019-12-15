#include "Estimater.h"
#include "solver.h"
#include <vector>
#include <execution>
#include <algorithm>
#include "utility.h"

std::vector<std::vector<TreeIndexType>> Prim(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum)
{
	std::vector<std::vector<TreeIndexType>> result(remvmax);
	std::vector<std::pair<int64_t, int32_t>> dist_from_tree(remvmax, { INF,-1 });
	dist_from_tree[remv.back()].first = 0;
	out_distsum = 0;
	for (size_t COUNT = 0; COUNT < remv.size(); COUNT++)
	{
		auto from_iter = std::min_element(std::execution::par_unseq, remv.begin(), remv.end(), [&](int32_t a, int32_t b) {
			return dist_from_tree[a].first < dist_from_tree[b].first;
		});
		if (from_iter != remv.end()) {
			std::iter_swap(from_iter, remv.rbegin().base());
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
		result[from].emplace_back((int16_t)dist_from_tree[from].second);
		result[dist_from_tree[from].second].emplace_back((int16_t)from);
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