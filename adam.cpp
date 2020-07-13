#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

typedef struct Data {
	double * features;
	int label;
}Dt;
typedef struct Param {
	double* wtSet;
	double bSet;
}Param;


class LR {
public:
	void train();
	void predict();
	//int loadModel();
	int storeModel();
	LR(string trainFile, string testFile, string predictOutFile);
	int testlength;
private:
	Dt* trainDataSet;
	Dt* testDataSet;
	int * predictVec;
	Param  param;
	string trainFile;
	string testFile;
	string predictOutFile;
	string weightParamFile = "modelweight.txt";

private:
	bool init();
	bool loadTestData();
	int storePredict(int* predict);
	void initParam();
	double wxbCalc(Dt * data);
	double sigmoidCalc(const double wxb);
	double MlossCal();
	double lossCal(int start);
	double MgradientSlope(const  Dt * data, int index, const double * sigmoidVec, double &db);
	double gradientSlope(const  Dt * data, int index, int start, const double * sigmoidVec, double &db);
	void transform_num(char* buffer, int lenght, Dt * data);
	bool fread_read(string file, Dt * data);
	double norm(Dt * data);
private:
	int featuresNum;
	const double wtInitV = 0;
	const double biasInit = 0;
	const double stepSize = 0.001;
	const int minbatch = 10;//小批量
	const int maxIterTimes =1200;
	const double predictTrueThresh = 0.5;
	const int train_show_step = 1000;
	const int readline =8000;
	const double beta1 = 0.9;
	const double beta2 = 0.999;
	const double e = 1e-8;
};

LR::LR(string trainF, string testF, string predictOutF)
{
	trainFile = trainF;
	testFile = testF;
	predictOutFile = predictOutF;
	featuresNum = 0;
	init();
}

void LR::transform_num(char* buffer, int lenght, Dt* data)
{
	bool is_dec = false;
	double nums = 0.0L;
	data[0].features = new double[20001];
	double*p = data[0].features;//feature[0];
	int k = 0;
	int nt = 10;
	int countnum = 0;
	bool sign = false;
	for (int i = 0; i < lenght; i++) {
		if (buffer[i] == ',') {
			if (sign) nums = -nums;
			*p = nums;
			p++;
			countnum++;
			nums = 0.0L;
			nt = 10;
			sign = false;
			is_dec = false;
			continue;
		}
		else if (buffer[i] == '\n') {
			featuresNum = countnum;//记录特征值数量
			countnum = 0;
			data[k].label = (int)nums;//存入特征值
			k++;
			data[k].features = new double[20001];
			p = data[k].features;
			nums = 0.0L;
			nt = 10;
			sign = false;
			is_dec = false;
			if (k == readline) {
				break;
				delete p;
			}
			continue;
		}
		else if (buffer[i] == '-') {
			sign = true;
		}
		else if (buffer[i] == '.') {
			is_dec = true;
			i++;
		}
		if (!is_dec)nums = buffer[i] - '0';
		else {
			nums += (double)(buffer[i] - '0') / nt;
			nt *= 10;
		}
	}
}

bool LR::fread_read(string file, Dt * data)
{
	FILE *fp;
	if ((fp = fopen(file.c_str(), "r")) == NULL) {
		cout << "打开文件错误" << endl;
		return false;
	}
	fseek(fp, 0L, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	cout << "loading" << endl;
	char *buffer = new char[len];    //把所有字符一次全部读取到buf里面，包括‘\n‘
	if (fread(buffer, 1, len, fp) != len) {
		cout << "读取失败" << endl;
		return false;
	}
	transform_num(buffer, len, data);
	fclose(fp);
	return true;
}

void LR::initParam()
{
	int i;
	param.wtSet = new double[featuresNum];
	for (i = 0; i < featuresNum; i++) {
		param.wtSet[i] = wtInitV;
	}
	param.bSet = biasInit;

}

bool LR::init()
{
	trainDataSet = new Dt[readline + 1];
	bool status = fread_read(trainFile, trainDataSet);
	if (status != true) {
		return false;
	}
	initParam();
	return true;
}


double LR::wxbCalc(Dt* data)//wx+b
{
	double mulSum = 0.0L;
	int i;
	double wtv, feav;
	for (i = 0; i < featuresNum; i++) {
		wtv = param.wtSet[i];
		feav = data->features[i];
		mulSum += wtv * feav;

	}
	return mulSum + param.bSet;
}

double LR::norm(Dt * data)
{
	double mulSum = 0.0L;
	int i, j;
	double E = 0.0, D = 0.0;
	for (i = 0; i < readline; i++) {
		for (j = 0; i < featuresNum; i++) {
			E += data[0].features[j];
		}
		E /= featuresNum;
		for (j = 0; i < featuresNum; i++) {
			D += (data[0].features[j] - E)*(data[0].features[j] - E);
		}
		D /= featuresNum;
		D = sqrt(D);
		for (j = 0; i <featuresNum; i++) {
			data[i].features[j] = (data[0].features[j] - E) / D;
		}
	}

	return mulSum;
}


inline double LR::sigmoidCalc(const double wxb)
{
	double expv = exp(-1 * wxb);
	double expvInv = 1 / (1 + expv);
	return expvInv;
}


double LR::MlossCal()
{
	double lossV = 0.0L;
	int i;

	for (i = 0; i < readline; i++) {
		lossV += trainDataSet[i].label * log(sigmoidCalc(wxbCalc(trainDataSet + i)));
		lossV += (1 - trainDataSet[i].label)*log(1 - sigmoidCalc(wxbCalc(trainDataSet + i)));
	}
	lossV /= readline;
	return -lossV;
}


///*  批量梯度
double LR::lossCal(int start)
{
	double lossV = 0.0L;
	int i;
	int end = start + minbatch;
	if (start + minbatch > readline) {
		for (i = start; i < readline; i++) {
			lossV += trainDataSet[i].label * log(sigmoidCalc(wxbCalc(trainDataSet + i)));
			lossV += (1 - trainDataSet[i].label)*log(1 - sigmoidCalc(wxbCalc(trainDataSet + i)));
		}
		end = minbatch - readline + start;
		for (i = 0; i < end; i++) {
			lossV += trainDataSet[i].label * log(sigmoidCalc(wxbCalc(trainDataSet + i)));
			lossV += (1 - trainDataSet[i].label)*log(1 - sigmoidCalc(wxbCalc(trainDataSet + i)));
		}
	}
	else {
		for (i = start; i < readline; i++) {
			lossV += trainDataSet[i].label * log(sigmoidCalc(wxbCalc(trainDataSet + i)));
			lossV += (1 - trainDataSet[i].label)*log(1 - sigmoidCalc(wxbCalc(trainDataSet + i)));
		}
	}
	lossV /= minbatch;
	return -lossV;
}
//*/
///*  批量梯度
double LR::MgradientSlope(const Dt* dataSet, int index, const double * sigmoidVec, double &db)//dw
{
	double gsV = 0.0L;
	int i;
	double sigv, label;
	db = 0;
	for (i = 0; i < readline; i++) {
		sigv = sigmoidVec[i];
		label = dataSet[i].label;
		double dev = sigv - label;//a-y
		db += dev;
		gsV += dev * (dataSet[i].features[index]);
	}
	db /= readline;
	gsV /= readline;
	return gsV;
}
//*/
double LR::gradientSlope(const Dt* dataSet, int index, int start, const double * sigmoidVec, double &db)//dw
{
	double gsV = 0.0L;
	int i;
	double sigv, label;
	db = 0;
	int end = start + minbatch;
	if (start + minbatch > readline) {
		for (i = start; i < readline; i++) {
			sigv = sigmoidVec[i];
			label = dataSet[i].label;
			double dev = sigv - label;//a-y
			db += dev;
			gsV += dev * (dataSet[i].features[index]);
		}
		end = minbatch - readline + start;
		for (i = 0; i < end; i++) {
			sigv = sigmoidVec[i];
			label = dataSet[i].label;
			double dev = sigv - label;//a-y
			db += dev;
			gsV += dev * (dataSet[i].features[index]);
		}
	}
	else {
		for (i = start; i < end; i++) {
			sigv = sigmoidVec[i];
			label = dataSet[i].label;
			double dev = sigv - label;//a-y
			db += dev;
			gsV += dev * (dataSet[i].features[index]);
		}
	}
	db /= minbatch;
	gsV /= minbatch;
	return gsV;
}
void LR::train()
{
	//time_t start_time = clock();
	//norm(trainDataSet);
	double sigmoidVal;
	double wxbVal;
	int i, j;
	double loss_cur = 1000;
	double step = stepSize;
	double* sigmoidVec = new double[readline];

	//
	double * vdw = new double[featuresNum];
	double vdb = param.bSet;
	double * Sdw = new double[featuresNum];
	double  Sdb = param.bSet;;
	for (i = 0; i < featuresNum; i++) {
		vdw[i] = param.wtSet[i];
		Sdw[i] = param.wtSet[i] * param.wtSet[i];
	}
	//
	int start = 0;
	for (i = 1; i < maxIterTimes; i++) {//&& (loss_cur>0.1)
		double beta1correct = 1 / (1 - pow(beta1, i));
		double beta2correct = 1 / (1 - pow(beta2, i));
		int end = start + minbatch;
		if (start + minbatch > readline) {
			end = minbatch - readline + start;
			for (j = start; j < readline; j++) {
				wxbVal = wxbCalc(trainDataSet + j);//wx+b
				sigmoidVal = sigmoidCalc(wxbVal);
				sigmoidVec[j] = sigmoidVal;
			}
			for (j = 0; j < end; j++) {
				wxbVal = wxbCalc(trainDataSet + j);//wx+b
				sigmoidVal = sigmoidCalc(wxbVal);
				sigmoidVec[j] = sigmoidVal;
			}
		}
		else {
			for (j = start; j < end; j++) {
				wxbVal = wxbCalc(trainDataSet + j);//wx+b
				sigmoidVal = sigmoidCalc(wxbVal);
				sigmoidVec[j] = sigmoidVal;
			}
		}
		double db = 0;
		for (j = 0; j < featuresNum; j++) {
			double gk = gradientSlope(trainDataSet, j, start, sigmoidVec, db);
			vdw[j] = beta1*vdw[j] + (1 - beta1)*gk;
			Sdw[j] = beta2*Sdw[j] + (1 - beta2)*gk*gk;
			double vdwcorrect = vdw[j] * beta1correct;
			double Sdwcorrect = Sdw[j] * beta2correct;
			param.wtSet[j] -= step*vdwcorrect / (sqrt(Sdwcorrect) + e);
		}
		vdb = beta1*vdb + (1 - beta1)* db;
		Sdb = beta2*Sdb + (1 - beta2)* db* db;
		double vdbcorrect = vdb * beta1correct;
		double Sdbcorrect = Sdb * beta2correct;
		param.bSet -= step * vdbcorrect / (sqrt(Sdbcorrect) + e);
		//更新参数
		if (end<readline)start = end;
		else start = 0;
		//}//小批量训练
		/*
		if (i % train_show_step == 0) {
		cout << "iter " << i << ". updated weight value is : ";
		for (j = 0; j < featuresNum; j++) {
		cout << param.wtSet[j] << "  ";
		}
		cout << param.bSet << endl;
		}
		*/
	}
	//time_t end_time = clock();
	//cout << "train time: " << end_time - start_time << "ms" << endl;
}

void LR::predict()
{
	double sigVal;
	int predictVal;
	loadTestData();
	predictVec = new int[testlength];
	for (int j = 0; j < testlength; j++) {
		sigVal = sigmoidCalc(wxbCalc(testDataSet + j));
		//sigVal = wxbCalc(testDataSet + j);
		predictVal = (sigVal >= predictTrueThresh ? 1 : 0);
		*(predictVec + j) = predictVal;
	}
	storePredict(predictVec);
}
/*
int LR::loadModel()
{
string line;
int i;
vector<double> wtTmp;
double dbt;

ifstream fin(weightParamFile.c_str());
if (!fin) {
cout << "打开模型参数文件失败" << endl;
exit(0);
}

getline(fin, line);
stringstream sin(line);
for (i = 0; i < featuresNum; i++) {
char c = sin.peek();
if (c == -1) {
cout << "模型参数数量少于特征数量，退出" << endl;
return -1;
}
sin >> dbt;
wtTmp.push_back(dbt);
}
param.wtSet.swap(wtTmp);
fin.close();
return 0;
}
*/
int LR::storeModel()
{
	string line;
	int i;

	ofstream fout(weightParamFile.c_str());
	if (!fout.is_open()) {
		cout << "打开模型参数文件失败" << endl;
	}
	//if (sizeof(param.wtSet) / sizeof(param.wtSet[0]) < featuresNum) {
	cout << "wtSet size is " << featuresNum << endl;
	//}
	for (i = 0; i < featuresNum; i++) {
		fout << param.wtSet[i] << " ";
	}
	fout.close();
	return 0;
}


bool LR::loadTestData()
{
	//time_t start_time = clock();
	FILE *fp;
	if ((fp = fopen(testFile.c_str(), "r")) == NULL) {
		cout << "打开文件错误" << endl;
		return false;
	}
	fseek(fp, 0L, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	testDataSet = new Dt[len / featuresNum];
	//cout << "loading" << endl;
	char *buffer = new char[len];    //把所有字符一次全部读取到buf里面，包括‘\n‘
	if (fread(buffer, 1, len, fp) != len) {
		cout << "读取失败" << endl;
		return false;
	}
	//
	bool is_dec = false;
	double nums = 0.0L;
	testDataSet[0].features = new double[20001];
	double*p = testDataSet[0].features;//feature[0];
	int k = 0;
	int nt = 10;
	bool sign = false;
	//cout << "loading" << endl;
	for (int i = 0; i < len; i++) {
		if (buffer[i] == ',') {
			if (sign) nums = -nums;
			*p = nums;
			p++;
			nums = 0.0L;
			nt = 10;
			sign = false;
			is_dec = false;
			continue;
		}
		else if (buffer[i] == '\n') {
			*p = nums;//存入特征值
			testDataSet[k].label = 0;
			k++;
			testDataSet[k].features = new double[20001];
			p = testDataSet[k].features;
			nums = 0.0L;
			nt = 10;
			sign = false;
			is_dec = false;
			continue;
		}
		else if (buffer[i] == '-') {
			sign = true;
		}
		else if (buffer[i] == '.') {
			is_dec = true;
			i++;
		}
		if (!is_dec)nums = buffer[i] - '0';
		else {
			nums += (double)(buffer[i] - '0') / nt;
			nt *= 10;
		}
	}
	testlength = k;
	//cout << "finish" << endl;
	//
	fclose(fp);
	//time_t end_time = clock();
	//cout << "testload time: " << end_time - start_time << "ms" << endl;
	return true;
}

bool loadAnswerData(string awFile, vector<int> &awVec)
{
	ifstream infile(awFile.c_str());
	if (!infile) {
		cout << "打开答案文件失败" << endl;
		exit(0);
	}

	while (infile) {
		string line;
		int aw;
		getline(infile, line);
		if (line.size() > 0) {
			stringstream sin(line);
			sin >> aw;
			awVec.push_back(aw);
		}
	}

	infile.close();
	return true;
}

int LR::storePredict(int *predict)
{
	string line;
	int i;

	ofstream fout(predictOutFile.c_str());
	if (!fout.is_open()) {
		cout << "打开预测结果文件失败" << endl;
	}
	for (i = 0; i < testlength; i++) {
		fout << predict[i] << endl;
	}
	fout.close();
	return 0;
}

int main(int argc, char *argv[])
{
	//time_t start_time = clock();
	vector<int> answerVec;
	vector<int> predictVec;
	int correctCount;
	double accurate;
	string trainFile = "/data/train_data.txt";
	string testFile = "/data/test_data.txt";
	string predictFile = "/projects/student/result.txt";

	string answerFile = "/projects/student/answer.txt";

	LR logist(trainFile, testFile, predictFile);

	//cout << "ready to train model" << endl;
	logist.train();

	//cout << "training ends, ready to store the model" << endl;
	//logist.storeModel();
	//#define TEST
#ifdef TEST
	//cout << "ready to load answer data" << endl;
	loadAnswerData(answerFile, answerVec);
#endif

	//cout << "let's have a prediction test" << endl;
	logist.predict();
	//time_t end_time = clock();
#ifdef TEST
	loadAnswerData(predictFile, predictVec);
	cout << "test data set size is " << predictVec.size() << endl;
	correctCount = 0;
	for (int j = 0; j < logist.testlength; j++) {
		if (j < answerVec.size()) {
			if (answerVec[j] == predictVec[j]) {
				correctCount++;
			}
		}
		else {
			cout << "answer size less than the real predicted value" << endl;
		}
	}

	accurate = ((double)correctCount) / answerVec.size();
	cout << "the prediction accuracy is " << accurate << "%" << endl;
	//cout << "locot running time is " << end_time - start_time <<"ms"<< endl;
#endif

	return accurate * 1000;
}
