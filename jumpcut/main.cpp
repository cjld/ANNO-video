#include "videoCutout.h"
#include <iostream>
#include <opencv2/ximgproc.hpp>

using namespace cv;
using namespace cv::ximgproc;
using namespace std;

void JumpCutOnce();
int main()
{
	JumpCutOnce();
	return 0;
}

void fuck(Mat mat) {
	double max,min;
	cv::minMaxLoc(mat, &min, &max);
	cout << mat.size() << " " << mat.type() << " max:" << max << " min:" << min << endl;
}

void getEdge(Mat &input_image, Mat &output_edge) {
	fuck(input_image);
	Mat input = input_image.clone();
	input.convertTo(input, cv::DataType<float>::type, 1/255.0);
	Mat edges(input.size(), input.type());
	static Ptr<StructuredEdgeDetection> pDollar;
	if (!pDollar) {
		printf("load model.\n");
		pDollar = createStructuredEdgeDetection("/home/randon/anno/ANNO-video/build/jumpcut/model.yml.gz");
	}
	pDollar->detectEdges(input, edges);
	output_edge = (1-edges)*255;
	output_edge.convertTo(output_edge, input_image.type());
	//cv::cvtColor(output_edge, output_edge, cv::COLOR_GRAY2BGR);
	//fuck(output_edge);
	//fuck(input);
}

void JumpCutOnce()
{
	clock_t time = clock();
	Mat sourceImg, sourceMask, targetImg, sourceEdge, targetEdge, result;
	Mat sourceEdge2, targetEdge2;
	char dirName[] = "";
	char filename[256];
	memset(filename, 0, sizeof(filename)); sprintf(filename, "%s0.png", dirName);
	sourceImg = imread(filename);
	memset(filename, 0, sizeof(filename)); sprintf(filename, "%s1.png", dirName);
	targetImg = imread(filename);
	memset(filename, 0, sizeof(filename)); sprintf(filename, "%s2.png", dirName);
	sourceMask = imread(filename, 0);


	getEdge(sourceImg, sourceEdge);
	//imwrite("sourceEdge.png", sourceEdge);
	getEdge(targetImg, targetEdge);
	//imwrite("targetEdge.png", targetEdge);

	int intervalNum = 4;
	Mat tempMat;
	bool useEdge = true;
	videoCutout vc;
	vc.PatchMatch(sourceImg, sourceMask, sourceEdge, tempMat, tempMat, tempMat, targetImg, targetEdge, intervalNum);
	vc.EdgeClassifier(tempMat, tempMat, tempMat);
	string videoPath = dirName;
	vc.levelset(result, useEdge);

	time = clock() - time;
	printf("Total time is %f seconds.\n", (float)time / CLOCKS_PER_SEC);
}
