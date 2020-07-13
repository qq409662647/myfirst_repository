/*
2020421对  score：1.682
path变一维数组
result变数组，结果不排序（提前排序）
__优化IO/建图
dfs中 减少变量赋值，减少
*/
#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <mutex>
#include<algorithm>
#include <chrono>
#include <stdio.h>
#include <string.h>

using namespace std;
typedef unsigned int ui;
#define CacheLineSize 128
int item = CacheLineSize / sizeof(int);
//#define TEST

#define THREADS_NUM   4
#define MAX_INSIZE     280000 //数据量级(28W)
#define MAX_MAPNODE    220000 //地图节点量级(28W)
#define MAX_OUTSIZE   3000000 //数据量级(300W)
#define MAX_SIZE  50//出入度55
#define MAX_pathnum  20
#define THREADBASELINE  8 * MAX_OUTSIZE * threadid
#define RESOFFSET 8 * threadNUM[threadid]
#define MERGEOFFSET 8 * it[threadid]
#define SIZE THREADBASELINE + MERGEOFFSET + 7

//路径
#ifdef TEST
string testFile = "./data/1004812/test_data.txt";//_10000_40000
string answerFile = "./data/1004812/result.txt";
string MyanswerFile = "./projects/student/result.txt";//_10000_40000
#else
string testFile = "/data/test_data.txt";//_10000_40000
string answerFile = "/data/result.txt";
string MyanswerFile = "/projects/student/result.txt";//_10000_40000
#endif // TEST


int readline = 0;
int in_data[2 * MAX_INSIZE];
#ifdef TEST
vector<vector <int>> Circle;
#endif // TEST

int Map[MAX_SIZE*MAX_MAPNODE];
int Maplen[MAX_MAPNODE] = { 0 };
int NMap[MAX_SIZE*MAX_MAPNODE];
int NMaplen[MAX_MAPNODE] = { 0 };
int NodeNum = 0;
int inside[MAX_MAPNODE];
int outside[MAX_MAPNODE];
int Maplist[MAX_MAPNODE];
int path[THREADS_NUM][MAX_MAPNODE*MAX_pathnum];//20  
int pathnum[THREADS_NUM][MAX_MAPNODE] = { 0 };
int pathsgn[THREADS_NUM][MAX_MAPNODE];
char *outbuff[THREADS_NUM];
int length[THREADS_NUM] = { 0 };


int threadres[8 * MAX_OUTSIZE * THREADS_NUM ];
int threadNUM[THREADS_NUM] = { 0 };
int* res3 = new int[3 * 500000];
int* res4 = new int[4 * 500000];
int* res5 = new int[5 * 1000000];
int* res6 = new int[6 * 2000000];
int* res7 = new int[7 * 3000000];
int *ans[5] = { res3,res4,res5,res6,res7 };//输出环
int anslen[8] = { 0 };

char  convert[220000][6];//注意数据大小

void transform_num(char* buffer, const int lenght)
{
	int nums = 0;
	int k = 0;
	int countnum = 0;
	int p = 0;
	for (int i = 0; i < lenght; i++) {
		if ((countnum + 1) % 3 == 0) {
			if (buffer[i] == '\n') {
				countnum++;//一行数据
				k++;
				readline = k;
				nums = 0;
				continue;
			}
			continue;
		}
		if (buffer[i] == ',') {
			countnum++;//一个数据
			in_data[p++] = nums;
			nums = 0;
			continue;
		}
		else {
			nums *= 10;
			nums += (int)(buffer[i] - '0');
		}
	}
}
bool fread_read(const string file)
{
	FILE *fp;
	if ((fp = fopen(file.c_str(), "rb")) == NULL) {
		cout << "File open error!" << endl;
		return false;
	}
	fseek(fp, 0L, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	cout << "Loading" << endl;
	char *buffer = new char[len];    //把所有字符一次全部读取到buf里面，包括‘\n‘
	if (fread(buffer, 1, len, fp) != len) {
		cout << "read text_data failed" << fread(buffer, 1, len, fp) << endl;
		return false;
	}
	thread threads[THREADS_NUM];
	transform_num(buffer, len);
	fclose(fp);
	return true;
}
void buildGraph() {			//														//loading
	if (bool status = fread_read(testFile)) {
		cout << "load text_data succeed" << endl;
	}
	else {
		cout << "load text_data failed" << endl;
	}
	int i;
	for (i = 0; i < readline; i++) {
		if (Maplen[in_data[2 * i]] == 0) {
			Maplist[NodeNum++] = in_data[2 * i];
		}
		Map[MAX_SIZE * in_data[2 * i] + Maplen[in_data[2 * i]]++] = in_data[2 * i + 1];
		NMap[MAX_SIZE * in_data[2 * i + 1] + NMaplen[in_data[2 * i + 1]]++] = in_data[2 * i];
	}
}
void cut() {//出入度剪枝
	memcpy(outside, Maplen, MAX_MAPNODE * sizeof(int));
	memcpy(inside, NMaplen, MAX_MAPNODE * sizeof(int));
	int top = 0;
	int x;
	int* stack = new int[NodeNum];
	for (int i = 0; i < NodeNum; i++) {
		if (inside[Maplist[i]] == 0)stack[top++] = Maplist[i];
	}
	while (top > 0) {
		x = stack[--top];
		for (int i = 0; i < Maplen[x]; i++) {
			if (--inside[Map[x * MAX_SIZE + i]] == 0)stack[++top] = Map[x * MAX_SIZE + i];
		}
		Maplen[x] = 0;
		NMaplen[x] = 0;
	}
	top = 0;
	for (int i = 0; i < NodeNum; i++) {
		if (outside[Maplist[i]] == 0)stack[top++] = Maplist[i];
	}
	while (top > 0)
	{
		int x = stack[--top];
		for (int i = 0; i < NMaplen[x]; i++)
		{
			if (--outside[NMap[x * MAX_SIZE + i]] == 0) stack[++top] = NMap[x * MAX_SIZE + i];
		}
		Maplen[x] = 0;
		NMaplen[x] = 0;
	}
	delete stack;

}
void sort() {
	sort(Maplist, Maplist + NodeNum);
	for (int i = 0; i < NodeNum; ++i) {//出入度剪枝
		if (Maplen[Maplist[i]] == 0) continue;
		sort(Map + MAX_SIZE * Maplist[i], Map + MAX_SIZE * Maplist[i] + Maplen[Maplist[i]]);
		sort(NMap + MAX_SIZE * Maplist[i], NMap + MAX_SIZE * Maplist[i] + NMaplen[Maplist[i]]);
	}
}
#ifdef TEST
void loadresult()
{
	FILE *fp;
	if ((fp = fopen(answerFile.c_str(), "rb")) == NULL) {
		cout << "File open error!" << endl;
	}
	fseek(fp, 0L, SEEK_END);
	ui len = ftell(fp);
	rewind(fp);
	cout << "Loading" << endl;
	char *buffer = new char[len];    //把所有字符一次全部读取到buf里面，包括‘\n‘
	if (fread(buffer, 1, len, fp) != len) {
		cout << "read text_data failed" << endl;
	}
	int nums = 0;
	vector<vector<int>> result;
	vector<int> p;
	int countnum = 0;
	for (int i = 0; i < len; i++) {
		if (buffer[i] == '\n') {
			countnum++;//一行数据
			p.push_back(nums);
			result.push_back(p);
			p.clear();
			nums = 0;
			continue;
		}
		if (buffer[i] == ',') {
			p.push_back(nums);
			countnum++;//一个数据
			nums = 0;
			continue;
		}
		else {
			nums *= 10;
			nums += (int)(buffer[i] - '0');
		}
	}
	fclose(fp);
	FILE *fp1;
	if ((fp1 = fopen(MyanswerFile.c_str(), "rb")) == NULL) {
		cout << "File open error!" << endl;
	}
	fseek(fp1, 0L, SEEK_END);
	len = ftell(fp1);
	rewind(fp1);
	cout << "Loading" << endl;
	char *buffer1 = new char[len];    //把所有字符一次全部读取到buf里面，包括‘\n‘
	if (fread(buffer1, 1, len, fp1) != len) {
		cout << "read text_data failed" << endl;
	}
	nums = 0;
	vector<vector<int>> Myresult;
	p.clear();
	countnum = 0;
	for (int i = 0; i < len; i++) {
		if (buffer1[i] == '\n') {
			countnum++;//一行数据
			p.push_back(nums);
			Myresult.push_back(p);
			p.clear();
			nums = 0;
			continue;
		}
		if (buffer1[i] == ',') {
			p.push_back(nums);
			countnum++;//一个数据
			nums = 0;
			continue;
		}
		else {
			nums *= 10;
			nums += (int)(buffer1[i] - '0');
		}
	}
	fclose(fp1);
	int line = fmin(Myresult.size(), result.size());
	cout << Myresult.size() << "/" << result.size() << endl;
	int flag = 1;
	int ernum = 0;
	for (int i = 0; i < line; i++) {
		if (Myresult[i].size() != result[i].size()) {
			flag = 0;
			ernum++;
			cout << i << endl;
			break;
		}
		else {
			for (int j = 0; j < Myresult[i].size(); j++) {
				if (Myresult[i][j] == result[i][j]) continue;
				else {
					flag = 0;
					ernum++;
					cout << i << endl;
					break;
				}
			}
		}
		if (flag == 0)break;
	}

	if (flag) cout << "congratulation!!!" << endl;
	else cout << "er!!!___" << ernum << endl;

}
#endif
void buildconvert(int threadid) {
	for (int i = threadid; i < 220000; i += THREADS_NUM) {
		if (i >= 100000) {
			convert[i][5] = '0' + i % 10;
			convert[i][4] = '0' + i / 10 % 10;
			convert[i][3] = '0' + i / 100 % 10;
			convert[i][2] = '0' + i / 1000 % 10;
			convert[i][1] = '0' + i / 10000 % 10;
			convert[i][0] = '0' + i / 100000 % 10;
		}
		else if (i >= 10000) {
			convert[i][4] = '0' + i % 10;
			convert[i][3] = '0' + i / 10 % 10;
			convert[i][2] = '0' + i / 100 % 10;
			convert[i][1] = '0' + i / 1000 % 10;
			convert[i][0] = '0' + i / 10000 % 10;
		}
		else if (i >= 1000) {
			convert[i][3] = '0' + i % 10;
			convert[i][2] = '0' + i / 10 % 10;
			convert[i][1] = '0' + i / 100 % 10;
			convert[i][0] = '0' + i / 1000 % 10;
		}
		else if (i >= 100) {
			convert[i][2] = '0' + i % 10;
			convert[i][1] = '0' + i / 10 % 10;
			convert[i][0] = '0' + i / 100 % 10;
		}
		else if (i >= 10) {
			convert[i][1] = '0' + i % 10;
			convert[i][0] = '0' + i / 10 % 10;
		}
		else convert[i][0] = '0' + i % 10;
	}
}
void int2str(const int &value, char* &buff, int &length, const char &i)//字符串转数字存入buff
{
	int tvalue = value;
	while (tvalue >= 10000)
	{
		tvalue /= 10000;
		length += 4;
	}
	if (tvalue >= 1000)length += 4;
	else if (tvalue >= 100)length += 3;
	else if (tvalue >= 10)length += 2;
	else length++;
	tvalue = value;
	int len = length - 1;
	while (tvalue) {
		buff[len--] = '0' + tvalue % 10;
		tvalue /= 10;
	}
	buff[length++] = i;
}

void result_out(const int size, int* res, int batch, int &length, char* buff) {//2020422循环展开
	for (int j = 0; j < size; ++j) {
		for (int k = 0; k < batch; ++k) {
			int &temp = res[j*batch + k];
			if (temp >= 100000) {
				//buff[length] = convert[temp][5];
				//buff[length + 1] = convert[temp][4];
				//buff[length + 2] = convert[temp][3];
				//buff[length + 3] = convert[temp][2];
				//buff[length + 4] = convert[temp][1];
				//buff[length + 5] = convert[temp][0];
				memcpy(buff+ length, convert[temp],sizeof(char)*6);
				length += 6;
			}
			else if (temp >= 10000) {
				//buff[length] = convert[temp][0];
				//buff[length + 1] = convert[temp][1];
				//buff[length + 2] = convert[temp][2];
				//buff[length + 3] = convert[temp][3];
				//buff[length + 4] = convert[temp][4];
				memcpy(buff + length, convert[temp], sizeof(char) * 5);
				length += 5;
			}
			else if (temp >= 1000) {
				//buff[length] = convert[temp][0];
				//buff[length + 1] = convert[temp][1];
				//buff[length + 2] = convert[temp][2];
				//buff[length + 3] = convert[temp][3];
				memcpy(buff + length, convert[temp], sizeof(char) * 4);
				length += 4;
			}
			else if (temp >= 100) {
				buff[length] = convert[temp][0];
				buff[length + 1] = convert[temp][1];
				buff[length + 2] = convert[temp][2];
				//memcpy(buff + length, convert[temp], sizeof(char) * 3);
				length += 3;
			}
			else if (temp >= 10) {
				buff[length] = convert[temp][0];
				buff[length + 1] = convert[temp][1];
				//memcpy(buff + length, convert[temp], sizeof(char) * 2);
				length += 2;
			}
			else buff[length++] = convert[temp][0];
			buff[length++] = ',';
		}
		buff[length - 1] = '\n';
	}
}

void result_outt1() {
	int outline = 0;
	outbuff[0] = new char[30 * anslen[3] + 40 * anslen[4] + 50 * anslen[5] + 60 * anslen[6]];//+ 70 * anslen[7]];
	outline = anslen[3] + anslen[4] + anslen[5] + anslen[6] + anslen[7];
	int2str(outline, outbuff[0], length[0], '\n');
	for (int i = 3; i < 7; ++i) {
		result_out(anslen[i], ans[i - 3], i, length[0], outbuff[0]);//ans为环的数据首地址
	}
}

void result_outt2(int threadid) {
	outbuff[threadid] = new char[anslen[7] / (THREADS_NUM - 1) * 60];
	if (threadid<THREADS_NUM - 1)result_out(anslen[7] / (THREADS_NUM - 1)*(threadid)-anslen[7] / (THREADS_NUM - 1)*(threadid - 1), res7 + anslen[7] / (THREADS_NUM - 1) * 7 * (threadid - 1), 7, length[threadid], outbuff[threadid]);
	else result_out(anslen[7] - anslen[7] / (THREADS_NUM - 1)*(threadid - 1), res7 + anslen[7] / (THREADS_NUM - 1) * 7 * (threadid - 1), 7, length[threadid], outbuff[threadid]);
}


void save(const string &file) {
	thread threads_tran[THREADS_NUM];
	threads_tran[0] = thread(buildconvert, 0);
	threads_tran[1] = thread(buildconvert, 1);
	threads_tran[2] = thread(buildconvert, 2);
	threads_tran[3] = thread(buildconvert, 3);
	threads_tran[0].join();
	threads_tran[1].join();
	threads_tran[2].join();
	threads_tran[3].join();
	thread threads[THREADS_NUM];
	threads[1] = thread(result_outt2, 1);
	threads[2] = thread(result_outt2, 2);
	threads[3] = thread(result_outt2, 3);
	threads[0] = thread(result_outt1);
	//result_outt1();
	FILE *fp;
	if ((fp = fopen(file.c_str(), "wb")) == NULL) {
		cout << "File open error!" << endl;
		exit(0);
	}
	threads[0].join();
	fwrite(outbuff[0], sizeof(char), length[0], fp);
	threads[1].join();
	fwrite(outbuff[1], sizeof(char), length[1], fp);
	threads[2].join();
	fwrite(outbuff[2], sizeof(char), length[2], fp);
	threads[3].join();
	fwrite(outbuff[3], sizeof(char), length[3], fp);
	fclose(fp);

}

void taowa(int NodeNum, int threadid) {
	int num = 0;
	for (int it = threadid; it < NodeNum; it += THREADS_NUM) {//首节点（0层循环）
		int head = Maplist[it];
		if (Maplen[head] == 0) continue;

		for (int itn1 = MAX_SIZE * head; itn1 < MAX_SIZE * head + NMaplen[head]; ++itn1) {//一层循环
			if (NMap[itn1] <= head) continue;//该节点符合拓扑排序
			for (int itn2 = MAX_SIZE * NMap[itn1]; itn2 < MAX_SIZE * NMap[itn1] + NMaplen[NMap[itn1]]; ++itn2) {//二层循环
				if (NMap[itn2] <= head) continue;
				path[threadid][NMap[itn2] * MAX_pathnum + pathnum[threadid][NMap[itn2]]] = NMap[itn1];
				pathsgn[threadid][num] = NMap[itn2];//环数
				++pathnum[threadid][NMap[itn2]];
				++num;
			}
		}//记忆化5+2
		for (int it1 = MAX_SIZE * head; it1 < MAX_SIZE * head + Maplen[head]; ++it1) {//1
			if (Map[it1] <= head) continue;

			for (int it2 = MAX_SIZE * Map[it1]; it2 < MAX_SIZE * Map[it1] + Maplen[Map[it1]]; ++it2) {//2
				if (Map[it2] <= head) continue;

				for (int it3 = MAX_SIZE * Map[it2]; it3 < MAX_SIZE * Map[it2] + Maplen[Map[it2]]; ++it3) {//5
					if (Map[it3] < head || Map[it3] == Map[it1]) continue;
					if (Map[it3] == head) {//3+0
						threadres[THREADBASELINE + RESOFFSET] = head;
						threadres[THREADBASELINE + RESOFFSET + 1] = Map[it1];
						threadres[THREADBASELINE + RESOFFSET + 2] = Map[it2];
						threadres[THREADBASELINE + RESOFFSET + 7] = 3;
						++threadNUM[threadid];
						continue;
					}
					for (int it4 = MAX_SIZE * Map[it3]; it4 < MAX_SIZE * Map[it3] + Maplen[Map[it3]]; ++it4) {
						if (Map[it4] < head || Map[it4] == Map[it1] || Map[it4] == Map[it2]) continue;
						if (Map[it4] == head) {//3+1
							threadres[THREADBASELINE + RESOFFSET] = head;
							threadres[THREADBASELINE + RESOFFSET + 1] = Map[it1];
							threadres[THREADBASELINE + RESOFFSET + 2] = Map[it2];
							threadres[THREADBASELINE + RESOFFSET + 3] = Map[it3];
							threadres[THREADBASELINE + RESOFFSET + 7] = 4;
							++threadNUM[threadid];
							continue;
						}
						for (int j = 0; j < pathnum[threadid][Map[it4]]; ++j) {//6
							if (Map[it1] == path[threadid][Map[it4] * MAX_pathnum + j] || \
								Map[it2] == path[threadid][Map[it4] * MAX_pathnum + j] || \
								Map[it3] == path[threadid][Map[it4] * MAX_pathnum + j])continue;
							threadres[THREADBASELINE + RESOFFSET] = head;
							threadres[THREADBASELINE + RESOFFSET + 1] = Map[it1];
							threadres[THREADBASELINE + RESOFFSET + 2] = Map[it2];
							threadres[THREADBASELINE + RESOFFSET + 3] = Map[it3];
							threadres[THREADBASELINE + RESOFFSET + 4] = Map[it4];
							threadres[THREADBASELINE + RESOFFSET + 5] = path[threadid][Map[it4] * MAX_pathnum + j];
							threadres[THREADBASELINE + RESOFFSET + 7] = 6;
							++threadNUM[threadid];
						}
						for (int it5 = MAX_SIZE * Map[it4]; it5 < MAX_SIZE * Map[it4] + Maplen[Map[it4]]; ++it5) {
							if (Map[it5] < head || Map[it5] == Map[it3] || Map[it5] == Map[it2] || Map[it5] == Map[it1]) continue;
							if (Map[it5] == head) {//5
								threadres[THREADBASELINE + RESOFFSET] = head;
								threadres[THREADBASELINE + RESOFFSET + 1] = Map[it1];
								threadres[THREADBASELINE + RESOFFSET + 2] = Map[it2];
								threadres[THREADBASELINE + RESOFFSET + 3] = Map[it3];
								threadres[THREADBASELINE + RESOFFSET + 4] = Map[it4];
								threadres[THREADBASELINE + RESOFFSET + 7] = 5;
								++threadNUM[threadid];
								continue;
							}
							for (int j = 0; j < pathnum[threadid][Map[it5]]; ++j) {//3+4
								if (Map[it1] == path[threadid][Map[it5] * MAX_pathnum + j] || \
									Map[it2] == path[threadid][Map[it5] * MAX_pathnum + j] || \
									Map[it3] == path[threadid][Map[it5] * MAX_pathnum + j] || \
									Map[it4] == path[threadid][Map[it5] * MAX_pathnum + j])continue;
								threadres[THREADBASELINE + RESOFFSET] = head;
								threadres[THREADBASELINE + RESOFFSET + 1] = Map[it1];
								threadres[THREADBASELINE + RESOFFSET + 2] = Map[it2];
								threadres[THREADBASELINE + RESOFFSET + 3] = Map[it3];
								threadres[THREADBASELINE + RESOFFSET + 4] = Map[it4];
								threadres[THREADBASELINE + RESOFFSET + 5] = Map[it5];
								threadres[THREADBASELINE + RESOFFSET + 6] = path[threadid][Map[it5] * MAX_pathnum + j];
								threadres[THREADBASELINE + RESOFFSET + 7] = 7;
								++threadNUM[threadid];
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < num; i++)
		{
			pathnum[threadid][pathsgn[threadid][i]] = 0;
		}
		num = 0;
	}
}

void merge_res() {//默认4线程编写(3)
	int it[THREADS_NUM] = { 0 };
	int threadid = 0;
	for (int i = 0; i < NodeNum; i += THREADS_NUM) {
		for (int threadid = 0; threadid < THREADS_NUM; threadid++) {
			while (Maplist[i + threadid] == threadres[THREADBASELINE + MERGEOFFSET]) {
				if (threadres[SIZE] == 3) {
					//res3[3 * anslen[3]] = threadres[THREADBASELINE + MERGEOFFSET];
					//res3[3 * anslen[3] + 1] = threadres[THREADBASELINE + MERGEOFFSET + 1];
					//res3[3 * anslen[3] + 2] = threadres[THREADBASELINE + MERGEOFFSET + 2];
					memcpy(res3 + 3 * anslen[3], threadres + THREADBASELINE + MERGEOFFSET,sizeof(int)*3);
					++anslen[3];
					++it[threadid];
				}
				else if (threadres[SIZE] == 4) {
					//res4[4 * anslen[4]] = threadres[THREADBASELINE + MERGEOFFSET];
					//res4[4 * anslen[4] + 1] = threadres[THREADBASELINE + MERGEOFFSET + 1];
					//res4[4 * anslen[4] + 2] = threadres[THREADBASELINE + MERGEOFFSET + 2];
					//res4[4 * anslen[4] + 3] = threadres[THREADBASELINE + MERGEOFFSET + 3];
					memcpy(res4 + 4 * anslen[4], threadres + THREADBASELINE + MERGEOFFSET, sizeof(int) * 4);
					++anslen[4];
					++it[threadid];
				}
				else if (threadres[SIZE] == 5) {
					//res5[5 * anslen[5]] = threadres[THREADBASELINE + MERGEOFFSET];
					//res5[5 * anslen[5] + 1] = threadres[THREADBASELINE + MERGEOFFSET + 1];
					//res5[5 * anslen[5] + 2] = threadres[THREADBASELINE + MERGEOFFSET + 2];
					//res5[5 * anslen[5] + 3] = threadres[THREADBASELINE + MERGEOFFSET + 3];
					//res5[5 * anslen[5] + 4] = threadres[THREADBASELINE + MERGEOFFSET + 4];
					memcpy(res5 + 5 * anslen[5], threadres + THREADBASELINE + MERGEOFFSET, sizeof(int) * 5);
					++anslen[5];
					++it[threadid];
				}
				else if (threadres[SIZE] == 6) {
					//res6[6 * anslen[6]] = threadres[THREADBASELINE + MERGEOFFSET];
					//res6[6 * anslen[6] + 1] = threadres[THREADBASELINE + MERGEOFFSET + 1];
					//res6[6 * anslen[6] + 2] = threadres[THREADBASELINE + MERGEOFFSET + 2];
					//res6[6 * anslen[6] + 3] = threadres[THREADBASELINE + MERGEOFFSET + 3];
					//res6[6 * anslen[6] + 4] = threadres[THREADBASELINE + MERGEOFFSET + 4];
					//res6[6 * anslen[6] + 5] = threadres[THREADBASELINE + MERGEOFFSET + 5];
					memcpy(res6 + 6 * anslen[6], threadres + THREADBASELINE + MERGEOFFSET, sizeof(int) * 6);
					++anslen[6];
					++it[threadid];
				}
				else if (threadres[SIZE] == 7) {
					//res7[7 * anslen[7]] = threadres[THREADBASELINE + MERGEOFFSET];
					//res7[7 * anslen[7] + 1] = threadres[THREADBASELINE + MERGEOFFSET + 1];
					//res7[7 * anslen[7] + 2] = threadres[THREADBASELINE + MERGEOFFSET + 2];
					//res7[7 * anslen[7] + 3] = threadres[THREADBASELINE + MERGEOFFSET + 3];
					//res7[7 * anslen[7] + 4] = threadres[THREADBASELINE + MERGEOFFSET + 4];
					//res7[7 * anslen[7] + 5] = threadres[THREADBASELINE + MERGEOFFSET + 5];
					//res7[7 * anslen[7] + 6] = threadres[THREADBASELINE + MERGEOFFSET + 6];
					memcpy(res7 + 7 * anslen[7], threadres + THREADBASELINE + MERGEOFFSET, sizeof(int) * 7);
					++anslen[7];
					++it[threadid];
				}
				else if (threadres[SIZE] == 0) break;

			}
		}
	}
}
int main()
{
	buildGraph();//建图

	cut();//剪枝
	sort();

	volatile int NUM = NodeNum;
	thread threadsdfs[THREADS_NUM];
	threadsdfs[0] = thread(taowa, NUM, 0);
	threadsdfs[1] = thread(taowa, NUM, 1);
	threadsdfs[2] = thread(taowa, NUM, 2);
	threadsdfs[3] = thread(taowa, NUM, 3);
	threadsdfs[0].join();
	threadsdfs[1].join();
	threadsdfs[2].join();
	threadsdfs[3].join();
	//长度排序

	merge_res();

	save(MyanswerFile);


#ifdef TEST
	loadresult();//计时
	while (1);
#endif

	exit(0);
	return 0;
}
