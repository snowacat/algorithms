#include "stdafx.h"
#include <iostream>
#include <time.h>
#define SIZE 512

using namespace std;

// ������ ���� ��� �������� 16 - ��������� ������� ������������.��� ��������, ��� ��� ����������� � �������������� ���������� ���������� � 16 - ��������� �������.
__declspec(align(16)) float a[SIZE*SIZE];
__declspec(align(16)) float b[SIZE*SIZE];
__declspec(align(16)) float c[SIZE*SIZE];
__declspec(align(16)) float d[SIZE*SIZE];

int main()
{
	float* aMatrix = a;
	float* bMatrix = b;
	float* resMatrix = c;

	// Initialize matrix
	srand(static_cast<unsigned> (time(NULL)));
	for (int i = 0; i < SIZE*SIZE; i++) 
	{ 
		aMatrix[i] = rand() % 10 + 1;
		bMatrix[i] = rand() % 10 + 1; 
	}

	// Trans the second matrix
	for (int i = 0; i < SIZE; i++) 
	{
		for (int j = 0; j <= i - 1; j++) 
		{
			float temp = bMatrix[i*SIZE + j];
			bMatrix[i*SIZE + j] = bMatrix[j*SIZE + i];
			bMatrix[j*SIZE + i] = temp;
		}
	}

	// Calculation on CPU
	float* resMatrixCPU = d;
	clock_t CPU = clock();
	for(int i=0; i < SIZE; i++)
	{
		for(int j=0; j < SIZE; j++)
		{
			for(int k = 0; k < SIZE; k++)
			{
				resMatrixCPU[i + j * SIZE] += aMatrix[i * SIZE + k]*bMatrix[k * SIZE + j];
			}
		}
	}
	cout << "Usual CPU: " <<  clock() - CPU << endl;
	//if (SIZE <= 256)
	//{
	//	// For CPU
	//	float matrixA[SIZE][SIZE];
	//	float matrixB[SIZE][SIZE];
	//	float matrixResult[SIZE][SIZE];

	//	// Convert to usual view
	//	for (int i = 0; i < SIZE; i++)
	//	{
	//		for (int j = 0; j < SIZE; j++)
	//		{
	//			matrixA[j][i] = aMatrix[i * SIZE + j];
	//			matrixB[j][i] = bMatrix[i * SIZE + j];
	//		}
	//	}

	//	//// Output
	//	//for (int i = 0; i < SIZE; i++)
	//	//   {
	//	//       for (int j = 0; j < SIZE; j++)
	//	//           cout << matrixA[i][j] << "  ";
	//	//
	//	//       cout << endl;
	//	//   }
	//	//   cout << endl << endl;

	//	// Calculation on CPU
	//	clock_t CPU = clock();
	//	for (int i = 0; i < SIZE ; i++)					
	//	{
	//		for (int j = 0; j < SIZE; j++)			
	//			for (int k = 0; k < SIZE; k++)
	//			{
	//				matrixResult[i][j] = 0;

	//				for (int d = 0; d < SIZE; d++)	
	//				{
	//					matrixResult[i][j] += matrixA[i][d] * matrixB[d][j];
	//				}
	//			}
	//	}
	//	cout << "Usual CPU: " <<  clock() - CPU << endl;
	//}

	// MMX Calculation
	clock_t MMX = clock();

	_asm
	{
		push ebx
			mov edx, resMatrix			// edx - �������� ����� ��������������� ������
			xor eax, eax				// ��������� ����������
			xor ebx, ebx
			xor ecx, ecx				// ����� ���������� �������� �����

			mloop :
		push eax						// eax - ����� ������� �������� ������ ���������� ��������, ������� ��� ���������
			mov esi, aMatrix			// esi - �������� ����� ������� �������
			mov edi, bMatrix			// edi - �������� ����� ������� �������

			add esi, ebx
			add edi, ecx
			xor eax, eax
			pxor xmm2, xmm2				// ��������� 64-��������� ������

			sloop :
		movaps xmm0, [esi]				// ������� �������� ������ FP �������� �� 1 �������
			movaps xmm1, [edi]			// ������� �������� ������ FP �������� �� 2 �������
			mulps xmm0, xmm1			// ������� ������� ����������� ����������� ��������; xmm0 - ��������� ���������
			addps xmm2, xmm0			// ������� ������� ���������� ����������� �������� ������; xmm2 - �������� ��������� 
			add[esi], 16				// �������� � 1 �������, 16 = ������ 4-�� �������� ��������
			add[edi], 16				// �������� �� 2 �������
			add eax, 4					// 
			cmp eax, SIZE				// eax - SIZE = ?; �������� � ����� ����������� �������
			jne sloop					// ������� ���� eax != SIZE
			haddps xmm2, xmm2			// �������������� ��������  a (A3, A2, A1, A0) and operand b (B3, B2, B1, B0) is (B3 + B2, B1 + B0, A3 + A2, A1 + A0).
			movss[edx], xmm2			// ���������� �������� FP � ��������� ��������� � �������������� ������
			add edx, 4					// �������� � �������������� �������
			add ecx, SIZE * 4			// ecx - �������� �� 1 ������
			cmp ecx, SIZE*SIZE * 4		// ���� �� ����� �� ����� �������, �� �� ����� OK
			jne ok
			; row++						//
			xor ecx, ecx				// �������� ����������
			add ebx, SIZE * 4			// ������� �������
			cmp ebx, SIZE*SIZE * 4		// �������� �� �������� �� ���������� �������
			jne ok
			xor ebx, ebx
			ok :
		pop eax							
			inc eax					
			cmp eax, SIZE*SIZE			// ���� ����� �� ���������� ��������
			jne mloop					// ���� �� �����, �� �������� ������
			pop ebx
	}

	cout << "WITH MMX: " << clock() - MMX << endl;
	
	clock_t NoMMX = clock();
	_asm
	{
		push ebx
			mov edx, resMatrix
			xor eax, eax
			xor ebx, ebx
			xor ecx, ecx
			mloop2 :
		mov esi, aMatrix
			mov edi, bMatrix

			add esi, ebx
			add edi, ecx
			push eax
			push ebx
			push ecx
			push edx
			xor eax, eax
			xor edx, edx
			sloop2 :
		mov edx, [esi]
			mov ecx, [edi]
			imul edx, ecx
			add ebx, edx
			add[esi], 4
			add[edi], 4
			inc eax
			cmp eax, SIZE
			jne sloop2
			pop edx
			pop ecx
			mov[edx], ebx
			pop ebx
			pop eax

			add ecx, SIZE * 4
			cmp ecx, (SIZE - 1) * SIZE * 4
			jne ok2

			xor ecx, ecx
			add ebx, SIZE * 4
			cmp ebx, (SIZE - 1) * SIZE * 4
			jne ok2
			xor ebx, ebx
			ok2 :
		inc eax
			cmp eax, SIZE*SIZE
			jne mloop2
			pop ebx
	}

	cout << "Without MMX: " << clock() - NoMMX << endl;

	char x;
	cin >> x;

	return 0;
}
