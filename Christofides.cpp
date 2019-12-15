#include "Estimater.h"
#include <algorithm>
#include <numeric>
#include <execution>
#include <cassert>

Christofides::Data::Data(const State& state)
{
	std::vector<int32_t> remv(2 * N);
	std::iota(remv.begin(), remv.end(), (int32_t)0);
	auto s_to_tree_index = *std::min_element(std::execution::par_unseq, remv.begin(), remv.end(), [](int32_t a, int32_t b) {
		return distance[2 * N][a] < distance[2 * N][b];
		});
	basetree = Prim(std::move(remv), 2 * N, { (int64_t*)distance, N_MAX, N_MAX }, treecost);

	total_cost = 2 * distance[2 * N][s_to_tree_index] + treecost;
	total_cost += state.cost;
}

Christofides::Data::Data(Data&& estimater_data, const State&)
{
	*this = std::move(estimater_data);
}



void Christofides::build_v_to_stnum(int nowv, int& now_index, int parent)
{
	const size_t size_ = data.basetree[nowv].size();
	for (size_t i = 0; i < size_; ++i) {
		const auto& e = data.basetree[nowv][i];
		if (e == parent) {
			continue;
		}
		build_v_to_stnum(e, now_index, nowv);
	}
	v_to_stnum[nowv] = now_index;
	now_index += (int)size_;
}
//@return 自身のsubtree番号
size_t Christofides::build_v_to_st(int nowv, int parent, int from)
{
	int64_t distmin = distance[from][nowv];
	int_fast16_t minpair = nowv;
	const size_t size_ = data.basetree[nowv].size();
	size_t this_index_add = size_;
	for (size_t i = 0; i < size_; ++i) {
		const auto& e = data.basetree[nowv][i];
		if (e == parent) {
			this_index_add = i;
			continue;
		}
		auto nextindex = build_v_to_st(e, nowv, from);
		if (v_to_st_dist[from][nextindex] < distmin) {
			distmin = v_to_st_dist[from][nextindex];
			minpair = v_to_st_pair[from][nextindex];
		}
	}
	auto this_index = v_to_stnum[nowv] + this_index_add;
	if (this_index_add != size_) {
		v_to_st_dist[from][this_index] = distmin;
		v_to_st_pair[from][this_index] = (int16_t)minpair;
	}
	assert(this_index_add != size_ || parent == -1);
	return this_index;
}
size_t Christofides::build_st_to_st(int nowv, int parent, int to_st)
{
	int64_t distmin = v_to_st_dist[nowv][to_st];
	std::pair<int16_t, int16_t> minpair = { (int16_t)nowv, v_to_st_pair[nowv][to_st] };
	const size_t size_ = data.basetree[nowv].size();
	size_t this_index_add = size_;
	for (size_t i = 0; i < size_; ++i) {
		const auto& e = data.basetree[nowv][i];
		if (e == parent) {
			this_index_add = i;
			continue;
		}
		auto nextindex = build_st_to_st(e, nowv, to_st);
		if (st_to_st_dist[to_st][nextindex] < distmin) {
			distmin = st_to_st_dist[to_st][nextindex];
			minpair = st_to_st_pair[to_st][nextindex];
		}
	}
	auto this_index = v_to_stnum[nowv] + this_index_add;
	if (this_index_add != size_) {
		st_to_st_dist[to_st][this_index] = distmin;
		st_to_st_pair[to_st][this_index] = minpair;
	}
	assert(this_index_add != size_ || parent == -1);
	return this_index;
}

Christofides::Christofides(Data& data, const State&, const std::vector<int32_t>& remv)
	: data(data)
	, subtree_dist_data(2 * (remv.size() - 1) * 2 * (remv.size() - 1), INF)
	, st_to_st_dist(subtree_dist_data.data(), 2 * (remv.size() - 1), 2 * (remv.size() - 1))
	, st_to_st_pair_data(subtree_dist_data.size())
	, st_to_st_pair(st_to_st_pair_data.data(), 2 * (remv.size() - 1), 2 * (remv.size() - 1))
	, v_to_stnum(2 * N)
	, v_to_st_dist_data(2 * N * 2 * (remv.size()-1), INF)
	, v_to_st_dist(v_to_st_dist_data.data(), 2 * N, 2 * (remv.size() - 1))
	, v_to_st_pair_data(v_to_st_dist_data.size())
	, v_to_st_pair(v_to_st_pair_data.data(), 2 * N, 2 * (remv.size() - 1))
{
	if (data.basetree.empty()) {
		int64_t treecost;
		//remvにmoveはつけない
		data.basetree = Prim(remv, 2 * N, { (int64_t*)distance, N_MAX, N_MAX }, treecost);
		assert(treecost == data.treecost);
	}

	{
		int index = 0;
		build_v_to_stnum(remv.back(), index, -1);
		assert(index == 2 * (remv.size() - 1));
	}
	std::for_each(std::execution::par_unseq, remv.begin(), remv.end(), [this, &remv](int32_t i) {
		build_v_to_st(i, -1, i);
		});
	std::vector<int_fast16_t> subtreeroot(2 * (remv.size() - 1));
	for (size_t v = 0; v < v_to_stnum.size(); ++v) {
		for (size_t i = 0; i < data.basetree[v].size(); i++)
		{
			subtreeroot[v_to_stnum[v] + i] = (int_fast16_t)v;
		}
	}
	std::for_each(std::execution::par_unseq, range_iterator<int32_t>(0), range_iterator<int32_t>((int32_t)(2 * (remv.size()-1))), [this, &remv, &subtreeroot](int32_t i) {
		build_st_to_st(subtreeroot[i], -1, i);
		});

	v_to_st_dist_data.clear();
	v_to_st_pair_data.clear();
}

auto Christofides::get_next(const State& prevstate, const std::vector<int32_t>& remv, int32_t nextv) const->Data
{
	Data res;
	res.treecost = prevstate.estimater.treecost;
	res.basetree = prevstate.estimater.basetree;//copy
	std::vector<int32_t> st_index;
	//木からvの辺の削除 vの削除によって生まれるsubtreeの列挙
	for(auto& nv_neighbor : data.basetree[nextv]){
		res.treecost -= distance[nextv][nv_neighbor];

		const size_t size_ = data.basetree[nv_neighbor].size();
		for (size_t i = 0; i < size_; ++i) {
			const auto& e = data.basetree[nv_neighbor][i];
			if (e == nextv) {
				st_index.emplace_back(
					v_to_stnum[nv_neighbor] + (int32_t)i
				);
				std::swap(res.basetree[nv_neighbor][i], res.basetree[nv_neighbor].back());
				res.basetree[nv_neighbor].pop_back();
				break;
			}
		}
	}
	res.basetree[nextv].clear();

	//部分木同士の最小全域木
	int64_t subtree_cost = 0;
	const auto stsize = 2 * (remv.size()-1);
	const auto& st_mst = Prim(st_index, (int32_t)stsize, st_to_st_dist, subtree_cost);
	res.treecost += subtree_cost;

	for (size_t i = 0; i < stsize; i++)
	{
		for (auto& j : st_mst[i]) {
			const auto& pair = st_to_st_pair[i][j];
			res.basetree[pair.first].push_back(pair.second);
			//res.basetree[pair.second].push_back(pair.first);//st_mst[i]に両方入ってる
		}
	}

	//端点二つ
	int64_t s_to_tree = INF;
	int64_t v_to_tree = INF;
	for (auto& v : remv) {
		if (v != nextv) {
			s_to_tree = std::min(s_to_tree, distance[2 * N][v]);
			v_to_tree = std::min(v_to_tree, distance[nextv][v]);
		}
	}

	res.total_cost = v_to_tree + s_to_tree + res.treecost;
	res.total_cost = std::max(prevstate.estimater.total_cost - prevstate.cost, res.total_cost);

	res.total_cost += prevstate.cost + distance[nextv][prevstate.pos];
	return std::move(res);
}

