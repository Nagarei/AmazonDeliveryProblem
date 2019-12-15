//���T�v
//����n�_S�̍��W���^�����܂��B
//N���^�����܂��B
//N�́A�ו��̏W�גn�_�̍��W�Ɣz�B�n�_�̍��W�̑g�ATask[N]���^�����܂��B
//�����Ɏ��Ă�ו��̏��M���^�����܂��B
//�n�_S����J�n���A�S�Ẳו���z�B���A�n�_S�ɖ߂铹�̂�̋������o���邾���Z�����Ă��������B
//
//���ڍאݒ�
//���W�͓񎟌����W�A���W�Ԃ̋����̓��[�N���b�h�����̎l�̌ܓ��Ƃ��܂��B
//�J�n���_�ł͂ǂ̉ו��������Ă��܂���B
//����ו��̏W�גn�_�ɂ����āA�����ɋ���Ƃ��ɁA�������Ă���ו���M�����Ȃ�΁A���̉ו��������Ƃ��o���܂��B
//����ו��̔z�B�n�_�ɂ����āA�����ɋ���Ƃ��ɁA���̉ו��������Ă���Ȃ�΁A���̉ו���z�B���邱�Ƃ��o���܂��B�z�B����Ƃ��̉ו��������Ă��Ȃ���ԂɂȂ�܂��B
//
//����
//1 <= N <= 10 ^ 3
//1 <= M <= 5
//���W�̐�Βl��10 ^ 9�ȉ�
//
//���s���Ԑ���
//10sec
//����������
//�l���ĂȂ�

#if 1
#include <iostream>
#include <fstream>
#include <cstdint>
#include "solver.h"

int32_t N;
int32_t M;
int64_t start[2];
int64_t tasks[N_MAX][2][2];

int main()
{
	std::ifstream in("input.txt");
	in >> start[0] >> start[1];
	in >> N;
	for (int32_t i = 0; i < N; i++)
	{
		in >> tasks[i][0][0] >> tasks[i][0][1];
		in >> tasks[i][1][0] >> tasks[i][1][1];
	}
	in >> M;
	in.close();
	
	auto result = solver_beamsearch();

	std::ofstream out("out.txt");
	for (auto& i : result) {
		if (result[0] != i) {
			out << ' ';
		}
		out << result[i];
	}

	return 0;
}
#endif
