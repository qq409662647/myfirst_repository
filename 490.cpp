/*
--------------
finalBaseline 2020.5.21
多线程建图+dij优先队列+0n3合并查Cn
--------------
*/

#include <cstdio>
#include <queue>
#include <stack>
#include <vector>
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
#include<map>
//#include<unistd.h>

using namespace std;

//#define TEST

#define CacheLineSize 128
#define THREADS_NUM 8
#define DIJTHREADS_NUM 12
#define MAX_INSIZE 2500000
#define MAX_OUTSIZE 100
#define THREADOUT 100/THREADS_NUM
#ifdef TEST
#include "timer.h"
string testFile = "/data/test_data.txt";//_10000_40000
string answerFile = "/data/result.txt";
string MyanswerFile = "/projects/student/result.txt";//_10000_40000
#else
string testFile = "/data/test_data.txt";//_10000_40000
string answerFile = "/data/result.txt";
string MyanswerFile = "/projects/student/result.txt";//_10000_40000
#endif
int item = CacheLineSize / sizeof(int);
//建图存边
typedef struct edgeInfo {
	uint32_t id;
	uint32_t money;
	edgeInfo() :id(0), money(0) {}
	edgeInfo(int id, int money) :id(id), money(money) {}

}edge;
//dij节点
struct node {
	uint32_t id;
	uint32_t dis;
	node(uint32_t id, uint32_t dis) :id(id), dis(dis) {}
	node(uint32_t id, uint32_t dis, uint32_t step) :id(id), dis(dis) {}
	node() :id(0), dis(0) {}
	const bool operator < (const node &a) const {
		return dis < a.dis;
	}
	const bool operator > (const node &a) const {
		return dis > a.dis;
	}
};
//输出节点
struct ansNode {
	uint32_t id;
	//fraction fra;
	double Cn;
	ansNode() :id(0), Cn(0.00) {}
	const bool operator < (const ansNode&a)const {
		return Cn < a.Cn;
	}
	const double operator -(const ansNode&a)const {
		return Cn - a.Cn;
	}
};

template<class T>
//两种不同的优先队列，用于不同的图
class PriorityQueue
{
private:
	int Capacity = MAX_INSIZE;  //队列容量
	int size;            //队列大小
	T *data;             //队列变量

public:
	PriorityQueue()
	{
		data = (T*)malloc(Capacity * sizeof(T));
		if (!data)
		{
			perror("Allocate dynamic memory");
			return;
		}
		size = 0;
	};
	~PriorityQueue()
	{
		while (!empty())
		{
			pop();
		}
	};

	bool empty()
	{
		if (size > 0)
		{
			return false;
		}
		return true;
	}
	void push(T key)
	{
		//空队则直接入队
		if (empty())
		{
			data[++size] = key;
			return;
		}

		int i;
		for (i = ++size; data[i / 2] > key; i /= 2)
		{
			data[i] = data[i / 2];
		}
		data[i] = key;
	}  //入队
	void pop()
	{
		int i, child;
		T min, last;

		if (empty())
		{
			perror("Empty queue\n");
			return;
		}
		last = data[size--];

		for (i = 1; i * 2 <= size; i = child)
		{
			child = i * 2;
			if (child != size && data[child + 1] < data[child])
			{
				child++;
			}
			if (last > data[child])
			{
				data[i] = data[child];
			}
			else
			{
				break;
			}
		}
		data[i] = last;
	}   //出队
	T top()
	{
		if (empty())
		{
			perror("Priority queue is full\n");
			return data[0];
		}
		return data[1];
	}          //队首
};
class prio_queue_dense {
public:
	prio_queue_dense() {
		A = new node[MAX_INSIZE];
		index = new uint32_t[MAX_INSIZE]();
		size = 0;
	}
	bool empty() { return size == 0; }
	size_t getsize() { return size; }
	void clear() { size = 0; }
	void push(node node) {
		int pos = index[node.id];
		if (pos == 0) {
			A[++size] = node;
			up(size);
		}
		else {
			A[pos].dis = node.dis;
			up(pos);
		}
	}
	node top() {
		return A[1];
	}
	void pop() {
		index[A[1].id] = 0;
		A[1] = A[size];
		index[A[size--].id] = 0;
		if (size) down(1);
	}
private:
	int parent(int curr) { return (curr >> 1); }
	int left(int curr) { return (curr << 1); }
	int right(int curr) { return ((curr << 1) + 1); }
	void down(int curr) {
		A[0] = A[curr];
		int l, r, min;
		while (1) {
			l = left(curr), r = right(curr);
			if (l <= size && A[l] < A[0]) min = l;
			else min = 0;
			if (r <= size && A[r] < A[min]) min = r;
			if (min != 0) {
				A[curr] = A[min];
				index[A[curr].id] = curr;
				curr = min;
			}
			else break;
		}
		A[curr] = A[0];
		index[A[curr].id] = curr;
	}
	void up(int curr) {
		node temp = A[curr];
		while (curr > 1 && temp < A[parent(curr)]) {
			A[curr] = A[parent(curr)];
			index[A[curr].id] = curr;
			curr = parent(curr);
		}
		A[curr] = temp;
		index[temp.id] = curr;
	}
	node* A;
	size_t size;
	uint32_t* index; // 可以换成u16和u32各一个
};

struct djiInfo
{
	uint16_t prelen;
	uint16_t num;
	uint32_t dis;
	djiInfo() :prelen(0), num(1), dis(UINT32_MAX) {}//图1采用数据结构及大小
};
unordered_map<uint32_t, uint32_t> Mapnode;//存图
uint32_t Maplist[MAX_INSIZE + 1];//逆图（id映射表）
uint32_t vislist[MAX_INSIZE + 1];//访问序列表
uint32_t NEWMaplist[MAX_INSIZE + 1];//访问序列表
uint32_t Maphead[MAX_INSIZE + 1] = { 0 };//出度前向星头节点
uint32_t NMaphead[MAX_INSIZE + 1] = { 0 };//入度前向星头节点
edge Map[MAX_INSIZE + 1];//前向星各点及出度点
edge NMap[MAX_INSIZE + 1];//前向星入度点
uint32_t NEWMaphead[MAX_INSIZE + 1] = { 0 };//出度前向星头节点
uint32_t NEWNMaphead[MAX_INSIZE + 1] = { 0 };//入度前向星头节点
edge NEWMap[MAX_INSIZE + 1];//前向星各点及出度点
edge NEWNMap[MAX_INSIZE + 1];//前向星入度点



uint32_t insize = 0;
uint32_t outsize = 0;
uint16_t Kn[MAX_INSIZE + 1] = { 0 };
int threadreadline[THREADS_NUM] = { 0 };//数据行数
uint32_t *threadin_data[THREADS_NUM];//多线程读入
uint32_t in_data[MAX_INSIZE * 3];//合并后的数据
int NodeNum = 1;//转换后的起始数据
int readline;//读入数据长度
int pos[5];//初始建图记录各线程len范围


		   //图2采用数据结构及大小
uint32_t pre[DIJTHREADS_NUM][MAX_INSIZE + 1];//类逆邻接表存储前驱结点
uint32_t *pres[DIJTHREADS_NUM];
uint16_t dis[DIJTHREADS_NUM][MAX_INSIZE + 1];
uint8_t num[DIJTHREADS_NUM][MAX_INSIZE + 1] = { 0 };//num<5000？？？
uint8_t prelen[DIJTHREADS_NUM][MAX_INSIZE + 1] = { 0 };
//图1采用数据结构及大小
djiInfo info[DIJTHREADS_NUM][MAX_INSIZE + 1];


double tempres[DIJTHREADS_NUM][MAX_INSIZE + 1] = { 0.0L };
int outPos[9] = { 1,THREADOUT ,THREADOUT * 2,THREADOUT * 3,THREADOUT * 4,THREADOUT * 5,THREADOUT * 6,THREADOUT * 7,101 };//结果输出定位
ansNode ans[MAX_INSIZE + 1];//输出节点矩阵
							//double tmpAns[DIJTHREADS_NUM][MAX_INSIZE + 1] = { 0.0 };
double tmpAnsd[DIJTHREADS_NUM][MAX_INSIZE + 1] = { 0.0 };
//fra tmpAns[DIJTHREADS_NUM][MAX_INSIZE + 1];
char *outbuff[THREADS_NUM];//多线程输出buffer
int length[THREADS_NUM] = { 0 };//buffer长度
atomic<uint32_t>threadCount(0);//dij原子变量
							   /***进度条***/
char bar[101];
int barCount = 0;
double barNow = 0.0;
int isjudge = 0;
void transform_num(char* buffer, int threadid)
{
	threadin_data[threadid] = new uint32_t[MAX_INSIZE];
	uint32_t readlinet = 0;
	uint32_t nums = 0;
	uint32_t p = 0;
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
	char *buffer = new char[len];    //把所有字符一次全部读取到buf里面，包括‘\n‿
	if (fread(buffer, 1, len, fp) != len) {
		cout << "read text_data failed" << endl;
		return false;
	}
	fclose(fp);
	//输入分成4块分别输入
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
	for (int i = 0; i < 4; i++)
	{
		readthread[i] = thread(transform_num, buffer, i);
	}
	for (int i = 0; i < 4; i++)
	{
		readthread[i].join();
	}

	return true;
}
bool cmp(edge id1, edge id2) {
	return id1.id < id2.id;
}
void maphash(int threadid) {
	auto tempy = Mapnode.begin(), tempx = Mapnode.begin();
	int readmax = readline;
	for (int i = threadid; i < readmax; i += 4) {
		tempy = Mapnode.find(in_data[3 * i + 1]);
		tempx = Mapnode.find(in_data[3 * i]);
		in_data[3 * i] = tempx->second;
		in_data[3 * i + 1] = tempy->second;
	}
}
//前向星存图
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
void buildMap() {
	edge temp;
	int *curlen = new int[NodeNum]();
	int *maplen = new int[NodeNum]();
	int i;
	int readmax = readline;
	int max = NodeNum;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 2] != 0) {
			maplen[in_data[3 * i]]++;
		}
	}
	for (i = 1; i <= max; ++i) {
		Maphead[i] = Maphead[i - 1] + maplen[i - 1];
	}
	int tempx;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 2] != 0) {
			tempx = in_data[3 * i];//x
			temp.id = in_data[3 * i + 1];
			temp.money = in_data[3 * i + 2];
			Map[Maphead[tempx] + curlen[tempx]] = temp;
			curlen[tempx]++;
		}
	}

}
void buildNMap() {
	edge temp;
	int *curlen1 = new int[NodeNum]();
	int *nmaplen = new int[NodeNum]();
	int i;
	int readmax = readline;
	int max = NodeNum;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 2] != 0) {
			nmaplen[in_data[3 * i + 1]]++;
		}
	}
	for (i = 1; i <= max; ++i) NMaphead[i] = NMaphead[i - 1] + nmaplen[i - 1];
	int tempy;
	for (i = 0; i < readmax; ++i) {
		if (in_data[3 * i + 2] != 0) {
			tempy = in_data[3 * i + 1];//x
			temp.id = in_data[3 * i];
			temp.money = in_data[3 * i + 2];
			NMap[NMaphead[tempy] + curlen1[tempy]] = temp;
			curlen1[tempy]++;
		}
	}
}
void buildGraph() {
	if (bool status = fread_read(testFile)) {
		cout << "load text_data succeed" << endl;
	}
	else {
		cout << "load text_data failed" << endl;
	}
	//多线程合并
	memcpy(in_data, threadin_data[0], sizeof(uint32_t) * 3 * threadreadline[0]);
	memcpy(in_data + threadreadline[0] * 3, threadin_data[1], sizeof(uint32_t) * 3 * threadreadline[1]);
	memcpy(in_data + threadreadline[0] * 3 + threadreadline[1] * 3, threadin_data[2], sizeof(uint32_t) * 3 * threadreadline[2]);
	memcpy(in_data + threadreadline[0] * 3 + threadreadline[1] * 3 + threadreadline[2] * 3, threadin_data[3], sizeof(uint32_t) * 3 * threadreadline[3]);
	readline = threadreadline[0] + threadreadline[1] + threadreadline[2] + threadreadline[3];
	delete[]threadin_data[0];
	delete[]threadin_data[1];
	delete[]threadin_data[2];
	delete[]threadin_data[3];
	threadin_data[0] = NULL;
	threadin_data[1] = NULL;
	threadin_data[2] = NULL;
	threadin_data[3] = NULL;
	int i = 0;
	int readmax = readline;
	int max = NodeNum;
	Mapnode.reserve(2500000);
	auto it = Mapnode.begin();
	//mapnode 无序点集
	//maplist逆转换
	for (i = 0; i < readmax; i++) {
		if (in_data[3 * i + 2] == 0) continue;
		it = Mapnode.find(in_data[3 * i]);
		if (it == Mapnode.end())
		{
			Mapnode[in_data[3 * i]] = max;
			Maplist[max++] = in_data[3 * i];
		}
		else outsize++;
		it = Mapnode.find(in_data[3 * i + 1]);
		if (it == Mapnode.end())
		{
			Mapnode[in_data[3 * i + 1]] = max;
			Maplist[max++] = in_data[3 * i + 1];
		}
		else insize++;
	}
	NodeNum = max;
	if (MAX_INSIZE>(NodeNum * 10) && readline>MAX_INSIZE / 5)//跳小图（边小于5W归类到稀疏图处理）
	{
		isjudge = 1;//标记图为稠密图
	}
	cout << "maplist success" << endl;
	cout << "new success" << endl;
	thread mapsh[4];
	mapsh[0] = thread(maphash, 0);
	mapsh[1] = thread(maphash, 1);
	mapsh[2] = thread(maphash, 2);
	mapsh[3] = thread(maphash, 3);
	mapsh[0].join();
	mapsh[1].join();
	mapsh[2].join();
	mapsh[3].join();

	buildMap();
	buildNMap();
	//按bfs重新排列id
	bool *vis = new bool[NodeNum + 1]();
	uint32_t id, cur;
	int j = 0;
	int cnt = 1;
	queue<uint32_t> Q;
	for(i = 1; i < NodeNum; i++) {
		if (!vis[i]) {
			Q.push(i);
			while (!Q.empty()) {
				cur = Q.front();
				Q.pop();
				if (!vis[cur]) {
					vis[cur] = 1;
					vislist[cur] = cnt++;//maplist i->排序
					for (j = Maphead[i]; j < Maphead[i + 1]; j++) {
						id = Map[j].id;
						if (!vis[id]) Q.push(id);
					}
				}
			}
		}
	}
	uint32_t *newhasp =new uint32_t[NodeNum + 1];
	for (i = 1; i < NodeNum; i++) { //新映射表
		NEWMaplist[vislist[i]] = Maplist[i];
		ans[vislist[i]].id = Maplist[i];
	}
	cout << cnt<< "/"<<NodeNum<< endl;
	memset(vis,0,sizeof(bool)*NodeNum);
	for (int i = 0; i < readmax; i++) {
			in_data[3 * i] = vislist[in_data[3 * i]];
			in_data[3 * i + 1] = vislist[in_data[3 * i + 1]];
	}

	buildMap();
	buildNMap();
	if (insize < outsize) {//当图特征为图一的反特征，交换出入度
		uint32_t *temp1 = new uint32_t[NodeNum + 1];
		edge *temp2 = new edge[MAX_INSIZE + 1];
		memcpy(temp1, Maphead, sizeof(uint32_t)*(NodeNum + 1));
		memcpy(Maphead, NMaphead, sizeof(uint32_t)*(NodeNum + 1));
		memcpy(NMaphead, temp1, sizeof(uint32_t)*(NodeNum + 1));
		memcpy(temp2, Map, sizeof(edge)*(MAX_INSIZE + 1));
		memcpy(Map, NMap, sizeof(edge)*(MAX_INSIZE + 1));
		memcpy(NMap, temp2, sizeof(edge)*(MAX_INSIZE + 1));
		for (i = 0; i < max; i++) {
			if (NMaphead[i] == NMaphead[i + 1] && Maphead[i + 1] - Maphead[i] == 1) {//入度为0，出度为1节点
				Kn[Map[Maphead[i]].id]++;//子节点标记2×n
			}
		}
	}
	else {
		for (i = 0; i < max; i++) {
			if (NMaphead[i] == NMaphead[i + 1] && Maphead[i + 1] - Maphead[i] == 1) {//入度为0，出度为1节点
				Kn[Map[Maphead[i]].id]++;//子节点标记2×n
			}
		}
	}
	
	cout << "success" << endl;

}

void mdij(int threadid) {
	pres[threadid] = new uint32_t[256 * (NodeNum + 1)];
	uint32_t *prel = new uint32_t[NodeNum]();
	uint32_t tmp, s;
	uint32_t *sign = new uint32_t[NodeNum + 1];
	uint32_t signnum = 0;
	uint32_t w, v;
	uint32_t cur;
	edge to;
	uint32_t dislast = 0;
	int i, j;
	switch (isjudge)
	{
	case 0:
	{
		PriorityQueue <node> q;//优先队列	
							   //uint32_t *stack = new uint32_t[NodeNum * 1000];//size?
		djiInfo init, infoCur, infoTo;
		init.dis = 0;
		while ((tmp = threadCount.load()) < NodeNum - 1) {
			while (!threadCount.compare_exchange_strong(tmp, tmp + 1)) {};
			s = tmp + 1;
			if (Maphead[s] == Maphead[s + 1] || (Maphead[s + 1] - Maphead[s] == 1 && NMaphead[s] == NMaphead[s + 1])) continue;
			signnum = 0;
			//初始化参数
			info[threadid][s] = init;
			q.push(node(s, 0));
			while (!q.empty()) {
				cur = q.top().id;
				dislast = q.top().dis;
				q.pop();
				infoCur = info[threadid][cur];
				if (infoCur.dis < dislast) continue;
				sign[signnum++] = cur;
				for (i = Maphead[cur]; i < Maphead[cur + 1]; i++) {//逆序访问
					to = Map[i];
					infoTo = info[threadid][to.id];
					if (infoTo.dis >to.money + infoCur.dis) {
						//更新节点和路径
						infoTo.dis = to.money + infoCur.dis;
						infoTo.num = info[threadid][cur].num;
						infoTo.prelen = 0;
						q.push(node(to.id, infoTo.dis));
						info[threadid][to.id] = infoTo;
						prel[to.id] = cur;

					}
					else if (infoTo.dis == to.money + infoCur.dis)
					{
						//若相等则将新的路径加入
						infoTo.num += infoCur.num;
						pre[threadid][NMaphead[to.id] + infoTo.prelen] = cur;
						++infoTo.prelen;
						info[threadid][to.id] = infoTo;
					}
				}
			}
			for (i = 0; i < signnum; ++i) {

				info[threadid][sign[i]].dis = UINT32_MAX;
			}
			for (i = signnum - 1; i >= 0; --i) {
				w = sign[i];//Pop w
				v = prel[w];
				prel[w] = 0;
				tempres[threadid][v] += (tempres[threadid][w] + 1)*(info[threadid][v].num * 1.0 / info[threadid][w].num);
				for (j = 0; j < info[threadid][w].prelen; ++j) {
					v = pre[threadid][NMaphead[w] + j];
					tempres[threadid][v] += (tempres[threadid][w] + 1)*(info[threadid][v].num * 1.0 / info[threadid][w].num);
				}
				if (w != s) tmpAnsd[threadid][w] += tempres[threadid][w];
				tmpAnsd[threadid][w] += Kn[s] * tempres[threadid][w];
				tempres[threadid][w] = 0;
			}
			if (s % 5000 == 0) cout << "xishu" << s << "/" << NodeNum << " finished!\n";
		}
		break;
	}
	case 1:
	{
		memset(dis[threadid], 0xff, sizeof(uint16_t)*(NodeNum + 1));

		prio_queue_dense q;


		while ((tmp = threadCount.load()) < NodeNum - 1) {
			while (!threadCount.compare_exchange_strong(tmp, tmp + 1)) {};
			s = tmp + 1;
			if (Maphead[s] == Maphead[s + 1] || (Maphead[s + 1] - Maphead[s] == 1 && NMaphead[s] == NMaphead[s + 1])) continue;

			signnum = 0;
			//初始化参数
			dis[threadid][s] = 0;
			prelen[threadid][s] = 0;
			num[threadid][s] = 1;
			q.push(node(s, 0));
			while (!q.empty()) {
				cur = q.top().id;
				dislast = q.top().dis;
				q.pop();
				if (dis[threadid][cur] < dislast) continue;
				sign[signnum++] = cur;
				for (i = Maphead[cur]; i < Maphead[cur + 1]; i++) {//逆序访问
					to = Map[i];

					if (dis[threadid][to.id] >to.money + dis[threadid][cur]) {
						//更新节点和路径
						dis[threadid][to.id] = to.money + dis[threadid][cur];
						q.push(node(to.id, dis[threadid][to.id]));
						num[threadid][to.id] = num[threadid][cur];
						prel[to.id] = cur;
						prelen[threadid][to.id] = 0;
					}
					else if (dis[threadid][to.id] == to.money + dis[threadid][cur])
					{
						//若相等则将新的路径加入
						num[threadid][to.id] += num[threadid][cur];
						pres[threadid][to.id * 256 + prelen[threadid][to.id]] = cur;
						++prelen[threadid][to.id];
					}
				}
			}
			for (i = 0; i < signnum; ++i) {
				dis[threadid][sign[i]] = UINT16_MAX;
			}
			for (i = signnum - 1; i >= 0; --i) {
				w = sign[i];//Pop w
				v = prel[w];
				prel[w] = 0;
				tempres[threadid][v] += (tempres[threadid][w] + 1)*(num[threadid][v] * 1.0 / num[threadid][w]);
				for (j = 0; j < prelen[threadid][w]; ++j) {
					v = pres[threadid][w * 256 + j];
					tempres[threadid][v] += (tempres[threadid][w] + 1)*(num[threadid][v] * 1.0 / num[threadid][w]);
				}
				if (w != s) tmpAnsd[threadid][w] += tempres[threadid][w];
				tmpAnsd[threadid][w] += Kn[s] * tempres[threadid][w];
				tempres[threadid][w] = 0;
			}

			if (s % 5000 == 0) cout << "choumi" << s << "/" << NodeNum << " finished!\n";
		}
		break;
	}
	}

}
void merge_result()
{
	for (int i = 0; i < NodeNum; i++)
	{
		ans[i].Cn += tmpAnsd[0][i] + tmpAnsd[1][i] + tmpAnsd[2][i] + tmpAnsd[3][i] + tmpAnsd[4][i] + tmpAnsd[5][i] + \
		tmpAnsd[6][i] + tmpAnsd[7][i]+tmpAnsd[8][i] + tmpAnsd[9][i]+tmpAnsd[10][i] + tmpAnsd[11][i];
	}
}
void sortAns()
{
	pair<double, uint32_t>maxCn;
	uint32_t i, j;
	for (i = 1; i <= 100 && i<NodeNum; i++)
	{
		maxCn = { ans[i].Cn,i };
		for (j = i + 1; j < NodeNum; j++)
		{
			if (abs(maxCn.first - ans[j].Cn) < 0.0001) maxCn = ans[maxCn.second].id > ans[j].id ? pair<double, uint32_t>(ans[j].Cn, j) : maxCn;
			else maxCn = maxCn.first < ans[j].Cn ? pair<double, uint32_t>(ans[j].Cn, j) : maxCn;
		}
		if (i == maxCn.second) continue;
		swap(ans[i], ans[maxCn.second]);
	}
}
void buildConvert(uint64_t numPoint, int &length, char *buff)
{
	int strLength = 20;
	char out[21];
	if (numPoint == 0)
	{
		buff[length++] = '0';
		return;
	}
	while (numPoint > 0)
	{
		out[strLength--] = '0' + numPoint % 10;
		numPoint /= 10;
	}
	memcpy(buff + length, out + strLength + 1, 20 - strLength);
	length += 20 - strLength;
}
void result_out(int threadid) {//2020422循环展开
	uint64_t tmpres;
	uint64_t resd;
	uint64_t resf;
	outbuff[threadid] = new char[450];
	int llength = length[threadid];
	for (int j = outPos[threadid]; j < outPos[threadid + 1]; ++j)
	{
		//存节点
		buildConvert((uint64_t)ans[j].id, llength, outbuff[threadid]);
		outbuff[threadid][llength++] = ',';
		tmpres = (uint64_t)(ans[j].Cn * 10000);
		if (tmpres % 10 > 4) tmpres += 10;
		tmpres /= 10;
		resf = (tmpres % 1000);
		resd = (tmpres / 1000);
		//cout << resd << "." << resf << endl;
		//存整数部分
		buildConvert(resd, llength, outbuff[threadid]);
		outbuff[threadid][llength++] = '.';
		//存小数部分
		if (resf >= 100)
		{
			outbuff[threadid][llength++] = resf / 100 + '0';
			outbuff[threadid][llength++] = resf / 10 % 10 + '0';
			outbuff[threadid][llength++] = resf % 10 + '0';
		}
		else if (resf >= 10)
		{
			outbuff[threadid][llength++] = '0';
			outbuff[threadid][llength++] = resf / 10 + '0';
			outbuff[threadid][llength++] = resf % 10 + '0';
		}
		else
		{
			outbuff[threadid][llength++] = '0';
			outbuff[threadid][llength++] = '0';
			outbuff[threadid][llength++] = resf + '0';
		}
		outbuff[threadid][llength++] = '\n';

	}
	length[threadid] = llength;
}
void save(const string &file) {
	int i;
	thread resbuff[THREADS_NUM];
	for (i = 0; i < THREADS_NUM; i++)
	{
		resbuff[i] = thread(result_out, i);
	}
	FILE *fp;
	if ((fp = fopen(file.c_str(), "wb")) == NULL) {
		cout << "File open error!" << endl;
		exit(0);
	}
	for (i = 0; i < THREADS_NUM; i++)
	{
		resbuff[i].join();
		fwrite(outbuff[i], sizeof(char), length[i], fp);
	}
	fclose(fp);
}
int main()
{
#ifdef TEST
	Timer t;
	t.start();
#endif
	buildGraph();//建图
	cout << "BuildGraph" << endl;
#ifdef TEST
	t.end();
	t.start();
#endif
	thread threads[DIJTHREADS_NUM];
	int i;
	for (i = 0; i < DIJTHREADS_NUM; i += 4) {
		threads[i] = thread(mdij, i);
		threads[i + 1] = thread(mdij, i + 1);
		threads[i + 2] = thread(mdij, i + 2);
		threads[i + 3] = thread(mdij, i + 3);
	}
	for (i = 0; i < DIJTHREADS_NUM; i += 4) {
		threads[i].join();
		threads[i + 1].join();
		threads[i + 2].join();
		threads[i + 3].join();
	}
	merge_result();
	cout << "DIJ finished" << endl;
	sortAns();
	cout << "SORT finished" << endl;

#ifdef TEST
	t.end();
	t.start();
#endif
	save(MyanswerFile);
#ifdef TEST
	cout << "output";
	t.end();
#endif
	cout << "finished" << endl;
	exit(0);
	return 0;
}