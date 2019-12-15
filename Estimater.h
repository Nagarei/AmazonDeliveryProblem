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
		int64_t total_cost;
		Data()noexcept {}
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
	ArrayView<int64_t, 2> st_to_st_dist;//subtree同士の最短距離
	std::vector<std::pair<int16_t, int16_t>> st_to_st_pair_data;//二次元のならし
	ArrayView<std::pair<int16_t, int16_t>, 2> st_to_st_pair;//st_to_st_distが何と何のペアかを保存
	std::vector<int32_t> v_to_stnum;//subtree[root_v][parent_v]のindexはv_to_stnum[root_v] + (find(basetree[root_v], parent_v) - basetree.begin())

	//temporary in constructor
	std::vector<int64_t> v_to_st_dist_data;//二次元のならし
	ArrayView<int64_t, 2> v_to_st_dist;//頂点とsubtreeの最短距離
	std::vector<int16_t> v_to_st_pair_data;//二次元のならし
	ArrayView<int16_t, 2> v_to_st_pair;//v_to_st_distが何とのペアかを保存

	void build_v_to_stnum(int nowv, int& now_index, int parent);
	size_t build_v_to_st(int nowv, int parent, int from);
	size_t build_st_to_st(int nowv, int parent, int to_st);
public:
	Christofides(Data& data, const State& state, const std::vector<int32_t>& remv);

	//スレッドセーフであること！
	Data get_next(const State& prevstate, const std::vector<int32_t>& remv, int32_t nextv)const;
};

std::vector<std::vector<TreeIndexType>> Prim(std::vector<int32_t> remv, int32_t remvmax, ArrayView<const int64_t, 2> distance_, int64_t& out_distsum);
