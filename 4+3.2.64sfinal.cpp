/*
--------------
å¤èµ›demoï¼ˆç¬¬Nç‰ˆï¼‰å‰å‘æ˜?VIS6+3+outå‡è¡¡è´Ÿè½½
æ¬ç –å·¥ï¼šè”¡å­Ÿå‡¯é»„å­æ˜
é²²é¹æµ‹è¯•2000Wæ•°æ®é›†ï¼š23s / è¯»å…¥å»ºå›¾3s / è¾“å‡º1.8s / å…¶ä½™dfsã€‚ä¿®æ”?/ ré‡‘é¢å‡ºé”™çš„é—®é¢?
æ—¶é—´ï¼?020 / 04 / 30
--------------
*/
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <thread>
#include<algorithm>
#include <chrono>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <atomic>
//#include <unistd.h>

//#define TEST

using namespace std;

#define CacheLineSize 128
int item = CacheLineSize / sizeof(int);
#define THREADS_NUM   4
#define DFSTHREADS_NUM   4
#define MAX_INSIZE     2000000 //æ•°æ®é‡çº§(200W)
#define MAX_OUTSIZE    20000000//æ•°æ®é‡çº§(2000W)

#ifdef TEST
#include "timer.h"
string testFile = "/data/test_data.txt";//_10000_40000
string answerFile = "/data/result.txt";
string MyanswerFile = "/projects/student/result.txt";//_10000_40000
#else
string testFile = "/data/test_data.txt";//_10000_40000
string answerFile = "/data/result.txt";
string MyanswerFile = "/projects/student/result.txt";//_10000_40000
#endif // TEST

int threadreadline[4] = { 0 };//æ•°æ®è¡Œæ•°
int readline;
int pos[5];
int *threadin_data[4];//4Ïß³Ì¶ÁÊı¾İ
int in_data[MAX_INSIZE * 3];
int NodeNum = 1;
typedef unsigned int ui;
typedef struct Mapinfo {
	int id;
	int money;
}Mfo;
int Maplist[MAX_INSIZE + 1];
int Maphead[MAX_INSIZE + 1] = { 0 };
Mfo Map[MAX_INSIZE + 1];
int NMaphead[MAX_INSIZE + 1] = { 0 };
Mfo NMap[MAX_INSIZE + 1];


int threadres[8 * MAX_OUTSIZE * DFSTHREADS_NUM];
int threadNUM[DFSTHREADS_NUM] = { 0 };
typedef struct Pathid {
	int id1;
	int id2;
	int money1;
	int money3;
}path;
path *mempath[DFSTHREADS_NUM];//20  
uint8_t pathnum[DFSTHREADS_NUM][MAX_INSIZE] = { 0 };//-0.8s
int *res3 = new int[3 * MAX_OUTSIZE];
int *res4 = new int[4 * MAX_OUTSIZE];
int *res5 = new int[5 * MAX_OUTSIZE];
int *res6 = new int[6 * MAX_OUTSIZE];
int *res7 = new int[7 * MAX_OUTSIZE];
int *ans[5] = { res3,res4,res5,res6,res7 };//è¾“å‡ºç?
int anslen[8] = { 0 };
char resultMap[11 * MAX_INSIZE];
int length[THREADS_NUM] = { 0 };
char *outbuff[THREADS_NUM];

struct divideMsg
{
	int start;
	int last;
	int length;
	divideMsg() :last(0), length(0), start(0) {}
};
divideMsg msg[THREADS_NUM];
int totalDivide = 0;
int outNum[THREADS_NUM] = { 0 };
atomic<int>threadCount(0);
uint8_t threadsPoint[MAX_INSIZE] = { 0 };
void transform_num(char* buffer, int threadid)
{
	threadin_data[threadid] = new int[MAX_INSIZE];
	int readlinet = 0;
	int nums = 0;
	int p = 0;
	for (int i = pos[threadid]; i < pos[threadid + 1]; i++) {
		if (buffer[i] == '\n') {
			threadin_data[threadid][p++] = nums;
			readlinet++;
			nums = 0;
			continue;
		}
		else if (buffer[i] == ',') {
			threadin_data[threadid][p++] = nums;
			nums = 0;
			continue;
		}
		else if (buffer[i] == '\r') continue;
		else {
			nums *= 10;
			nums += (int)(buffer[i] - '0');
		}
	}
	threadreadline[threadid] = readlinet;
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
	char *buffer = new char[len];    //æŠŠæ‰€æœ‰å­—ç¬¦ä¸€æ¬¡å…¨éƒ¨è¯»å–åˆ°bufé‡Œé¢ï¼ŒåŒ…æ‹¬â€˜\nâ€?
	if (fread(buffer, 1, len, fp) != len) {
		cout << "read text_data failed" << endl;
		return false;
	}
	fclose(fp);
	pos[1] = len / 4;
	pos[2] = pos[1] * 2;
	pos[3] = pos[1] * 3;
	pos[4] = len;
	while (buffer[pos[1]] != '\n') pos[1]++;
	pos[1]++;
	while (buffer[pos[2]] != '\n') pos[2]++;
	pos[2]++;
	while (buffer[pos[3]] != '\n') pos[3]++;
	pos[3]++;
	thread readthread[4];
	readthread[0] = thread(transform_num, buffer, 0);
	readthread[1] = thread(transform_num, buffer, 1);
	readthread[2] = thread(transform_num, buffer, 2);
	readthread[3] = thread(transform_num, buffer, 3);
	readthread[0].join();
	readthread[1].join();
	readthread[2].join();
	readthread[3].join();
	delete[] buffer;
	buffer = NULL;
	return true;
}

bool cmp(Mfo id1, Mfo id2) {
	return id1.id < id2.id;
}
void maphash(int threadid, unordered_map<int, int> &Mapnode) {
	auto tempy = Mapnode.begin(), tempx = Mapnode.begin();
	int readmax = readline;
	for (int i = threadid; i < readmax; i += 4) {
		tempy = Mapnode.find(in_data[3 * i + 1]);
		if (tempy != Mapnode.end()) {
			tempx = Mapnode.find(in_data[3 * i]);
			in_data[3 * i] = tempx->second;
			in_data[3 * i + 1] = tempy->second;
		}
		else {
			in_data[3 * i + 1] = 0;
		}
	}
}
void buildMap() {
	Mfo temp;
	int *curlen = new int[NodeNum]();
	int *maplen = new int[NodeNum]();
	int i;
	int readmax = readline;
	int max = NodeNum;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 1] != 0) {
			maplen[in_data[3 * i]]++;
		}
	}
	for (i = 1; i <= max; ++i) {
		Maphead[i] = Maphead[i - 1] + maplen[i - 1];
	}
	int tempx;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 1] != 0) {
			tempx = in_data[3 * i];//x
			temp.id = in_data[3 * i + 1];
			temp.money = in_data[3 * i + 2];
			Map[Maphead[tempx] + curlen[tempx]] = temp;
			curlen[tempx]++;
		}
	}
	delete[]curlen;
	delete[]maplen;
}
void buildNMap() {
	Mfo temp;
	int *curlen1 = new int[NodeNum]();
	int *nmaplen = new int[NodeNum]();
	int i;
	int readmax = readline;
	int max = NodeNum;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 1] != 0) {
			nmaplen[in_data[3 * i + 1]]++;
		}
	}
	for (i = 1; i <= max; ++i) NMaphead[i] = NMaphead[i - 1] + nmaplen[i - 1];
	int tempy;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 1] != 0) {
			tempy = in_data[3 * i + 1];//x
			temp.id = in_data[3 * i];
			temp.money = in_data[3 * i + 2];
			NMap[NMaphead[tempy] + curlen1[tempy]] = temp;
			curlen1[tempy]++;
		}
	}
	delete[]curlen1;
	delete[]nmaplen;
}
void mapsort(int threadid) {
	int i;
	int max = NodeNum;
	for (i = threadid; i < max; i += 4) {
		if (NMaphead[i] == NMaphead[i + 1]) continue;
		sort(NMap + NMaphead[i], NMap + NMaphead[i + 1], cmp);
	}
	for (i = threadid; i < max; i += 4) {
		sort(Map + Maphead[i], Map + Maphead[i + 1], cmp);
	}
}
void buildGraph() {
#ifdef TEST
	Timer t;
	t.start();
#endif
	if (bool status = fread_read(testFile)) {
		cout << "load text_data succeed" << endl;
	}
	else {
		cout << "load text_data failed" << endl;
	}
#ifdef TEST
	t.end();
	t.start();
#endif
	memcpy(in_data, threadin_data[0], sizeof(int) * 3 * threadreadline[0]);
	memcpy(in_data + threadreadline[0] * 3, threadin_data[1], sizeof(int) * 3 * threadreadline[1]);
	memcpy(in_data + threadreadline[0] * 3 + threadreadline[1] * 3, threadin_data[2], sizeof(int) * 3 * threadreadline[2]);
	memcpy(in_data + threadreadline[0] * 3 + threadreadline[1] * 3 + threadreadline[2] * 3, threadin_data[3], sizeof(int) * 3 * threadreadline[3]);
	readline = threadreadline[0] + threadreadline[1] + threadreadline[2] + threadreadline[3];
	delete[]threadin_data[0];
	delete[]threadin_data[1];
	delete[]threadin_data[2];
	delete[]threadin_data[3];
	int i;
	//bool* Vistemp = new bool[UINT32_MAX];
#ifdef TEST
	t.end();
	t.start();
#endif
	unordered_map<int, bool> Vistemp;
	Vistemp.reserve(2200000);//220W
	int readmax = readline;
	int max = NodeNum;
	for (i = 0; i < readmax; i++) {
		if (Vistemp[in_data[3 * i]] != 0) continue;
		Vistemp[in_data[3 * i]] = 1;
		Maplist[max++] = in_data[3 * i];
	}
	NodeNum = max;
	//delete[]Vistemp;
	sort(Maplist + 1, Maplist + max);
	unordered_map<int, int> Mapnode;
	Mapnode.reserve(2200000);
	for (i = 1; i < max; ++i) Mapnode[Maplist[i]] = i;
#ifdef TEST
	t.end();
	t.start();
#endif

	thread mapsh[4];
	mapsh[0] = thread(maphash, 0, ref(Mapnode));
	mapsh[1] = thread(maphash, 1, ref(Mapnode));
	mapsh[2] = thread(maphash, 2, ref(Mapnode));
	mapsh[3] = thread(maphash, 3, ref(Mapnode));
	mapsh[0].join();
	mapsh[1].join();
	mapsh[2].join();
	mapsh[3].join();
	thread BUILDMAP = thread(buildMap);
	thread BUILDNMAP = thread(buildNMap);
	BUILDMAP.join();
	BUILDNMAP.join();

	thread sortthread[4];
	sortthread[0] = thread(mapsort, 0);
	sortthread[1] = thread(mapsort, 1);
	sortthread[2] = thread(mapsort, 2);
	sortthread[3] = thread(mapsort, 3);
	sortthread[0].join();
	sortthread[1].join();
	sortthread[2].join();
	sortthread[3].join();
#ifdef TEST
	t.end();
	t.start();
#endif
}

uint64_t const a = 3;
uint64_t const b = 5;

inline bool check(int X, int Y) {//
	return (Y <= a * X) && (X <= b * Y);
}
//#define check(X,Y) (Y <= a * X) && (X <= b * Y)
inline bool pathcmp(path id1, path id2) {
	if (id1.id2 != id2.id2)return id1.id2 < id2.id2;
	return id1.id1 < id2.id1;
}
#define THREADBASELINE  8 * MAX_OUTSIZE * threadid
#define RESOFFSET 8 * resnum
#define MAX_pathnum 128
void taowa7(int threadid, int resnum) {
	int *pathsgn = new int[MAX_OUTSIZE];
	mempath[threadid] = new path[NodeNum*MAX_pathnum];
	int num = 0;
	int i, j;
	int head;
	int tmp;
	Mfo itn1, itn2, itn3, it1, it2, it3, it4, it5, it6;
	int itn11, itn22, itn33, it11, it22, it33, it44, it55, it66;
	int itn1size, itn2size, itn3size, it1size, it2size, it3size, it4size, it5size, it6size;
	int threadbaseline = THREADBASELINE;
	path temp;
	int max = NodeNum;
	while ((tmp = threadCount.load())<max - 1) {
		while (!threadCount.compare_exchange_strong(tmp, tmp + 1)) {};
		i = tmp + 1;
		if (NMaphead[i] == NMaphead[i + 1]) continue;
		threadsPoint[i] = threadid;
		head = i;//¸¸½Úµã
		itn1size = NMaphead[head + 1];
		for (itn11 = NMaphead[head]; itn11 < itn1size; ++itn11) {//Ò»²ãÑ­»·
			itn1 = NMap[itn11];
			if (itn1.id <= head) continue;//¸Ã½Úµã·ûºÏÍØÆËÅÅĞò

			itn2size = NMaphead[itn1.id + 1];
			for (itn22 = NMaphead[itn1.id]; itn22 <itn2size; ++itn22) {//¶ş²ãÑ­»·
				itn2 = NMap[itn22];
				if (!check(itn2.money, itn1.money) || itn2.id <= head) continue;//¸Ã½Úµã·ûºÏÍØÆËÅÅĞò

				itn3size = NMaphead[itn2.id + 1];
				for (itn33 = NMaphead[itn2.id]; itn33 < itn3size; ++itn33) {
					itn3 = NMap[itn33];
					if (!check(itn3.money, itn2.money) || itn3.id < head || itn3.id == itn1.id) continue;//¸Ã½Úµã·ûºÏÍØÆËÅÅĞò
					mempath[threadid][itn3.id*MAX_pathnum + pathnum[threadid][itn3.id]].id1 = itn1.id;
					mempath[threadid][itn3.id*MAX_pathnum + pathnum[threadid][itn3.id]].id2 = itn2.id;
					mempath[threadid][itn3.id*MAX_pathnum + pathnum[threadid][itn3.id]].money1 = itn1.money;
					mempath[threadid][itn3.id*MAX_pathnum + pathnum[threadid][itn3.id]].money3 = itn3.money;
					pathnum[threadid][itn3.id]++;//»·Êı
					pathsgn[num] = itn3.id;
					num++;
				}
			}
		}//¼ÇÒä»¯4+3
		if (num == 0) continue;
		sort(mempath[threadid] + head*MAX_pathnum, mempath[threadid] + head*MAX_pathnum + pathnum[threadid][head], pathcmp);
		for (int j = 0; j < pathnum[threadid][head]; j++) {//3+0
			temp = mempath[threadid][head*MAX_pathnum + j];
			if (!check(temp.money1, temp.money3)) continue;
			threadres[threadbaseline + RESOFFSET] = 3;
			threadres[threadbaseline + RESOFFSET + 1] = head;
			threadres[threadbaseline + RESOFFSET + 2] = temp.id2;
			threadres[threadbaseline + RESOFFSET + 3] = temp.id1;
			++resnum;
			//anslen[3]++;
		}
		it1size = Maphead[head + 1];
		for (it11 = Maphead[head]; it11 <it1size; ++it11) {//1.
			it1 = Map[it11];
			if (it1.id <= head) continue;
			sort(mempath[threadid] + it1.id*MAX_pathnum, mempath[threadid] + it1.id*MAX_pathnum + pathnum[threadid][it1.id], pathcmp);
			for (int j = 0; j < pathnum[threadid][it1.id]; j++) {//3+1
				temp = mempath[threadid][it1.id*MAX_pathnum + j];
				if (!check(it1.money, temp.money3) || \
					!check(temp.money1, it1.money)) continue;
				threadres[threadbaseline + RESOFFSET] = 4;
				threadres[threadbaseline + RESOFFSET + 1] = head;
				threadres[threadbaseline + RESOFFSET + 2] = it1.id;
				threadres[threadbaseline + RESOFFSET + 3] = temp.id2;
				threadres[threadbaseline + RESOFFSET + 4] = temp.id1;
				++resnum;
				//anslen[4]++;
			}
			it2size = Maphead[it1.id + 1];
			for (it22 = Maphead[it1.id]; it22 < it2size; ++it22) {//2
				it2 = Map[it22];
				if (it2.id <= head || !check(it1.money, it2.money)) continue;
				sort(mempath[threadid] + it2.id*MAX_pathnum, mempath[threadid] + it2.id*MAX_pathnum + pathnum[threadid][it2.id], pathcmp);
				for (int j = 0; j < pathnum[threadid][it2.id]; j++) {//3+2
					temp = mempath[threadid][it2.id*MAX_pathnum + j];
					if (it1.id == temp.id2 || \
						it1.id == temp.id1)continue;
					if (!check(it2.money, temp.money3) || \
						!check(temp.money1, it1.money)) continue;
					threadres[threadbaseline + RESOFFSET] = 5;
					threadres[threadbaseline + RESOFFSET + 1] = head;
					threadres[threadbaseline + RESOFFSET + 2] = it1.id;
					threadres[threadbaseline + RESOFFSET + 3] = it2.id;
					threadres[threadbaseline + RESOFFSET + 4] = temp.id2;
					threadres[threadbaseline + RESOFFSET + 5] = temp.id1;
					++resnum;
					//anslen[5]++;
				}
				it3size = Maphead[it2.id + 1];
				for (it33 = Maphead[it2.id]; it33 < it3size; ++it33) {//2
					it3 = Map[it33];
					if (it3.id <= head || it3.id == it1.id || !check(it2.money, it3.money)) continue;
					sort(mempath[threadid] + it3.id*MAX_pathnum, mempath[threadid] + it3.id*MAX_pathnum + pathnum[threadid][it3.id], pathcmp);
					for (int j = 0; j < pathnum[threadid][it3.id]; j++) {//3+3
						temp = mempath[threadid][it3.id*MAX_pathnum + j];
						if (it1.id == temp.id1 || \
							it2.id == temp.id1 || \
							it1.id == temp.id2 || \
							it2.id == temp.id2)continue;
						if (!check(it3.money, temp.money3) || \
							!check(temp.money1, it1.money)) continue;
						threadres[threadbaseline + RESOFFSET] = 6;
						threadres[threadbaseline + RESOFFSET + 1] = head;
						threadres[threadbaseline + RESOFFSET + 2] = it1.id;
						threadres[threadbaseline + RESOFFSET + 3] = it2.id;
						threadres[threadbaseline + RESOFFSET + 4] = it3.id;
						threadres[threadbaseline + RESOFFSET + 5] = temp.id2;
						threadres[threadbaseline + RESOFFSET + 6] = temp.id1;
						++resnum;
						//anslen[6]++;
					}
					it4size = Maphead[it3.id + 1];
					for (it44 = Maphead[it3.id]; it44 < it4size; ++it44) {
						it4 = Map[it44];
						if (it4.id <= head || it4.id == it1.id || it4.id == it2.id || !check(it3.money, it4.money)) continue;//²»Âú×ãÏÂËÑÌõ¼ş

						sort(mempath[threadid] + it4.id*MAX_pathnum, mempath[threadid] + it4.id*MAX_pathnum + pathnum[threadid][it4.id], pathcmp);
						for (int j = 0; j < pathnum[threadid][it4.id]; j++) {//3+4
							temp = mempath[threadid][it4.id*MAX_pathnum + j];
							if (it1.id == temp.id1 || \
								it1.id == temp.id2 || \
								it2.id == temp.id1 || \
								it2.id == temp.id2 || \
								it3.id == temp.id1 || \
								it3.id == temp.id2
								)continue;
							if (!check(it4.money, temp.money3) || \
								!check(temp.money1, it1.money)) continue;
							threadres[threadbaseline + RESOFFSET] = 7;
							threadres[threadbaseline + RESOFFSET + 1] = head;
							threadres[threadbaseline + RESOFFSET + 2] = it1.id;
							threadres[threadbaseline + RESOFFSET + 3] = it2.id;
							threadres[threadbaseline + RESOFFSET + 4] = it3.id;
							threadres[threadbaseline + RESOFFSET + 5] = it4.id;
							threadres[threadbaseline + RESOFFSET + 6] = temp.id2;
							threadres[threadbaseline + RESOFFSET + 7] = temp.id1;
							++resnum;
							//anslen[7]++;
						}
					}
				}
			}
		}
		for (j = 0; j < num; ++j) pathnum[threadid][pathsgn[j]] = 0;
		num = 0;
	}
	delete[]pathsgn;
	pathsgn = NULL;
}
#define MERGEOFFSET 8 * it[threadid] + 1
#define SIZE threadbaseline + MERGEOFFSET - 1
void merge_res() {
	int it[DFSTHREADS_NUM] = { 0 };
	int threadid = 0;
	int threadbaseline;
	int i;
	int anslen3 = 0, anslen4 = 0, anslen5 = 0, anslen6 = 0, anslen7 = 0;
	int max = NodeNum;
	for (i = 1; i < max; ++i) {
		threadid = threadsPoint[i];
		threadbaseline = THREADBASELINE;
		while (i == threadres[threadbaseline + MERGEOFFSET]) {
			switch (threadres[SIZE])
			{
			case 3:
			{
				memcpy(res3 + 3 * anslen3, threadres + threadbaseline + MERGEOFFSET, sizeof(int) * 3);
				++anslen3;
				++it[threadid];
				break;
			}
			case 4:
			{
				memcpy(res4 + 4 * anslen4, threadres + threadbaseline + MERGEOFFSET, sizeof(int) * 4);
				++anslen4;
				++it[threadid];
				break;
			}
			case 5:
			{
				memcpy(res5 + 5 * anslen5, threadres + threadbaseline + MERGEOFFSET, sizeof(int) * 5);
				++anslen5;
				++it[threadid];
				break;
			}
			case 6:
			{
				memcpy(res6 + 6 * anslen6, threadres + threadbaseline + MERGEOFFSET, sizeof(int) * 6);
				++anslen6;
				++it[threadid];
				break;
			}
			case 7:
			{
				memcpy(res7 + 7 * anslen7, threadres + threadbaseline + MERGEOFFSET, sizeof(int) * 7);
				++anslen7;
				++it[threadid];
				break;
			}
			default:
			{
				break;
			}
			}
		}
		//threadid++;
		//if (threadid == DFSTHREADS_NUM) threadid = 0;
	}
	anslen[3] = anslen3;
	anslen[4] = anslen4;
	anslen[5] = anslen5;
	anslen[6] = anslen6;
	anslen[7] = anslen7;
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
	char *buffer = new char[len];    //æŠŠæ‰€æœ‰å­—ç¬¦ä¸€æ¬¡å…¨éƒ¨è¯»å–åˆ°bufé‡Œé¢ï¼ŒåŒ…æ‹¬â€˜\nâ€?
	if (fread(buffer, 1, len, fp) != len) {
		cout << "read text_data failed" << endl;
	}
	int nums = 0;
	vector<vector<int>> result;
	vector<int> p;
	int countnum = 0;
	for (int i = 0; i < len; i++) {
		if (buffer[i] == '\n') {
			countnum++;//ä¸€è¡Œæ•°æ?
			p.push_back(nums);
			result.push_back(p);
			p.clear();
			nums = 0;
			continue;
		}
		if (buffer[i] == ',') {
			p.push_back(nums);
			countnum++;//ä¸€ä¸ªæ•°æ?
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
	char *buffer1 = new char[len];    //æŠŠæ‰€æœ‰å­—ç¬¦ä¸€æ¬¡å…¨éƒ¨è¯»å–åˆ°bufé‡Œé¢ï¼ŒåŒ…æ‹¬â€˜\nâ€?
	if (fread(buffer1, 1, len, fp1) != len) {
		cout << "read text_data failed" << endl;
	}
	nums = 0;
	vector<vector<int>> Myresult;
	p.clear();
	countnum = 0;
	for (int i = 0; i < len; i++) {
		if (buffer1[i] == '\n') {
			countnum++;//ä¸€è¡Œæ•°æ?
			p.push_back(nums);
			Myresult.push_back(p);
			p.clear();
			nums = 0;
			continue;
		}
		if (buffer1[i] == ',') {
			p.push_back(nums);
			countnum++;//ä¸€ä¸ªæ•°æ?
			nums = 0;
			continue;
		}
		else if (buffer1[i] == '\r') continue;
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
	for (int i = 1; i < line; i++) {
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
					for (int j = 0; j < Myresult[i].size(); j++)cout << Myresult[i][j] << "->";
					cout << endl;
					for (int j = 0; j < result[i].size(); j++)cout << result[i][j] << "->";
					cout << endl;
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
void int2str(const int &value, char *buff, int &length, const char &i)//å­—ç¬¦ä¸²è½¬æ•°å­—å­˜å…¥buff
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
void result_out(int size, int* inputres, int batch, int &length, char *buff) {//2020422å¾ªç¯å±•å¼€
	int k = 0;
	//int temp;
	//int temp_high;
	//int temp_low;
	int begin;
	int max = batch * size;
	int numlength;
	int offset;
	int llength = length;
	for (int j = 0; j < max; ++j)
	{
		++k;
		begin = inputres[j] * 11;
		numlength = resultMap[begin];
		offset = 11 - numlength;
		switch (numlength)
		{
		case 1:
		{
			buff[llength] = resultMap[begin + 10];
			break;
		}
		case 2:
		{
			buff[llength] = resultMap[begin + 9];
			buff[llength + 1] = resultMap[begin + 10];
			break;
		}
		case 3:
		{
			buff[llength] = resultMap[begin + 8];
			buff[llength + 1] = resultMap[begin + 9];
			buff[llength + 2] = resultMap[begin + 10];
			break;
		}
		default:
		{
			memcpy(buff + llength, resultMap + begin + offset, sizeof(char)*numlength);
			break;
		}
		}
		llength += numlength;
		buff[llength++] = ',';
		if (k == batch) {
			buff[llength - 1] = '\n';
			k = 0;
		}
	}
	length = llength;
}

void buildreMap(int id)
{
	int num;
	int total;
	num = Maplist[0];
	total = 10;
	if (id == 0) {
		if (num == 0) resultMap[total--] = '0';
		while (num > 0)
		{
			resultMap[total--] = '0' + num % 10;
			num /= 10;
		}
		resultMap[0] = (10 - total);
		num = Maplist[1];
		total = 10;
		if (num == 0) resultMap[11 + total--] = '0';
		while (num > 0)
		{
			resultMap[11 + total--] = '0' + num % 10;
			num /= 10;
		}
		resultMap[11] = (10 - total);
	}
	for (int i = id + 2; i < NodeNum; i += 4) {
		num = Maplist[i];
		total = 10;
		if (num == 0) resultMap[11 * i + total--] = '0';
		while (num > 0)
		{
			resultMap[11 * i + total--] = '0' + num % 10;
			num /= 10;
		}
		resultMap[11 * i] = (10 - total);
	}
}
void result_outt(int threadid) {
	time_t t = clock();
	int len1, len2, outline;
	divideMsg lmsg = msg[threadid];
	outbuff[threadid] = new char[outNum[threadid] * 80];
	if (threadid == 0)
	{
		outline = anslen[3] + anslen[4] + anslen[5] + anslen[6] + anslen[7];
		int2str(outline, outbuff[0], length[0], '\n');
	}
	switch (lmsg.length)
	{
	case 0:
	{
		result_out(outNum[threadid], ans[lmsg.last - 3] + lmsg.start*lmsg.last, lmsg.last, length[threadid], outbuff[threadid]);
		break;
	}
	case 1:
	{
		len1 = (anslen[lmsg.last] - lmsg.start);
		len2 = outNum[threadid] - len1;
		result_out(len1, ans[lmsg.last - 3] + lmsg.start*lmsg.last, lmsg.last, length[threadid], outbuff[threadid]);
		result_out(len2, ans[lmsg.last - 3 + 1], lmsg.last + 1, length[threadid], outbuff[threadid]);
		break;
	}
	case 2:
	{
		len1 = (anslen[lmsg.last] - lmsg.start);
		len2 = (outNum[threadid] - len1 - anslen[lmsg.last + 1]);
		result_out(len1, ans[lmsg.last - 3] + lmsg.start*lmsg.last, lmsg.last, length[threadid], outbuff[threadid]);
		result_out(anslen[lmsg.last + 1], ans[lmsg.last - 3 + 1], lmsg.last + 1, length[threadid], outbuff[threadid]);
		result_out(len2, ans[lmsg.last - 3 + 2], lmsg.last + 2, length[threadid], outbuff[threadid]);
		break;
	}
	case 3:
	{
		len1 = (anslen[lmsg.last] - lmsg.start);
		len2 = (outNum[threadid] - len1 - anslen[lmsg.last + 1] - anslen[lmsg.last + 2]);
		result_out(len1, ans[lmsg.last - 3] + lmsg.start*lmsg.last, lmsg.last, length[threadid], outbuff[threadid]);
		result_out(anslen[lmsg.last + 1], ans[lmsg.last - 3 + 1], lmsg.last + 1, length[threadid], outbuff[threadid]);
		result_out(anslen[lmsg.last + 2], ans[lmsg.last - 3 + 2], lmsg.last + 2, length[threadid], outbuff[threadid]);
		result_out(len2, ans[lmsg.last - 3 + 3], lmsg.last + 3, length[threadid], outbuff[threadid]);
		break;
	}
	case 4:
	{
		len1 = (anslen[lmsg.last] - lmsg.start);
		len2 = (outNum[threadid] - len1 - anslen[lmsg.last + 1] - anslen[lmsg.last + 2] - anslen[lmsg.last + 3]);
		result_out(len1, ans[lmsg.last - 3] + lmsg.start*lmsg.last, lmsg.last, length[threadid], outbuff[threadid]);
		result_out(anslen[lmsg.last + 1], ans[lmsg.last - 3 + 1], lmsg.last + 1, length[threadid], outbuff[threadid]);
		result_out(anslen[lmsg.last + 2], ans[lmsg.last - 3 + 2], lmsg.last + 2, length[threadid], outbuff[threadid]);
		result_out(anslen[lmsg.last + 3], ans[lmsg.last - 3 + 3], lmsg.last + 3, length[threadid], outbuff[threadid]);
		result_out(len2, ans[lmsg.last - 3 + 4], lmsg.last + 4, length[threadid], outbuff[threadid]);
		break;
	}
	}
	cout << clock() - t << endl;
}
void save(const string &file) {
	divideMsg msgD;
	int divideNum = totalDivide;
	int anslength = 0;
	int last = 0;
	int i;

	thread threads_tran[4];
	threads_tran[0] = thread(buildreMap, 0);
	threads_tran[1] = thread(buildreMap, 1);
	threads_tran[2] = thread(buildreMap, 2);
	threads_tran[3] = thread(buildreMap, 3);
	threads_tran[0].join();
	threads_tran[1].join();
	threads_tran[2].join();
	threads_tran[3].join();

	for (i = 0; i<THREADS_NUM; i++)
	{
		if (msgD.last == 0)
		{
			msgD.last = 3;
			while (divideNum>0)
			{
				divideNum -= anslen[anslength + msgD.last] * (anslength + msgD.last);
				outNum[i] += anslen[anslength + msgD.last];
				anslength++;
			}
			anslength--;
			last = anslen[anslength + 3] - (int)(-divideNum / (anslength + 3));
			outNum[i] -= (anslen[anslength + 3] - last);
			msgD.length = anslength;
			msg[i] = msgD;
		}
		else
		{
			divideNum = totalDivide;
			if (last == 0) anslength++;
			msgD.last = msgD.last + anslength;
			anslength = 0;
			if (anslen[msgD.last] * msgD.last - last*msgD.last - divideNum >= 0)
			{
				msgD.length = 0;
				msgD.start = last;
				if (i == THREADS_NUM - 1) outNum[THREADS_NUM - 1] = anslen[7] - msgD.start;
				else outNum[i] = divideNum / msgD.last;
				if (anslen[msgD.last] * msgD.last - last*msgD.last - divideNum == 0) last = 0;

				else last = last + divideNum / msgD.last;


			}
			else
			{
				if (last != 0)
				{

					divideNum -= (anslen[msgD.last] - last)*msgD.last;
					outNum[i] += anslen[msgD.last] - last;
					anslength++;
				}
				while (anslength + msgD.last<8 && divideNum>0)
				{
					divideNum -= anslen[anslength + msgD.last] * (anslength + msgD.last);
					outNum[i] += anslen[anslength + msgD.last];
					anslength++;
				}
				msgD.length = --anslength;
				msgD.start = last;

				last = anslen[anslength + msgD.last] - (int)(-divideNum / (anslength + msgD.last));
				if (i <THREADS_NUM - 1) outNum[i] -= (anslen[anslength + msgD.last] - last);
			}
			msg[i] = msgD;
		}
	}
	thread threads[THREADS_NUM];

	threads[0] = thread(result_outt, 0);
	threads[1] = thread(result_outt, 1);
	threads[2] = thread(result_outt, 2);
	threads[3] = thread(result_outt, 3);
	//threads[4] = thread(result_outt, 4);
	//threads[5] = thread(result_outt, 5);
	//threads[6] = thread(result_outt, 6);
	//threads[7] = thread(result_outt, 7);


	FILE *fp;
	if ((fp = fopen(file.c_str(), "wb")) == NULL) {
		cout << "File open error!" << endl;
		exit(0);
	}

	for (i = 0; i < THREADS_NUM; i += 4) {
		threads[i].join();
		fwrite(outbuff[i], sizeof(char), length[i], fp);
		threads[i + 1].join();
		fwrite(outbuff[i + 1], sizeof(char), length[i + 1], fp);
		threads[i + 2].join();
		fwrite(outbuff[i + 2], sizeof(char), length[i + 2], fp);
		threads[i + 3].join();
		fwrite(outbuff[i + 3], sizeof(char), length[i + 3], fp);
	}
	fclose(fp);

}

int main()
{
#ifdef TEST
	Timer t;
	t.start();
#endif
	buildGraph();//å»ºå›¾
	cout << "BuildGraph";
#ifdef TEST
	t.end();
	t.start();
#endif

	thread threadsdfs[DFSTHREADS_NUM];
	int i;
	for (i = 0; i < DFSTHREADS_NUM; i += 4) {
		threadsdfs[i] = thread(taowa7, i, ref(threadNUM[i]));
		threadsdfs[i + 1] = thread(taowa7, i + 1, ref(threadNUM[i + 1]));
		threadsdfs[i + 2] = thread(taowa7, i + 2, ref(threadNUM[i + 2]));
		threadsdfs[i + 3] = thread(taowa7, i + 3, ref(threadNUM[i + 3]));
	}
	for (i = 0; i < DFSTHREADS_NUM; i += 4) {
		threadsdfs[i].join();
		threadsdfs[i + 1].join();
		threadsdfs[i + 2].join();
		threadsdfs[i + 3].join();
	}
	cout << "DFS";


#ifdef TEST
	t.end();
	t.start();
#endif
	merge_res();
	cout << "anslen is" << anslen[3] + anslen[4] + anslen[5] + anslen[6] + anslen[7] << endl;
	totalDivide = (anslen[3] * 3 + anslen[4] * 4 + anslen[5] * 5 + anslen[6] * 6 + anslen[7] * 7) / THREADS_NUM;
	cout << "merge";
#ifdef TEST
	t.end();

	t.start();
#endif
	save(MyanswerFile);

	delete[] res3;
	delete[] res4;
	delete[] res5;
	delete[] res6;
	delete[] res7;
	res3 = NULL;
	res4 = NULL;
	res5 = NULL;
	res6 = NULL;
	res7 = NULL;

#ifdef TEST
	cout << "output";
	t.end();
#endif
	cout << "finished" << endl;

#ifdef TEST
	//loadresult();//è®¡æ—¶

	//while (1);
#endif

	exit(0);
	return 0;
}

