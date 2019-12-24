#include "Estimater.h"
#include "solver.h"
#include <vector>
#include <execution>
#include <algorithm>
#include <array>
#include "utility.h"

void Prim_impl(std::vector<int32_t> remv, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum, std::vector<std::vector<TreeIndexType>>& result, std::vector<std::pair<int64_t, int32_t>>& dist_from_tree)
{
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

	return;
}
std::vector<std::vector<TreeIndexType>> Prim(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum)
{
	std::vector<std::vector<TreeIndexType>> result(remvmax);
	std::vector<std::pair<int64_t, int32_t>> dist_from_tree(remvmax, { INF,-1 });
	Prim_impl(std::move(remv), distance_, out_distsum, result, dist_from_tree);
	return std::move(result);
}
void Prim_threadunsafe(std::vector<int32_t> remv, int64_t& out_distsum, std::vector<std::vector<TreeIndexType>>& result)
{
	static std::vector<std::pair<int64_t, int32_t>> dist_from_tree(2 * N, { INF,-1 });
	if (result.empty()) {
		result.resize(2 * N);
	}
	std::for_each(std::execution::par_unseq, result.begin(), result.end(), [&](std::vector<TreeIndexType>& v) {
		v.clear();
		});
	std::fill(std::execution::par_unseq, dist_from_tree.begin(), dist_from_tree.end(), std::pair<int64_t, int32_t>{ INF, -1 });
	Prim_impl(std::move(remv), { (int64_t*)distance, sizeof(distance) / sizeof(*distance), sizeof(*distance) / sizeof(**distance) }, out_distsum, result, dist_from_tree);
}
void Prim_threadunsafe2(std::vector<int32_t> remv, int64_t& out_distsum, std::vector<TreeIndexType>& result, std::vector<int32_t>& v_to_stnum)
{
	static std::vector<std::pair<int64_t, int32_t>> dist_from_tree(2 * N, { INF,-1 });
	static std::vector<std::vector<TreeIndexType>> result_2d(2 * N);
	std::fill(std::execution::par_unseq, dist_from_tree.begin(), dist_from_tree.end(), std::pair<int64_t, int32_t>{ INF,-1 });
	std::for_each(std::execution::par_unseq, result_2d.begin(), result_2d.end(), [&](std::vector<TreeIndexType>& v) {
		v.clear();
		});
	Prim_impl(std::move(remv), { (int64_t*)distance, sizeof(distance) / sizeof(*distance), sizeof(*distance) / sizeof(**distance) }, out_distsum, result_2d, dist_from_tree);
	result.clear();
	result.reserve(2 * N);
	if (v_to_stnum.empty()) {
		v_to_stnum.resize(2 * N + 1);
	}
	for (size_t i = 0; i < 2*N; i++) {
		v_to_stnum[i] = result.size();
		for (auto& v : result_2d[i]) {
			result.push_back(v);
		}
	}
	v_to_stnum[2 * N] = (int32_t)result.size();
}

void Prim_pair_impl(std::vector<int32_t> remv, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum,/* std::vector<std::vector<TreeIndexType>>& result,*/ std::vector<std::pair<int64_t, int32_t>>& dist_from_tree)
{
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
		//if (from != dist_from_tree[from].second) {
		//	result[from].emplace_back((int16_t)dist_from_tree[from].second);
		//	result[dist_from_tree[from].second].emplace_back((int16_t)from);
		//}
		//for(const auto& to : remv){
		std::for_each(std::execution::par_unseq, remv.begin(), remv.end(), [&](const int32_t& to) {
			if (distance_[from][to] < dist_from_tree[to].first) {
				dist_from_tree[to].first = distance_[from][to];
				dist_from_tree[to].second = from;
			}
			//}
		});

		//強制的にペアを採用
		const auto pair = (from % 2 == 0) ? (from + 1) : (from - 1);
		for (auto iter = remv.begin(), iter_end = remv.end(); iter != iter_end; ++iter)
		{
			if (*iter == pair) {
				//コピペ
				if (iter != --remv.end()) {
					std::swap(*iter, remv.back());
				}
				auto from2 = remv.back();
				remv.pop_back();
				//out_distsum += dist_from_tree[from2].first;
				out_distsum += distance_[from][from2];
				//if (from2 != dist_from_tree[from2].second) {
				//	result[from2].emplace_back((int16_t)dist_from_tree[from2].second);
				//	result[dist_from_tree[from2].second].emplace_back((int16_t)from2);
				//}
				//for(const auto& to : remv){
				std::for_each(std::execution::par_unseq, remv.begin(), remv.end(), [&](const int32_t& to) {
					if (distance_[from2][to] < dist_from_tree[to].first) {
						dist_from_tree[to].first = distance_[from2][to];
						dist_from_tree[to].second = from2;
					}
				//}
				});
				--COUNT;
				break;
			}
		}
	}

	return;
}
void Prim_pair(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum)
{
	//std::vector<std::vector<TreeIndexType>> result(remvmax);
	std::vector<std::pair<int64_t, int32_t>> dist_from_tree(remvmax, { INF,-1 });
	Prim_pair_impl(std::move(remv), distance_, out_distsum, /*result,*/ dist_from_tree);
	//return std::move(result);
}
//void Prim_threadunsafe2_pair(std::vector<int32_t> remv, int64_t& out_distsum, std::vector<TreeIndexType>& result, std::vector<int32_t>& v_to_stnum)
//{
//	static std::vector<std::pair<int64_t, int32_t>> dist_from_tree(2 * N, { INF,-1 });
//	static std::vector<std::vector<TreeIndexType>> result_2d(2 * N);
//	std::fill(std::execution::par_unseq, dist_from_tree.begin(), dist_from_tree.end(), std::pair<int64_t, int32_t>{ INF, -1 });
//	std::for_each(std::execution::par_unseq, result_2d.begin(), result_2d.end(), [&](std::vector<TreeIndexType>& v) {
//		v.clear();
//		});
//	Prim_pair_impl(std::move(remv), { (int64_t*)distance, sizeof(distance) / sizeof(*distance), sizeof(*distance) / sizeof(**distance) }, out_distsum, result_2d, dist_from_tree);
//	result.clear();
//	result.reserve(2 * N);
//	if (v_to_stnum.empty()) {
//		v_to_stnum.resize(2 * N + 1);
//	}
//	for (size_t i = 0; i < 2 * N; i++) {
//		v_to_stnum[i] = result.size();
//		for (auto& v : result_2d[i]) {
//			result.push_back(v);
//		}
//	}
//	v_to_stnum[2 * N] = (int32_t)result.size();
//}

int64_t Bruteforce::get_next_bruteforce_dp(int8_t have, size_t posi, uint32_t mask, const bool(&finished)[2 * N_MAX])
{
	const auto& remv = *prevdata.remv_ptr;
	auto& dp = *prevdata.dp_ptr;
	auto pos = remv[posi];
	if (mask == ((1ui32<< remv.size())-1)) {
		return distance[2 * N][pos];
	}
	auto& memo = dp[mask][have][posi];
	if (memo != -1) {
		return memo;
	}

	int64_t res = INF;
	for (size_t i = 0; i < remv.size(); i++)
	{
		if (mask & (1<<i)) { continue; }
		if (finished[remv[i]]) { continue;  }

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
			if (next_have <= 0) {
				continue;
			}
			--next_have;
		}
		res = std::min(res, distance[pos][remv[i]] + get_next_bruteforce_dp(next_have, i, mask | (1 << i), finished));
	}
	return memo = res;
}

Bruteforce Bruteforce::Init(Data&& prevdata_, int8_t have, const std::vector<int32_t>& remv, const bool(&finished)[2 * N_MAX])
{
	Bruteforce res;
	res.prevdata = std::move(prevdata_);
	if (res.prevdata.dp_ptr) { return std::move(res); }

	assert(remv.size() <= 20);
	std::array<std::array<int64_t, 20>, M_MAX+1> init_value;
	for (size_t j = 0; j <= M_MAX; j++) {
		for (size_t k = 0; k < 20; k++) {
			init_value[j][k] = -1;
		}
	}
	res.prevdata.dp_ptr = std::make_shared<decltype(res.prevdata.dp_ptr)::element_type>(size_t(1) << remv.size(), init_value);
	res.prevdata.remv_ptr = std::make_shared<decltype(res.prevdata.remv_ptr)::element_type>(remv);
	for (size_t i = 0; i < (size_t(1) << remv.size()); i++) {
		for (int8_t j = 0; j <= M; j++) {
			for (int32_t k = 0; k < remv.size(); k++) {
				res.get_next_bruteforce_dp(j, k, (uint32_t)i, finished);
			}
		}
	}

	for (size_t i = 0; i < remv.size(); i++)
	{
		int8_t next_have = have;
		if ((remv[i] & 1) == 0) {
			//from
			if (next_have >= M) {
				continue;
			}
			++next_have;
		}
		else {
			//to
			if (!finished[remv[i] - 1]) {
				continue;
			}
			--next_have;
		}
		res.get_next_bruteforce_dp(next_have, i, (uint32_t)(1)<<i, finished);
	}
	return std::move(res);
}
int64_t Bruteforce::get_next_bruteforce_impl(int8_t prevstate_have, const bool(&prevstate_finished)[2 * N_MAX], int32_t nextv)const
{
	auto& remv = *(prevdata.remv_ptr);
	uint32_t mask = 0;
	int8_t next_have = prevstate_have;
	size_t posi = 0;
	for (size_t i = 0; i < remv.size(); i++)
	{
		if (prevstate_finished[remv[i]]) {
			mask |= (1 << i);
		}
		else if (remv[i] == nextv) {
			if ((nextv & 1) == 0) {
				++next_have;
			}
			else {
				--next_have;
			}
			posi = i;
			mask |= (1 << i);
			//return get_next_bruteforce_dp(next_have, i, (1 << i), prevstate_finished);
		}
	}
	auto& dp = *(prevdata.dp_ptr);
	return dp[mask][next_have][posi];
}
