//問題概要
//ある地点Sの座標が与えられます。
//Nが与えられます。
//N個の、荷物の集荷地点の座標と配達地点の座標の組、Task[N]が与えられます。
//同時に持てる荷物の上限Mが与えられます。
//地点Sから開始し、全ての荷物を配達し、地点Sに戻る道のりの距離を出来るだけ短くしてください。
//
//問題詳細設定
//座標は二次元座標、座標間の距離はユークリッド距離の四捨五入とします。
//開始時点ではどの荷物も持っていません。
//ある荷物の集荷地点において、そこに居るときに、今持っている荷物がM個未満ならば、その荷物を持つことが出来ます。
//ある荷物の配達地点において、そこに居るときに、その荷物を持っているならば、その荷物を配達することが出来ます。配達するとその荷物を持っていない状態になります。
//
//制約
//1 <= N <= 10 ^ 3
//1 <= M <= 5
//座標の絶対値は10 ^ 9以下
//
//実行時間制限
//10sec
//メモリ制限
//考えてない

#if 1
#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include "solver.h"

int32_t N;
int32_t M;
int64_t start[2];
int64_t tasks[N_MAX][2][2];

constexpr const char* const INPUT_FILE = "5input.txt";
constexpr const char* const OUTPUT_FILE = "5output.txt";

int main()
{
	std::ifstream in(INPUT_FILE);
	if (!in) {
		std::cerr << "NO INPUT" << std::endl;
		return 0;
	}
	in >> start[0] >> start[1];
	in >> N;
	for (int32_t i = 0; i < N; i++)
	{
		in >> tasks[i][0][0] >> tasks[i][0][1];
		in >> tasks[i][1][0] >> tasks[i][1][1];
	}
	in >> M;
	in.close();
	auto starttime = std::chrono::system_clock::now();
	auto result = solver_beamsearch();
	auto finish = std::chrono::system_clock::now();

	std::cout << "COMPLETE" << std::endl;
	std::cout << result.size() << std::endl;
	std::ofstream out(OUTPUT_FILE);
	for (auto& v : result) {
		if (result[0] != v) {
			out << ' ';
		}
		out << v;
	}
	std::cout << "TIME: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - starttime).count() << "ms" << std::endl;

	return 0;
}
#endif
