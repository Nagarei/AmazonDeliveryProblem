#pragma once
#include <vector>
#include "utility.h"
#include "solver.h"

using TreeIndexType = int16_t;

class Christofides final
{
public:
	struct Data final {
	private:
		friend Christofides;
		using State = StateT<Data>;
		std::vector<std::vector<TreeIndexType>> basetree;
		int64_t treecost;
		int32_t s_to_tree_index;
		int64_t total_cost;
	public:
		Data(const StateT<Data>& state);//root初期化
		Data(Data&& estimater_data, const StateT<Data>& state);
		Christofides Init(const State& state, const std::vector<int32_t>& remv) { return { *this, state, remv }; }

		bool operator<(const Data& r)const {
			return (total_cost) < (r.total_cost);
		}
		bool operator>(const Data& r)const {
			return (total_cost) > (r.total_cost);
		}
	};
	using State = Data::State;
private:
	const Data& data;
	std::vector<int64_t> subtree_dist_data;//二次元のならし
	ArrayView<int64_t, 2> st_to_st_dist;

	//temporary in constructor
	size_t vnum;
	std::vector<int64_t> v_to_st_dist_data;
	ArrayView<int64_t, 2> v_to_st_dist;

	int64_t build_v_to_st(int nowv, int& now_index, int from);
	int64_t build_st_to_st(int nowv, int& now_index, int to_st);
public:
	Christofides(Data& data, const State& state, const std::vector<int32_t>& remv);

	//スレッドセーフであること！
	Data get_next(const State& prevstate, int32_t nextv)const;
};

std::vector<std::vector<TreeIndexType>> Prim(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum);
